/**
 * @file    Lcd.c
 * @brief   Low-level driver for the LCD1602 (HD44780) in 4-bit mode.
 * @details
 * This driver communicates with the LCD using a 4-bit parallel interface.
 * All timing is derived from the SysTick-based delay utility to ensure
 * reliable command processing without blocking interrupts.
 */

#include "Lcd.h"
#include "Gpio.h"
#include "Rcc.h"
#include "Utils.h"

/* =========================================================
 * Private Function Prototypes
 * ========================================================= */
static void Lcd_WriteNibble(uint8 dataNibble);
static void Lcd_WriteByte(uint8 dataVal, uint8 modeBit);

/* =========================================================
 * Low-Level Hardware Helpers
 * ========================================================= */

/**
 * @brief  Sends 4 bits of data to the LCD data lines and triggers the Enable pulse.
 * @param  dataNibble The 4 bits to be sent (bits 0-3 mapped to D4-D7).
 */
static void Lcd_WriteNibble(uint8 dataNibble)
{
    // Map individual bits of the nibble to the designated GPIO pins
    Gpio_WritePin(LCD_D4_PORT, LCD_D4_PIN, (uint8)((dataNibble >> 0U) & 0x01U));
    Gpio_WritePin(LCD_D5_PORT, LCD_D5_PIN, (uint8)((dataNibble >> 1U) & 0x01U));
    Gpio_WritePin(LCD_D6_PORT, LCD_D6_PIN, (uint8)((dataNibble >> 2U) & 0x01U));
    Gpio_WritePin(LCD_D7_PORT, LCD_D7_PIN, (uint8)((dataNibble >> 3U) & 0x01U));

    // Generate Enable Pulse: High-to-Low transition latches the data
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, HIGH);
    delay_ms(1U);   // Conservative delay to satisfy LCD cycle time
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, LOW);
}

/**
 * @brief  Transmits a full 8-bit byte as two consecutive 4-bit nibbles.
 * @param  dataVal  The 8-bit command or character data.
 * @param  modeBit  Selects Register (0 for Command, 1 for Data).
 */
static void Lcd_WriteByte(uint8 dataVal, uint8 modeBit)
{
    // Set Register Select (RS) pin according to mode
    Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, modeBit);

    // Transmit Upper Nibble (Most Significant)
    Lcd_WriteNibble((uint8)(dataVal >> 4U));

    // Transmit Lower Nibble (Least Significant)
    Lcd_WriteNibble((uint8)(dataVal & 0x0FU));

    // Execution time for standard commands
    delay_ms(1U);
}

/* =========================================================
 * Public Driver Interface
 * ========================================================= */

/**
 * @brief  Executes the mandatory hardware initialization sequence for 4-bit mode.
 */
void Lcd_Init(void)
{
    // Ensure the bus clock for GPIOB is active
    Rcc_Enable(RCC_GPIOB);

    // Setup control and data pins as Push-Pull Outputs
    Gpio_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_EN_PORT, LCD_EN_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);

    // Initial hardware reset of control lines
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, LOW);
    Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, LOW);

    // --- HD44780 Power-On Wakeup Sequence ---
    delay_ms(20); // Wait for VDD to stabilize

    Lcd_WriteNibble(0x03);
    delay_ms(5);

    Lcd_WriteNibble(0x03);
    delay_ms(1);

    Lcd_WriteNibble(0x03);
    delay_ms(1);

    // Activate 4-Bit Mode
    Lcd_WriteNibble(0x02);
    delay_ms(1);

    // Configure Display: 2-Lines, 5x8 Dots
    Lcd_SendCommand(LCD_CMD_4BIT_2LINE);

    // Turn Display ON, hide cursor/blink
    Lcd_SendCommand(LCD_CMD_DISPLAY_ON);

    // Auto-increment cursor position
    Lcd_SendCommand(LCD_CMD_ENTRY_MODE);

    // Clear Screen
    Lcd_Clear();
}

/**
 * @brief  Sends a configuration command to the LCD controller.
 */
void Lcd_SendCommand(uint8 cmd)
{
    Lcd_WriteByte(cmd, 0U);
}

/**
 * @brief  Writes a single ASCII character to the current cursor position.
 */
void Lcd_SendData(uint8 data)
{
    Lcd_WriteByte(data, 1U);
}

/**
 * @brief  Wipes the display memory and resets the cursor to (0,0).
 */
void Lcd_Clear(void)
{
    Lcd_SendCommand(LCD_CMD_CLEAR);
    delay_ms(2U); // Clearing requires extra time to process (min 1.52ms)
}

/**
 * @brief  Moves the cursor to a specific coordinate on the 16x2 grid.
 */
void Lcd_SetCursor(uint8 row, uint8 col)
{
    uint8 targetAddress;

    if (row == 0U)
    {
        targetAddress = LCD_DDRAM_ROW0 + col;
    }
    else
    {
        targetAddress = LCD_DDRAM_ROW1 + col;
    }

    Lcd_SendCommand((uint8)(LCD_SET_DDRAM_CMD | targetAddress));
}

/**
 * @brief  Prints a string of characters sequentially.
 */
void Lcd_Print(const char *str)
{
    uint32 i = 0U;

    while (str[i] != '\0')
    {
        Lcd_SendData((uint8)str[i]);
        i++;
    }
}

/**
 * @brief  Helper to display integers with optional padding for alignment.
 */
void Lcd_PrintInt(sint32 value, uint8 width)
{
    uint8  digitBuffer[12U];
    uint8  charPos    = 11U;
    uint8  isNegative  = 0U;
    sint32 magnitude = value;

    digitBuffer[charPos] = '\0';

    if (value < 0)
    {
        isNegative = 1U;
        magnitude  = -value;
    }

    // Convert integer to string in reverse
    if (magnitude == 0)
    {
        charPos--;
        digitBuffer[charPos] = '0';
    }
    else
    {
        while (magnitude > 0)
        {
            charPos--;
            digitBuffer[charPos] = (uint8)('0' + (magnitude % 10));
            magnitude /= 10;
        }
    }

    if (isNegative)
    {
        charPos--;
        digitBuffer[charPos] = '-';
    }

    // Handle right-alignment padding
    uint8 actualLen = (uint8)(11U - charPos);
    sint32 spacePadding = (sint32)width - (sint32)actualLen;

    for (sint32 p = 0; p < spacePadding; p++)
    {
        Lcd_SendData((uint8)' ');
    }

    Lcd_Print((const char *)&digitBuffer[charPos]);
}

/**
 * @brief  Specialized printer for temperature values scaled by 10.
 * Displays '276' as "27.6".
 */
void Lcd_PrintTemp(sint32 temp_x10)
{
    sint32 wholeUnits;
    sint32 fractionalUnits;

    wholeUnits      = temp_x10 / 10;
    fractionalUnits = temp_x10 % 10;

    if (fractionalUnits < 0)
    {
        fractionalUnits = -fractionalUnits;
    }

    // Display the whole number followed by the precision digit
    Lcd_PrintInt(wholeUnits, 2U);
    Lcd_SendData((uint8)'.');
    Lcd_SendData((uint8)('0' + (uint8)fractionalUnits));
}