#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "Gpio.h" /* To know GPIOA_BASE, etc. */

/* ================================================================= */
/* 1. SYSTEM LOGIC THRESHOLDS (FSM)                                  */
/* Temperature thresholds in 0.1 C units (e.g., 250 = 25.0 C)        */
/* ================================================================= */
#define CONFIG_THRESH_LOW   250U   /* 25.0 C -> Enter Cooling */
#define CONFIG_THRESH_MID   300U   /* 30.0 C -> Fan 33% to 66% */
#define CONFIG_THRESH_HIGH  350U   /* 35.0 C -> Fan 66% to 100% */
#define CONFIG_THRESH_OVER  400U   /* 40.0 C -> SYSTEM OVERHEAT! */

/* ================================================================= */
/* 2. HARDWARE PIN MAPPINGS                                          */
/* ================================================================= */

/* ---- Alarm LED Configuration ---- */
#define CONFIG_LED_ALARM_PORT  GPIOA_BASE
#define CONFIG_LED_ALARM_PIN   5

/* ---- LCD 1602 Configuration (4-bit mode) ---- */
#define CONFIG_LCD_PORT        GPIOB_BASE
#define CONFIG_LCD_RS_PIN      0
#define CONFIG_LCD_EN_PIN      1
#define CONFIG_LCD_D4_PIN      2
/* Note: D5, D6, D7 must be sequentially wired to Pins 3, 4, 5 */

#endif /* APP_CONFIG_H */