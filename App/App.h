/**
 * @file    App.h
 * @brief   Header file for the Auto-Cooler System Application Layer.
 * @details This file defines the operational states, thermal events, and
 * the public interface for the Mealy Finite State Machine (FSM).
 */

#ifndef APP_H
#define APP_H

#include "Std_Types.h"

/* =========================================================
 * FSM System States
 * ========================================================= */

/**
 * @brief Enumeration for the system's operational states.
 * In a Mealy FSM, outputs are determined by both the current state
 * and the incoming thermal events.
 */
typedef enum
{
    APP_STATE_IDLE     = 0U,  /**< Resting state: Ambient temperature is within safe limits. */
    APP_STATE_COOLING  = 1U,  /**< Active state: Fan speed is adjusted based on thermal demand. */
    APP_STATE_OVERHEAT = 2U   /**< Critical state: Temperature exceeds 40C; Alarm is active. */
} AppState_t;

/* =========================================================
 * FSM Thermal Events
 * ========================================================= */

/**
 * @brief Enumeration for events derived from the ADC temperature sensor readings.
 */
typedef enum
{
    APP_EVENT_TEMP_BELOW_25           = 0U,  /**< Temperature is less than 25°C. */
    APP_EVENT_TEMP_25_TO_30           = 1U,  /**< Temperature is between 25°C and 30°C. */
    APP_EVENT_TEMP_30_TO_35           = 2U,  /**< Temperature is between 30°C and 35°C. */
    APP_EVENT_TEMP_35_TO_40           = 3U,  /**< Temperature is between 35°C and 40°C. */
    APP_EVENT_TEMP_ABOVE_40           = 4U,  /**< Temperature has reached the 40°C threshold. */
    APP_EVENT_TEMP_BELOW_40_RECOVERY  = 5U   /**< Recovery event: Temp dropped below 40°C after an overheat. */
} AppEvent_t;

/* =========================================================
 * Application Logic Interface (Public APIs)
 * ========================================================= */

/**
 * @brief Configures the system peripherals and initial FSM conditions.
 * Sets up Gpios for the Alarm LED, PWM channels for the Fan, and initializes
 * the state to IDLE.
 */
void App_Init(void);

/**
 * @brief Primary task loop for the Auto-Cooler logic.
 * Processes the latest non-blocking ADC samples, evaluates the corresponding
 * thermal event, and triggers the appropriate state transition handler.
 */
void App_Run(void);

#endif /* APP_H */