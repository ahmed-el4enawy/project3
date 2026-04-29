#ifndef LCD_H
#define LCD_H

#include "Std_Types.h"

/* =========================================================
 * Hardware Pin Mapping (GPIOB)
 * ========================================================= */
#define LCD_RS_PORT     GPIO_B
#define LCD_RS_PIN      (0U)    /**< PB0 — Register Select (0=Cmd, 1=Data) */

#define LCD_EN_PORT     GPIO_B
#define LCD_EN_PIN      (1U)    /**< PB1 — Enable pulse                    */

#define LCD_D4_PORT     GPIO_B
#define LCD_D4_PIN      (4U)    /**< PB4 — Data bit 4                      */

#define LCD_D5_PORT     GPIO_B
#define LCD_D5_PIN      (5U)    /**< PB5 — Data bit 5                      */

#define LCD_D6_PORT     GPIO_B
#define LCD_D6_PIN      (6U)    /**< PB6 — Data bit 6                      */

#define LCD_D7_PORT     GPIO_B
#define LCD_D7_PIN      (7U)    /**< PB7 — Data bit 7                      */

/* =========================================================
 * LCD Command Constants
 * ========================================================= */
#define LCD_CMD_CLEAR           (0x01U) /**< Clear display, return home    */
#define LCD_CMD_RETURN_HOME     (0x02U) /**< Return cursor to home         */
#define LCD_CMD_ENTRY_MODE      (0x06U) /**< Increment, no display shift   */
#define LCD_CMD_DISPLAY_ON      (0x0CU) /**< Display ON, cursor OFF        */
#define LCD_CMD_4BIT_2LINE      (0x28U) /**< 4-bit, 2 lines, 5×8 font     */
#define LCD_DDRAM_ROW0          (0x00U) /**< DDRAM base address — row 0    */
#define LCD_DDRAM_ROW1          (0x40U) /**< DDRAM base address — row 1    */
#define LCD_SET_DDRAM_CMD       (0x80U) /**< OR with address to set DDRAM  */

/* =========================================================
 * Public API
 * ========================================================= */

/**
 * @brief  Initialise LCD1602 in 4-bit mode following HD44780 power-on sequence.
 *         Uses delay_ms() — safe before the timer driver is started.
 */
void Lcd_Init(void);

/**
 * @brief  Send a command byte to the LCD controller.
 * @param  cmd  HD44780 command byte.
 */
void Lcd_SendCommand(uint8 cmd);

/**
 * @brief  Send a data (character) byte to the LCD controller.
 * @param  data  ASCII character to display.
 */
void Lcd_SendData(uint8 data);

/**
 * @brief  Clear the display and wait for completion.
 */
void Lcd_Clear(void);

/**
 * @brief  Move cursor to the specified row and column.
 * @param  row  0 = first line, 1 = second line.
 * @param  col  Column position [0..15].
 */
void Lcd_SetCursor(uint8 row, uint8 col);

/**
 * @brief  Print a null-terminated ASCII string starting at the current cursor.
 * @param  str  Pointer to string.
 */
void Lcd_Print(const char *str);

/**
 * @brief  Print a signed integer right-aligned within <width> characters.
 *         No sprintf — manual digit extraction used.
 * @param  value  Integer value to print.
 * @param  width  Total character width (pad with spaces on the left).
 */
void Lcd_PrintInt(sint32 value, uint8 width);

/**
 * @brief  Print temperature formatted as "XX.X" using integer arithmetic.
 *         Example: temp_x10 = 276 → prints "27.6".
 * @param  temp_x10  Temperature in tenths of °C.
 */
void Lcd_PrintTemp(sint32 temp_x10);

#endif /* LCD_H */
