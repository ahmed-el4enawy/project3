/**
 * @file    App.h
 * @brief   Public interface for the Auto-Cooler Mealy State Machine.
 *          Defines state and event enumerations and the two public functions.
 *
 *  Created on: 2026-04-27
 *  Author    : Team 29
 */

#ifndef APP_H
#define APP_H

#include "Std_Types.h"

/* =========================================================
 * State Machine — States
 * ========================================================= */

/**
 * @brief  Mealy FSM state enumeration.
 */
typedef enum
{
    APP_STATE_IDLE     = 0U,  /**< Fan OFF — temperature is safe      */
    APP_STATE_COOLING  = 1U,  /**< Fan running — active cooling       */
    APP_STATE_OVERHEAT = 2U   /**< T ≥ 40 °C — alarm asserted        */
} AppState_t;

/* =========================================================
 * State Machine — Events
 * ========================================================= */

/**
 * @brief  Mealy FSM event enumeration derived from temperature reading.
 */
typedef enum
{
    APP_EVENT_TEMP_BELOW_25           = 0U,  /**< T < 25 °C                  */
    APP_EVENT_TEMP_25_TO_30           = 1U,  /**< 25 °C ≤ T < 30 °C          */
    APP_EVENT_TEMP_30_TO_35           = 2U,  /**< 30 °C ≤ T < 35 °C          */
    APP_EVENT_TEMP_35_TO_40           = 3U,  /**< 35 °C ≤ T < 40 °C          */
    APP_EVENT_TEMP_ABOVE_40           = 4U,  /**< T ≥ 40 °C                  */
    APP_EVENT_TEMP_BELOW_40_RECOVERY  = 5U   /**< T < 40 °C while in OVERHEAT */
} AppEvent_t;

/* =========================================================
 * Public API
 * ========================================================= */

/**
 * @brief  Initialise application: configure fan PWM and alarm LED GPIO.
 *         Sets initial state to APP_STATE_IDLE with fan at 0% duty.
 */
void App_Init(void);

/**
 * @brief  Execute one iteration of the Mealy state machine.
 *         Call this every time Adc_DataReady is set by the DMA ISR.
 *         Reads the latest ADC value, classifies the temperature, and
 *         dispatches to the appropriate state handler.
 */
void App_Run(void);

#endif /* APP_H */
