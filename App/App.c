/**
 * @file    App.c
 * @brief   Core Application layer and Mealy FSM logic for the Auto-Cooler.
 * @author  Team 27 (Ahmed Salah Geoshy Elshenawy & Ahmed Ahmed Mokhtar)
 * @details
 * - ADC Integration: Uses asynchronous, non-blocking interrupts.
 * - FSM Design: Function-pointer dispatch table to eliminate nested conditionals.
 * - Fan Control: TIM3 CH1 PWM on PA6 (AF2) @ 10 kHz.
 * - Alarm System: PD14 (Active HIGH).
 * - Display: 16x2 LCD configured in 4-bit mode.
 */

#include "App.h"
#include "Adc.h"
#include "Lcd.h"
#include "Gpio.h"
#include "Pwm.h"
#include "Rcc.h"
#include "Timer.h"

/* =========================================================
 * Hardware Mapping & Configuration Constants
 * ========================================================= */

// Alarm LED: Configured for PD14
#define APP_ALARM_LED_PORT      GPIO_D
#define APP_ALARM_LED_PIN       (14U)

// Fan Control: TIM3_CH1 on PA6
#define APP_FAN_TIMER_ID        TIMER3
#define APP_FAN_CHANNEL         PWM_CHANNEL_1
#define APP_FAN_GPIO_PORT       GPIO_A
#define APP_FAN_GPIO_PIN        (6U)
#define APP_FAN_PSC             (15U)   // 16 MHz / 16 = 1 MHz timer tick
#define APP_FAN_ARR             (99U)   // 1 MHz / 100 = 10 kHz PWM frequency

// Pre-defined PWM Duty Cycles
#define APP_FAN_DUTY_OFF        (0U)
#define APP_FAN_DUTY_LOW        (33U)
#define APP_FAN_DUTY_MED        (66U)
#define APP_FAN_DUTY_FULL       (100U)

// Operational Temperature Thresholds (Celsius * 10)
#define APP_THRESH_25_X10       (250)
#define APP_THRESH_30_X10       (300)
#define APP_THRESH_35_X10       (350)
#define APP_THRESH_40_X10       (400)

// ADC Calibration & Scaling
#define APP_ADC_VREF_MV         (3300UL) // System VREF is 3.3V
#define APP_ADC_RESOLUTION      (4096UL) // 12-bit depth
#define APP_ADC_TEMP_MULTIPLIER (6UL)    // Simulation scaling factor

/* =========================================================
 * Asynchronous ADC State variables
 * ========================================================= */

// Volatile flags updated directly by the ADC Interrupt Service Routine
static volatile uint16 Adc_LatestRawSample = 0U;
static volatile uint8  Adc_NewDataAvailable = 0U;

/* =========================================================
 * FSM State Tracking
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

// Function pointer array mapped to AppState_t enumeration
typedef void (*FsmHandler_t)(AppEvent_t event, sint32 currentTempX10);

static const FsmHandler_t FsmDispatchTable[3] =
{
    App_HandleIdle,     // 0: APP_STATE_IDLE
    App_HandleCooling,  // 1: APP_STATE_COOLING
    App_HandleOverheat  // 2: APP_STATE_OVERHEAT
};

/* =========================================================
 * Core Application Interface
 * ========================================================= */

/**
 * @brief Bootstraps the application, configures peripherals, and launches the first ADC read.
 */
void App_Init(void)
{
    // Initialize Alarm LED (Starts OFF)
    Gpio_Init(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, LOW);

    // Initialize Fan PWM GPIO pin
    Gpio_Init(APP_FAN_GPIO_PORT, APP_FAN_GPIO_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(APP_FAN_GPIO_PORT, APP_FAN_GPIO_PIN, GPIO_AF2);

    // Initialize Timer 3 for Fan PWM at 0% Duty Cycle
    Pwm_Init(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, APP_FAN_PSC, APP_FAN_ARR);
    Pwm_SetDutyPercent(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, APP_FAN_DUTY_OFF);
    Pwm_Start(APP_FAN_TIMER_ID, APP_FAN_CHANNEL);

    // Reset FSM
    ActiveState = APP_STATE_IDLE;

    // Render initial UI to clear display artifacts
    Lcd_Clear();
    Lcd_SetCursor(0U, 0U);
    Lcd_Print("Temp:  --.- C   ");
    Lcd_SetCursor(1U, 0U);
    Lcd_Print("Fan:    0%      ");

    // Register our callback and trigger the initial background conversion
    Adc_ReadSingleChannelAsync(App_AdcCallback);
    Adc_StartConversion();
}

/**
 * @brief Main execution routine. Evaluates new sensor data continuously without blocking.
 */
void App_Run(void)
{
    uint16     adcRawRead;
    sint32     currentTempX10;
    AppEvent_t currentEvent;

    // Yield control immediately if no fresh ADC data is present
    if (Adc_NewDataAvailable == 0U)
    {
        return;
    }

    // Capture the data and reset the ISR flag safely
    Adc_NewDataAvailable = 0U;
    adcRawRead           = Adc_LatestRawSample;

    // Immediately trigger the next hardware conversion
    Adc_StartConversion();

    // Process the reading and categorize the event
    currentTempX10 = App_RawToTempCelsius_x10(adcRawRead);
    currentEvent   = App_ClassifyTemperature(currentTempX10);

    // Trigger the appropriate state handler
    FsmDispatchTable[ActiveState](currentEvent, currentTempX10);
}

/* =========================================================
 * Internal Routines
 * ========================================================= */

/**
 * @brief Interrupt callback executed by the ADC driver when conversion completes.
 */
static void App_AdcCallback(uint16 rawResult)
{
    Adc_LatestRawSample  = rawResult;
    Adc_NewDataAvailable = 1U;
}

/**
 * @brief Transforms 12-bit ADC data into a scaled Celsius value.
 */
static sint32 App_RawToTempCelsius_x10(uint16 rawValue)
{
    return ((sint32)rawValue * (sint32)APP_ADC_VREF_MV * (sint32)APP_ADC_TEMP_MULTIPLIER)
           / (sint32)APP_ADC_RESOLUTION;
}

/**
 * @brief Maps the active temperature into a discrete event for the FSM.
 */
static AppEvent_t App_ClassifyTemperature(sint32 currentTempX10)
{
    AppEvent_t event;

    if (currentTempX10 >= APP_THRESH_40_X10)
    {
        event = APP_EVENT_TEMP_ABOVE_40;
    }
    else if (ActiveState == APP_STATE_OVERHEAT)
    {
        // System is cooling down from an overheat scenario
        event = APP_EVENT_TEMP_BELOW_40_RECOVERY;
    }
    else if (currentTempX10 >= APP_THRESH_35_X10)
    {
        event = APP_EVENT_TEMP_35_TO_40;
    }
    else if (currentTempX10 >= APP_THRESH_30_X10)
    {
        event = APP_EVENT_TEMP_30_TO_35;
    }
    else if (currentTempX10 >= APP_THRESH_25_X10)
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
 * @brief Synchronizes the Fan, LED, and LCD to match the active state and event.
 */
static void App_UpdateOutputs(AppState_t state, AppEvent_t event, sint32 currentTempX10)
{
    uint8 targetDuty;

    // Calculate required fan effort based on the thermal event
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

    // Apply speed
    Pwm_SetDutyPercent(APP_FAN_TIMER_ID, APP_FAN_CHANNEL, targetDuty);

    // Manage visual warning indicator
    if (state == APP_STATE_OVERHEAT)
    {
        Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, HIGH);
    }
    else
    {
        Gpio_WritePin(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, LOW);
    }

    // Refresh UI - Line 1
    Lcd_SetCursor(0U, 0U);
    Lcd_Print("Temp:");
    Lcd_PrintTemp(currentTempX10);
    Lcd_Print(" C  ");

    // Refresh UI - Line 2
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

/**
 * @brief Executes logic when the system is resting below 25C.
 */
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

        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            ActiveState = APP_STATE_IDLE;
            App_UpdateOutputs(APP_STATE_IDLE, APP_EVENT_TEMP_BELOW_25, currentTempX10);
            break;

        default:
            break;
    }
}

/**
 * @brief Executes logic when active cooling measures are engaged.
 */
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

        case APP_EVENT_TEMP_BELOW_40_RECOVERY:
            ActiveState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, APP_EVENT_TEMP_35_TO_40, currentTempX10);
            break;

        default:
            break;
    }
}

/**
 * @brief Executes lockdown/alarm logic during critical temperatures.
 */
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

            // Re-evaluate the specific cooling bracket we dropped into
            {
                AppEvent_t recoveryTarget;

                if (currentTempX10 >= APP_THRESH_35_X10)
                {
                    recoveryTarget = APP_EVENT_TEMP_35_TO_40;
                }
                else if (currentTempX10 >= APP_THRESH_30_X10)
                {
                    recoveryTarget = APP_EVENT_TEMP_30_TO_35;
                }
                else if (currentTempX10 >= APP_THRESH_25_X10)
                {
                    recoveryTarget = APP_EVENT_TEMP_25_TO_30;
                }
                else
                {
                    recoveryTarget = APP_EVENT_TEMP_BELOW_25;
                }

                App_UpdateOutputs(APP_STATE_COOLING, recoveryTarget, currentTempX10);
            }
            break;

        default:
            ActiveState = APP_STATE_COOLING;
            App_UpdateOutputs(APP_STATE_COOLING, event, currentTempX10);
            break;
    }
}