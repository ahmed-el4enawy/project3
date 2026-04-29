/**
 * @file    Lcd.h
 * @brief   Header file for the LCD1602 Driver (4-bit Parallel Mode).
 * @details
 * This driver provides a high-level interface for the HD44780-based LCD1602.
 * It is optimized for 4-bit communication to save GPIO pins on the STM32F401.
 */

#ifndef LCD_H
#define LCD_H

#include "Std_Types.h"

/* =========================================================
 * Hardware Connection Mapping (GPIOB)
 * ========================================================= */
 
// Control Pins
#define LCD_RS_PORT     GPIO_B
#define LCD_RS_PIN      (0U)    // Register Select: 0 for Instruction, 1 for Data

#define LCD_EN_PORT     GPIO_B
#define LCD_EN_PIN      (1U)    // Enable: Latches data on falling edge

// Data Bus (4-bit Mode)
#define LCD_D4_PORT     GPIO_B
#define LCD_D4_PIN      (4U)    

#define LCD_D5_PORT     GPIO_B
#define LCD_D5_PIN      (5U)    

#define LCD_D6_PORT     GPIO_B
#define LCD_D6_PIN      (6U)    

#define LCD_D7_PORT     GPIO_B
#define LCD_D7_PIN      (7U)    

/* =========================================================
 * LCD Controller Command Set (HD44780)
 * ========================================================= */
#define LCD_CMD_CLEAR           (0x01U) // Wipes display and resets cursor
#define LCD_CMD_RETURN_HOME     (0x02U) // Returns cursor to (0,0) without clearing
#define LCD_CMD_ENTRY_MODE      (0x06U) // Sets cursor direction to right
#define LCD_CMD_DISPLAY_ON      (0x0CU) // Turns on pixels, hides cursor and blink
#define LCD_CMD_4BIT_2LINE      (0x28U) // Configures 4-bit bus, 2-line display, 5x8 dots
#define LCD_DDRAM_ROW0          (0x00U) // Memory address start for Line 1
#define LCD_DDRAM_ROW1          (0x40U) // Memory address start for Line 2
#define LCD_SET_DDRAM_CMD       (0x80U) // Base command for setting DDRAM address

/* =========================================================
 * Driver Public Interface
 * ========================================================= */

/**
 * @brief  Initializes the LCD hardware using the 4-bit startup sequence.
 * Includes the necessary power-up delays for the HD44780 controller.
 */
void Lcd_Init(void);

/**
 * @brief  Sends a low-level instruction command to the LCD.
 * @param  cmd The 8-bit command hex value.
 */
void Lcd_SendCommand(uint8 cmd);

/**
 * @brief  Sends a single ASCII character to be displayed.
 * @param  data The ASCII value to write to DDRAM.
 */
void Lcd_SendData(uint8 data);

/**
 * @brief  Clears all text from the screen and resets the cursor.
 */
void Lcd_Clear(void);

/**
 * @brief  Positions the cursor at a specific line and column.
 * @param  row Line index (0 for Top, 1 for Bottom).
 * @param  col Column index (0 to 15).
 */
void Lcd_SetCursor(uint8 row, uint8 col);

/**
 * @brief  Displays a full string starting from the current cursor position.
 * @param  str Pointer to the null-terminated character array.
 */
void Lcd_Print(const char *str);

/**
 * @brief  Prints a signed integer value with alignment.
 * Useful for displaying sensor readings without using 'sprintf'.
 * @param  value The signed 32-bit integer to display.
 * @param  width Total number of spaces on screen (right-aligned).
 */
void Lcd_PrintInt(sint32 value, uint8 width);

/**
 * @brief  Converts and prints a temperature value scaled by 10.
 * Useful for avoiding floating-point math overhead.
 * @param  temp_x10 Temperature in Celsius * 10 (e.g., 25.5C = 255).
 */
void Lcd_PrintTemp(sint32 temp_x10);

#endif /* LCD_H */