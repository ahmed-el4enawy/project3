/**
 * @file    App_Config.h
 * @brief   Master Configuration File for the Auto-Cooler System.
 * @details
 * This file serves as the Hardware Abstraction Layer (HAL) for the project.
 * It contains all physical pin mappings, timer parameters, and thermal
 * control thresholds.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* =========================================================
 * Hardware Pin Mappings (Unique Team 27 Design)
 * ========================================================= */

// 1. Analog Temperature Sensor (LM35)
// Connected to PA1 (ADC1_IN1)
#define APP_TEMP_SENSOR_PORT    GPIO_A
#define APP_TEMP_SENSOR_PIN     (1U)

// 2. Cooling Fan Control (PWM Output)
// Configured for PA6 (Timer 3, Channel 1)
#define APP_FAN_GPIO_PORT       GPIO_A
#define APP_FAN_GPIO_PIN        (6U)
#define APP_FAN_TIMER_ID        TIMER3
#define APP_FAN_CHANNEL         PWM_CHANNEL_1

// 3. System Critical Alarm (Red LED)
// Configured for PD14
#define APP_ALARM_LED_PORT      GPIO_D
#define APP_ALARM_LED_PIN       (14U)

// 4. LCD1602 Interface (4-bit Mode)
// Pins PB8-PB15 are used to avoid BOOT1 (PB2) and SWD Debug (PB3/PB4) interference.
#define LCD_RS_PORT             GPIO_B
#define LCD_RS_PIN              (8U)
#define LCD_EN_PORT             GPIO_B
#define LCD_EN_PIN              (9U)

#define LCD_D4_PORT             GPIO_B
#define LCD_D4_PIN              (12U)
#define LCD_D5_PORT             GPIO_B
#define LCD_D5_PIN              (13U)
#define LCD_D6_PORT             GPIO_B
#define LCD_D6_PIN              (14U)
#define LCD_D7_PORT             GPIO_B
#define LCD_D7_PIN              (15U)

/* =========================================================
 * Timer & PWM Constants
 * ========================================================= */

// PWM Frequency set to 10 kHz (PSC=15, ARR=99 @ 16 MHz)
#define APP_FAN_PSC             (15U)
#define APP_FAN_ARR             (99U)

// Duty Cycle Levels (%)
#define APP_FAN_DUTY_OFF        (0U)
#define APP_FAN_DUTY_LOW        (33U)
#define APP_FAN_DUTY_MED        (66U)
#define APP_FAN_DUTY_FULL       (100U)

/* =========================================================
 * Thermal Control Policy (Celsius * 10)
 * ========================================================= */

#define APP_THRESH_SAFE         (250)   // Fan starts at 25.0 C
#define APP_THRESH_MID          (300)   // Medium speed at 30.0 C
#define APP_THRESH_HIGH         (350)   // High speed at 35.0 C
#define APP_THRESH_CRITICAL     (400)   // Overheat Alarm at 40.0 C

/* =========================================================
 * ADC Calibration & Conversion Settings
 * ========================================================= */

#define APP_ADC_VREF_MV         (3300UL) // Logic level 3.3V
#define APP_ADC_RESOLUTION      (4096UL) // 12-bit ADC depth
#define APP_ADC_TEMP_MULTIPLIER (6UL)    // Tuned scaling factor for simulation

#endif /* APP_CONFIG_H */