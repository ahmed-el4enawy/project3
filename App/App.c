/**
 * @file    App.c
 * @brief   Auto-Cooler Mealy State Machine implementation.
 *          Outputs depend on both current state AND current temperature event.
 *          Implemented as switch-case dispatch table -- no flat if-else chains.
 *
 *  ADC driver : AbdallahDarwish interrupt-based driver (Adc.h).
 *               Adc_ReadSingleChannelAsync() used -- no busy-wait in App layer.
 *               Temperature conversion (moved from Adc driver) lives here.
 *
 *  Alarm LED  : PD12 (Active HIGH)
 *  Fan PWM    : TIMER3 / PWM_CHANNEL_1 on PA6 (AF2), 10 kHz
 *  LCD        : 16x2 LCD1602 in 4-bit mode (see Lcd.h)
 *
 */

#include "App.h"
#include "Adc.h"
#include "Lcd.h"
#include "Gpio.h"
#include "Pwm.h"
#include "Rcc.h"
#include "Timer.h"

/* =========================================================
 * Hardware resource constants
 * ========================================================= */

/** Alarm LED: PD14 (Red LED on STM32F407 Discovery), active HIGH.
 *  Discovery board LED map: PD12=Green, PD13=Orange, PD14=Red, PD15=Blue. */
#define APP_ALARM_LED_PORT      GPIO_D
#define APP_ALARM_LED_PIN       (14U)

/** Fan PWM: TIM3 CH1, PA6, AF2 */
#define APP_FAN_TIMER_ID        TIMER3
#define APP_FAN_CHANNEL         PWM_CHANNEL_1
#define APP_FAN_GPIO_PORT       GPIO_A
#define APP_FAN_GPIO_PIN        (6U)
#define APP_FAN_PSC             (15U)   /**< PSC+1=16, 16 MHz / 16 = 1 MHz tick */
#define APP_FAN_ARR             (99U)   /**< ARR+1=100 → 1 MHz / 100 = 10 kHz   */

/** Fan duty cycle levels (percent) */
#define APP_FAN_DUTY_OFF        (0U)
#define APP_FAN_DUTY_LOW        (33U)
#define APP_FAN_DUTY_MED        (66U)
#define APP_FAN_DUTY_FULL       (100U)

/** Temperature thresholds in tenths of degree C */
#define APP_THRESH_25_X10       (250)   /**< 25.0 C x 10 */
#define APP_THRESH_30_X10       (300)   /**< 30.0 C x 10 */
#define APP_THRESH_35_X10       (350)   /**< 35.0 C x 10 */
#define APP_THRESH_40_X10       (400)   /**< 40.0 C x 10 */

/** ADC conversion constants (LM35: 10 mV/C, Vref=3.3V, 12-bit=4096 steps)
 *  A ×15 multiplier is applied after the voltage conversion so that the
 *  simulation reading scales up to a more useful display range. */
#define APP_ADC_VREF_MV         (3300UL)  /**< Reference voltage in mV        */
#define APP_ADC_RESOLUTION      (4096UL)  /**< 12-bit ADC full scale           */
#define APP_ADC_TEMP_MULTIPLIER (6UL)    /**< Scale factor applied to reading */

/* =========================================================
 * Private async-ADC state  (set from ISR, consumed in App_Run)
 * ========================================================= */

/** Raw ADC sample stored by the ADC ISR callback.  volatile because it is
 *  written from interrupt context and read from the main loop. */
static volatile uint16 App_AdcLastRaw  = 0U;

/** Set to 1 by the ADC callback when a new sample is available. */
static volatile uint8  App_AdcReady    = 0U;

/* =========================================================
 * Private state variable
 * ========================================================= */
static AppState_t currentState = APP_STATE_IDLE;

/* =========================================================
 * Private function prototypes
 * ========================================================= */
static void       App_AdcCallback(uint16 rawResult);   /* ADC ISR callback     */
static sint32     App_RawToTempCelsius_x10(uint16 rawValue);
static AppEvent_t App_ClassifyTemperature(sint32 temp_x10);
static void       App_UpdateOutputs(AppState_t state, AppEvent_t event, sint32 temp_x10);
static void       App_HandleIdle(AppEvent_t event, sint32 temp_x10);
static void       App_HandleCooling(AppEvent_t event, sint32 temp_x10);
static void       App_HandleOverheat(AppEvent_t event, sint32 temp_x10);

/* Handler function-pointer table indexed by AppState_t */
typedef void (*AppStateHandler_t)(AppEvent_t event, sint32 temp_x10);

static const AppStateHandler_t App_StateTable[3] =
{
    App_HandleIdle,     /* APP_STATE_IDLE     = 0 */
    App_HandleCooling,  /* APP_STATE_COOLING  = 1 */
    App_HandleOverheat  /* APP_STATE_OVERHEAT = 2 */
};

/* =========================================================
 * Public API
 * ========================================================= */

/**
 * @brief  Initialise fan PWM and alarm LED; set state to IDLE, fan OFF.
 */
void App_Init(void)
{
    /* --- Alarm LED: PD12 as push-pull output, initially LOW (OFF) --- */
    Gpio_Init(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, LOW);

    /* --- Fan GPIO: PA6 as alternate function AF2 (TIM3_CH1) --- */
    Gpio_Init(APP_FAN_GPIO_PORT, APP_FAN_GPIO_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(APP_FAN_GPIO_PORT, APP_FAN_GPIO_PIN, GPIO_AF2);

    /* --- PWM: TIM3 CH1 @ 10 kHz, start with 0% duty (fan OFF) --- */
    Pwm_Init(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, APP_FAN_PSC, APP_FAN_ARR);
    Pwm_SetDutyPercent(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, APP_FAN_DUTY_OFF);
    Pwm_Start(APP_FAN_TIMER_ID, APP_FAN_CHANNEL);

    /* Initial state */
    currentState = APP_STATE_IDLE;

    /* Display initial splash -- clear any garbage on LCD */
    Lcd_Clear();
    Lcd_SetCursor(0U, 0U);
    Lcd_Print("Temp:  --.- C   ");
    Lcd_SetCursor(1U, 0U);
    Lcd_Print("Fan:    0%      ");

    /* ---- Arm async ADC: register callback, fire first conversion ----
     * Adc_ReadSingleChannelAsync enables EOCIE + NVIC so the ADC ISR
     * (ADC_IRQHandler in Adc.c) will call App_AdcCallback when done.
     * The callback stores the result and re-arms the next conversion,
     * ensuring no busy-wait ever blocks the main loop.
     */
    Adc_ReadSingleChannelAsync(App_AdcCallback);
    Adc_StartConversion();   /* kick off the very first sample */
}

/**
 * @brief  Check for a new ADC sample and, if available, run the state machine.
 *         Returns immediately (non-blocking) when no new data is ready.
 *         Requirement 4 compliant: no busy-wait in application code.
 */
void App_Run(void)
{
    uint16     rawValue;
    sint32     temp_x10;
    AppEvent_t event;

    /* No new sample yet -- return without blocking */
    if (App_AdcReady == 0U)
    {
        return;
    }

    /* Atomically snapshot and clear the ready flag */
    App_AdcReady = 0U;
    rawValue     = App_AdcLastRaw;

    /* Arm the next conversion immediately so the ADC ISR fires again */
    Adc_StartConversion();

    /* Convert raw 12-bit value to tenths of a degree */
    temp_x10 = App_RawToTempCelsius_x10(rawValue);
    event    = App_ClassifyTemperature(temp_x10);

    /* Dispatch to state handler via function-pointer table */
    App_StateTable[currentState](event, temp_x10);
}

/* =========================================================
 * Private helpers
 * ========================================================= */

/**
 * @brief  ADC ISR callback -- called from ADC_IRQHandler in Adc.c.
 *         Stores the raw result and sets the ready flag consumed by App_Run.
 *         MUST be kept short; no LCD or NVIC calls here.
 * @param  rawResult  12-bit ADC raw value delivered by the ISR.
 */
static void App_AdcCallback(uint16 rawResult)
{
    App_AdcLastRaw = rawResult;
    App_AdcReady   = 1U;
}

/**
 * @brief  Convert a raw 12-bit ADC reading to temperature in tenths of degree C.
 *         Formula: Temp_x10 = (rawValue * Vref_mV * 15) / 4096
 *         LM35: 10 mV/C => Voltage_mV = Temp x 10, then scaled by x15.
 *         Overflow check: 4095 * 3300 * 15 = 202,702,500 < sint32 max (~2.1B).
 */
static sint32 App_RawToTempCelsius_x10(uint16 rawValue)
{
    return ((sint32)rawValue * (sint32)APP_ADC_VREF_MV * (sint32)APP_ADC_TEMP_MULTIPLIER)
           / (sint32)APP_ADC_RESOLUTION;
}

/**
 * @brief  Map a temperature reading to one of the defined FSM events.
 *         Uses the current state to distinguish OVERHEAT recovery.
 */
static AppEvent_t App_ClassifyTemperature(sint32 temp_x10)
{
    AppEvent_t event;

    if (temp_x10 >= APP_THRESH_40_X10)
    {
        event = APP_EVENT_TEMP_ABOVE_40;
    }
    else if (currentState == APP_STATE_OVERHEAT)
    {
        /* Temperature dropped below 40 — recovery event */
        event = APP_EVENT_TEMP_BELOW_40_RECOVERY;
    }
    else if (temp_x10 >= APP_THRESH_35_X10)
    {
        event = APP_EVENT_TEMP_35_TO_40;
    }
    else if (temp_x10 >= APP_THRESH_30_X10)
    {
        event = APP_EVENT_TEMP_30_TO_35;
    }
    else if (temp_x10 >= APP_THRESH_25_X10)
    {
        event = APP_EVENT_TEMP_25_TO_30;
    }
    else
    {
        event = APP_EVENT_TEMP_BELOW_25;
    }

    return event;
}

/**
 * @brief  Apply hardware outputs: fan duty, alarm LED, and LCD update.
 *         Called by each state handler after computing the next state.
 * @param  state    The state determining the LCD line 2 format.
 * @param  event    Current temperature event (determines fan duty).
 * @param  temp_x10 Temperature × 10 for display.
 */
static void App_UpdateOutputs(AppState_t state, AppEvent_t event, sint32 temp_x10)
{
    uint8 fanDuty;

    /* --- Determine fan duty from event --- */
    switch (event)
    {
        case APP_EVENT_TEMP_BELOW_25:
            fanDuty = APP_FAN_DUTY_OFF;
            break;

        case APP_EVENT_TEMP_25_TO_30:
            fanDuty = APP_FAN_DUTY_LOW;
            break;

        case APP_EVENT_TEMP_30_TO_35:
            fanDuty = APP_FAN_DUTY_MED;
            break;

        case APP_EVENT_TEMP_35_TO_40:
        case APP_EVENT_TEMP_ABOVE_40:
        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            fanDuty = APP_FAN_DUTY_FULL;
            break;

        default:
            fanDuty = APP_FAN_DUTY_OFF;
            break;
    }

    /* --- Set PWM duty cycle --- */
    Pwm_SetDutyPercent(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, fanDuty);

    /* --- Alarm LED --- */
    if (state == APP_STATE_OVERHEAT)
    {
        Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, HIGH);
    }
    else
    {
        Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, LOW);
    }

    /* --- LCD Line 1: Temperature --- */
    Lcd_SetCursor(0U, 0U);
    Lcd_Print("Temp:");
    Lcd_PrintTemp(temp_x10);
    Lcd_Print(" C  ");

    /* --- LCD Line 2: Fan speed or OVERHEAT warning --- */
    Lcd_SetCursor(1U, 0U);

    if (state == APP_STATE_OVERHEAT)
    {
        Lcd_Print("!! OVERHEAT !!  ");
    }
    else
    {
        Lcd_Print("Fan:");
        Lcd_PrintInt((sint32)fanDuty, 3);
        Lcd_Print("%           ");
    }
}

/**
 * @brief  Handle events while in IDLE state.
 */
static void App_HandleIdle(AppEvent_t event, sint32 temp_x10)
{
    switch (event)
    {
        case APP_EVENT_TEMP_BELOW_25:
            /* Stay IDLE, fan OFF */
            currentState = APP_STATE_IDLE;
            App_UpdateOutputs(APP_STATE_IDLE, event, temp_x10);
            break;

        case APP_EVENT_TEMP_25_TO_30:
        case APP_EVENT_TEMP_30_TO_35:
        case APP_EVENT_TEMP_35_TO_40:
            /* Transition to COOLING */
            currentState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, temp_x10);
            break;

        case APP_EVENT_TEMP_ABOVE_40:
            /* Jump directly to OVERHEAT */
            currentState = APP_STATE_OVERHEAT;
            App_UpdateOutputs(APP_STATE_OVERHEAT, event, temp_x10);
            break;

        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            /* Should not occur in IDLE, but handle gracefully */
            currentState = APP_STATE_IDLE;
            App_UpdateOutputs(APP_STATE_IDLE, APP_EVENT_TEMP_BELOW_25, temp_x10);
            break;

        default:
            /* Unreachable — defensive default */
            break;
    }
}

/**
 * @brief  Handle events while in COOLING state.
 */
static void App_HandleCooling(AppEvent_t event, sint32 temp_x10)
{
    switch (event)
    {
        case APP_EVENT_TEMP_BELOW_25:
            /* Temperature safe — return to IDLE, fan OFF */
            currentState = APP_STATE_IDLE;
            App_UpdateOutputs(APP_STATE_IDLE, event, temp_x10);
            break;

        case APP_EVENT_TEMP_25_TO_30:
        case APP_EVENT_TEMP_30_TO_35:
        case APP_EVENT_TEMP_35_TO_40:
            /* Stay in COOLING, adjust fan duty */
            currentState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, temp_x10);
            break;

        case APP_EVENT_TEMP_ABOVE_40:
            /* Overheat — assert alarm */
            currentState = APP_STATE_OVERHEAT;
            App_UpdateOutputs(APP_STATE_OVERHEAT, event, temp_x10);
            break;

        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            /* Should not occur in COOLING, treat as below-35 */
            currentState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, APP_EVENT_TEMP_35_TO_40, temp_x10);
            break;

        default:
            break;
    }
}

/**
 * @brief  Handle events while in OVERHEAT state.
 */
static void App_HandleOverheat(AppEvent_t event, sint32 temp_x10)
{
    switch (event)
    {
        case APP_EVENT_TEMP_ABOVE_40:
            /* Still overheating — maintain full fan and alarm */
            currentState = APP_STATE_OVERHEAT;
            App_UpdateOutputs(APP_STATE_OVERHEAT, event, temp_x10);
            break;

        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            /* Temperature recovered below 40 °C — clear alarm, resume COOLING */
            currentState = APP_STATE_COOLING;
            /* Re-classify event properly for the cooling range */
            {
                AppEvent_t recoveryEvent;

                if (temp_x10 >= APP_THRESH_35_X10)
                {
                    recoveryEvent = APP_EVENT_TEMP_35_TO_40;
                }
                else if (temp_x10 >= APP_THRESH_30_X10)
                {
                    recoveryEvent = APP_EVENT_TEMP_30_TO_35;
                }
                else if (temp_x10 >= APP_THRESH_25_X10)
                {
                    recoveryEvent = APP_EVENT_TEMP_25_TO_30;
                }
                else
                {
                    recoveryEvent = APP_EVENT_TEMP_BELOW_25;
                }

                App_UpdateOutputs(APP_STATE_COOLING, recoveryEvent, temp_x10);
            }
            break;

        default:
            /* Any other below-40 event should not reach here, but handle safely */
            currentState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, temp_x10);
            break;
    }
}
