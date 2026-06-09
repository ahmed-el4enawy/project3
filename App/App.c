/**
 * @file    App.c
 * @brief   Core Application logic and Mealy FSM implementation for the Auto-Cooler.
 * @author  Team 27 (Ahmed Salah Geoshy Elshenawy & Ahmed Ahmed Mokhtar)
 * @date    2026-04-29
 *
 * @details
 * - Hardware Abstraction: All pin and threshold settings are imported from App_Config.h.
 * - ADC Integration: Uses asynchronous, non-blocking interrupt callbacks.
 * - FSM Design: Implemented via a function-pointer dispatch table for O(1) transitions.
 * - Performance: Satisfies non-blocking requirements (no polling/busy-waits).
 */

#include "App.h"
#include "App_Config.h"
#include "Adc.h"
#include "Lcd.h"
#include "Gpio.h"
#include "Pwm.h"
#include "Rcc.h"
#include "Timer.h"

/* =========================================================
 * Private Asynchronous ADC State
 * ========================================================= */

/** Volatile sample buffer and flag, updated by the ADC ISR in Adc.c */
static volatile uint16 Adc_LatestRawSample  = 0U;
static volatile uint8  Adc_NewDataAvailable = 0U;

/* =========================================================
 * Private FSM Tracking
 * ========================================================= */
static AppState_t ActiveState = APP_STATE_IDLE;

/* =========================================================
 * Private Prototypes & Dispatch Table
 * ========================================================= */
static void       App_AdcCallback(uint16 rawResult);
static sint32     App_RawToTempCelsius_x10(uint16 rawValue);
static AppEvent_t App_ClassifyTemperature(sint32 currentTempX10);
static void       App_UpdateOutputs(AppState_t state, AppEvent_t event, sint32 currentTempX10);
static void       App_HandleIdle(AppEvent_t event, sint32 currentTempX10);
static void       App_HandleCooling(AppEvent_t event, sint32 currentTempX10);
static void       App_HandleOverheat(AppEvent_t event, sint32 currentTempX10);

/** Function pointer array mapped to AppState_t enumeration for clean dispatching */
typedef void (*FsmHandler_t)(AppEvent_t event, sint32 currentTempX10);

static const FsmHandler_t FsmDispatchTable[3] =
{
    App_HandleIdle,     // APP_STATE_IDLE     (0)
    App_HandleCooling,  // APP_STATE_COOLING  (1)
    App_HandleOverheat  // APP_STATE_OVERHEAT (2)
};

/* =========================================================
 * Public API Implementation
 * ========================================================= */

/**
 * @brief  Bootstrap system peripherals and FSM initial conditions.
 */
void App_Init(void)
{
    /* --- Alarm System: LED --- */
    Gpio_Init(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, LOW);

    /* --- Cooling System: PWM --- */
    Gpio_Init(APP_FAN_GPIO_PORT, APP_FAN_GPIO_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(APP_FAN_GPIO_PORT, APP_FAN_GPIO_PIN, GPIO_AF2);
    Pwm_Init(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, APP_FAN_PSC, APP_FAN_ARR);
    Pwm_SetDutyPercent(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, APP_FAN_DUTY_OFF);
    Pwm_Start(APP_FAN_TIMER_ID, APP_FAN_CHANNEL);

    /* --- Logic Reset --- */
    ActiveState = APP_STATE_IDLE;

    /* --- User Interface: LCD --- */
    Lcd_Clear();
    Lcd_SetCursor(0U, 0U);
    Lcd_Print("Temp:  --.- C   ");
    Lcd_SetCursor(1U, 0U);
    Lcd_Print("Fan:    0%      ");

    /* --- Start Background Sensing --- */
    Adc_ReadSingleChannelAsync(App_AdcCallback);
    Adc_StartConversion();
}

/**
 * @brief  Task handler called in the main loop. Dispatches FSM if new data is ready.
 */
void App_Run(void)
{
    uint16     adcSnapshot;
    sint32     currentTempX10;
    AppEvent_t currentEvent;

    /* Non-blocking: Exit immediately if ADC hardware is still processing */
    if (Adc_NewDataAvailable == 0U)
    {
        return;
    }

    /* Capture sample and reset ISR flag */
    Adc_NewDataAvailable = 0U;
    adcSnapshot          = Adc_LatestRawSample;

    /* Immediately trigger next background read to maximize sampling rate */
    Adc_StartConversion();

    /* Process captured raw data */
    currentTempX10 = App_RawToTempCelsius_x10(adcSnapshot);
    currentEvent   = App_ClassifyTemperature(currentTempX10);

    /* Execute the state handler logic */
    FsmDispatchTable[ActiveState](currentEvent, currentTempX10);
}

/* =========================================================
 * Internal Logic Helpers
 * ========================================================= */

/**
 * @brief  Asynchronous ISR callback. Updates shared memory with latest sample.
 */
static void App_AdcCallback(uint16 rawResult)
{
    Adc_LatestRawSample  = rawResult;
    Adc_NewDataAvailable = 1U;
}

/**
 * @brief  Converts 12-bit ADC result to temperature (Celsius * 10).
 * Formula: Temp_x10 = (rawValue * Vref_mV * Multiplier) / 4096
 */
static sint32 App_RawToTempCelsius_x10(uint16 rawValue)
{
    return ((sint32)rawValue * (sint32)APP_ADC_VREF_MV * (sint32)APP_ADC_TEMP_MULTIPLIER)
           / (sint32)APP_ADC_RESOLUTION;
}

/**
 * @brief  Categorizes temperature into specific events using Config thresholds.
 */
static AppEvent_t App_ClassifyTemperature(sint32 currentTempX10)
{
    AppEvent_t event;

    if (currentTempX10 >= APP_THRESH_CRITICAL)
    {
        event = APP_EVENT_TEMP_ABOVE_40;
    }
    else if (ActiveState == APP_STATE_OVERHEAT)
    {
        /* Recovery logic for hysteresis: State was Overheat but T dropped < 40 */
        event = APP_EVENT_TEMP_BELOW_40_RECOVERY;
    }
    else if (currentTempX10 >= APP_THRESH_HIGH)
    {
        event = APP_EVENT_TEMP_35_TO_40;
    }
    else if (currentTempX10 >= APP_THRESH_MID)
    {
        event = APP_EVENT_TEMP_30_TO_35;
    }
    else if (currentTempX10 >= APP_THRESH_SAFE)
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
 * @brief  Coordinates hardware outputs based on State Machine decisions.
 */
static void App_UpdateOutputs(AppState_t state, AppEvent_t event, sint32 currentTempX10)
{
    uint8 targetDuty;

    /* Mapping Events to Fan Speeds */
    switch (event)
    {
        case APP_EVENT_TEMP_BELOW_25:
            targetDuty = APP_FAN_DUTY_OFF;
            break;
        case APP_EVENT_TEMP_25_TO_30:
            targetDuty = APP_FAN_DUTY_LOW;
            break;
        case APP_EVENT_TEMP_30_TO_35:
            targetDuty = APP_FAN_DUTY_MED;
            break;
        case APP_EVENT_TEMP_35_TO_40:
        case APP_EVENT_TEMP_ABOVE_40:
        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            targetDuty = APP_FAN_DUTY_FULL;
            break;
        default:
            targetDuty = APP_FAN_DUTY_OFF;
            break;
    }

    Pwm_SetDutyPercent(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, targetDuty);

    /* Toggle Alert based on State */
    if (state == APP_STATE_OVERHEAT)
    {
        Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, HIGH);
    }
    else
    {
        Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, LOW);
    }

    /* Refresh Display */
    Lcd_SetCursor(0U, 0U);
    Lcd_Print("Temp:");
    Lcd_PrintTemp(currentTempX10);
    Lcd_Print(" C  ");

    Lcd_SetCursor(1U, 0U);
    if (state == APP_STATE_OVERHEAT)
    {
        Lcd_Print("!! OVERHEAT !!  ");
    }
    else
    {
        Lcd_Print("Fan:");
        Lcd_PrintInt((sint32)targetDuty, 3);
        Lcd_Print("%           ");
    }
}

/* =========================================================
 * FSM Handler Implementation
 * ========================================================= */

static void App_HandleIdle(AppEvent_t event, sint32 currentTempX10)
{
    switch (event)
    {
        case APP_EVENT_TEMP_BELOW_25:
            ActiveState = APP_STATE_IDLE;
            App_UpdateOutputs(APP_STATE_IDLE, event, currentTempX10);
            break;
        case APP_EVENT_TEMP_25_TO_30:
        case APP_EVENT_TEMP_30_TO_35:
        case APP_EVENT_TEMP_35_TO_40:
            ActiveState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, currentTempX10);
            break;
        case APP_EVENT_TEMP_ABOVE_40:
            ActiveState = APP_STATE_OVERHEAT;
            App_UpdateOutputs(APP_STATE_OVERHEAT, event, currentTempX10);
            break;
        default:
            break;
    }
}

static void App_HandleCooling(AppEvent_t event, sint32 currentTempX10)
{
    switch (event)
    {
        case APP_EVENT_TEMP_BELOW_25:
            ActiveState = APP_STATE_IDLE;
            App_UpdateOutputs(APP_STATE_IDLE, event, currentTempX10);
            break;
        case APP_EVENT_TEMP_25_TO_30:
        case APP_EVENT_TEMP_30_TO_35:
        case APP_EVENT_TEMP_35_TO_40:
            ActiveState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, currentTempX10);
            break;
        case APP_EVENT_TEMP_ABOVE_40:
            ActiveState = APP_STATE_OVERHEAT;
            App_UpdateOutputs(APP_STATE_OVERHEAT, event, currentTempX10);
            break;
        default:
            break;
    }
}

static void App_HandleOverheat(AppEvent_t event, sint32 currentTempX10)
{
    switch (event)
    {
        case APP_EVENT_TEMP_ABOVE_40:
            ActiveState = APP_STATE_OVERHEAT;
            App_UpdateOutputs(APP_STATE_OVERHEAT, event, currentTempX10);
            break;
        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            ActiveState = APP_STATE_COOLING;
            /* Re-classify for the lower cooling ranges */
            AppEvent_t subEvent = App_ClassifyTemperature(currentTempX10);
            App_UpdateOutputs(APP_STATE_COOLING, subEvent, currentTempX10);
            break;
        default:
            ActiveState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, currentTempX10);
            break;
    }
}