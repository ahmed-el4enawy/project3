#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* =========================================================
 * Hardware Pin Mappings (The Master Control Panel)
 * ========================================================= */

// 1. Temperature Sensor (LM35) - PA1
#define APP_TEMP_SENSOR_PORT    GPIO_A
#define APP_TEMP_SENSOR_PIN     (1U)

// 2. Cooling Fan (PWM) - PA6 (TIM3 CH1)
#define APP_FAN_GPIO_PORT       GPIO_A
#define APP_FAN_GPIO_PIN        (6U)
#define APP_FAN_TIMER_ID        TIMER3
#define APP_FAN_CHANNEL         PWM_CHANNEL_1

// 3. Alarm LED - PD14
#define APP_ALARM_LED_PORT      GPIO_D
#define APP_ALARM_LED_PIN       (14U)

// 4. LCD1602 (4-bit Mode) - Port B
#define LCD_RS_PORT             GPIO_B
#define LCD_RS_PIN              (0U)
#define LCD_EN_PORT             GPIO_B
#define LCD_EN_PIN              (1U)
#define LCD_D4_PORT             GPIO_B
#define LCD_D4_PIN              (4U)
#define LCD_D5_PORT             GPIO_B
#define LCD_D5_PIN              (5U)
#define LCD_D6_PORT             GPIO_B
#define LCD_D6_PIN              (6U)
#define LCD_D7_PORT             GPIO_B
#define LCD_D7_PIN              (7U)

/* =========================================================
 * System Constants & Policies
 * ========================================================= */

// Fan Performance Settings
#define APP_FAN_PSC             (15U)   // 10 kHz PWM Frequency
#define APP_FAN_ARR             (99U)
#define APP_FAN_DUTY_OFF        (0U)
#define APP_FAN_DUTY_LOW        (33U)
#define APP_FAN_DUTY_MED        (66U)
#define APP_FAN_DUTY_FULL       (100U)

// Thermal Control Thresholds (C * 10)
#define APP_THRESH_SAFE         (250)
#define APP_THRESH_MID          (300)
#define APP_THRESH_HIGH         (350)
#define APP_THRESH_CRITICAL     (400)

// ADC Calibration
#define APP_ADC_VREF_MV         (3300UL)
#define APP_ADC_RESOLUTION      (4096UL)
#define APP_ADC_TEMP_MULTIPLIER (6UL)

#endif /* APP_CONFIG_H */