/**
 * @file    Lcd.c
 * @brief   HD44780 LCD1602 driver in 4-bit parallel mode.
 *          Uses only the Gpio driver API — no HAL, no register writes in this file.
 *          Timing provided by delay_ms() from Utils.h.
 *

 */

#include "Lcd.h"
#include "Gpio.h"
#include "Rcc.h"
#include "Utils.h"

/* =========================================================
 * Private function prototypes
 * ========================================================= */
static void Lcd_SendNibble(uint8 nibble);
static void Lcd_SendByte(uint8 byte, uint8 isData);

/* =========================================================
 * Private helpers
 * ========================================================= */

/**
 * @brief  Drive D4–D7 with bits [3:0] of nibble and pulse the EN line.
 *         Bit 0 of nibble → D4, bit 1 → D5, bit 2 → D6, bit 3 → D7.
 */
static void Lcd_SendNibble(uint8 nibble)
{
    /* --- Drive data pins from the lower 4 bits of nibble --- */
    Gpio_WritePin(LCD_D4_PORT, LCD_D4_PIN, (uint8)((nibble >> 0U) & 0x01U));
    Gpio_WritePin(LCD_D5_PORT, LCD_D5_PIN, (uint8)((nibble >> 1U) & 0x01U));
    Gpio_WritePin(LCD_D6_PORT, LCD_D6_PIN, (uint8)((nibble >> 2U) & 0x01U));
    Gpio_WritePin(LCD_D7_PORT, LCD_D7_PIN, (uint8)((nibble >> 3U) & 0x01U));

    /* --- Pulse EN: HIGH → short delay → LOW --- */
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, HIGH);
    delay_ms(1U);   /* ≥ 1 µs required; 1 ms is conservative but safe */
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, LOW);
}

/**
 * @brief  Send a full byte to the LCD as two 4-bit nibbles.
 *         Sets RS based on isData, then sends upper nibble then lower nibble.
 * @param  byte    8-bit data or command value.
 * @param  isData  1 = data (RS=HIGH), 0 = command (RS=LOW).
 */
static void Lcd_SendByte(uint8 byte, uint8 isData)
{
    Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, isData);

    /* Send upper nibble first */
    Lcd_SendNibble((uint8)(byte >> 4U));

    /* Send lower nibble */
    Lcd_SendNibble((uint8)(byte & 0x0FU));

    /* Wait for LCD to process the command (≥ 37 µs; 1 ms is safe) */
    delay_ms(1U);
}

/* =========================================================
 * Public API
 * ========================================================= */

/**
 * @brief  Initialise LCD1602 in 4-bit mode using the HD44780 power-up sequence.
 */
void Lcd_Init(void)
{
    /* Enable GPIOB clock (RCC_GPIOB enabled by main, but safe to call again) */
    Rcc_Enable(RCC_GPIOB);

    /* Initialise all LCD GPIO pins as push-pull outputs, defaulting LOW */
    Gpio_Init(LCD_RS_PORT, LCD_RS_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_EN_PORT, LCD_EN_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D4_PORT, LCD_D4_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D5_PORT, LCD_D5_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D6_PORT, LCD_D6_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_D7_PORT, LCD_D7_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);

    /* Ensure EN and RS start LOW */
    Gpio_WritePin(LCD_EN_PORT, LCD_EN_PIN, LOW);
    Gpio_WritePin(LCD_RS_PORT, LCD_RS_PIN, LOW);

    /* ---- HD44780 4-bit initialisation sequence ---- */

    /* Step 1: Wait ≥ 15 ms after VDD rises above 4.5 V (or 2.7 V for 3 V LCD) */
    delay_ms(15U);

    /* Step 2: Send 0x3 nibble, wait ≥ 4.1 ms */
    Lcd_SendNibble(0x3U);
    delay_ms(5U);

    /* Step 3: Send 0x3 nibble again, wait ≥ 100 µs */
    Lcd_SendNibble(0x3U);
    delay_ms(1U);

    /* Step 4: Send 0x3 nibble a third time */
    Lcd_SendNibble(0x3U);
    delay_ms(1U);

    /* Step 5: Switch to 4-bit mode */
    Lcd_SendNibble(0x2U);
    delay_ms(1U);

    /* Step 6: Function set — 4-bit bus, 2 display lines, 5×8 font */
    Lcd_SendCommand(LCD_CMD_4BIT_2LINE);

    /* Step 7: Display ON, cursor OFF, blink OFF */
    Lcd_SendCommand(LCD_CMD_DISPLAY_ON);

    /* Step 8: Entry mode — increment cursor, no display shift */
    Lcd_SendCommand(LCD_CMD_ENTRY_MODE);

    /* Step 9: Clear display */
    Lcd_SendCommand(LCD_CMD_CLEAR);
    delay_ms(2U);   /* clear command needs ≥ 1.52 ms */
}

/**
 * @brief  Send a command byte to the LCD.
 */
void Lcd_SendCommand(uint8 cmd)
{
    Lcd_SendByte(cmd, 0U);  /* RS = LOW for commands */
}

/**
 * @brief  Send a character data byte to the LCD.
 */
void Lcd_SendData(uint8 data)
{
    Lcd_SendByte(data, 1U); /* RS = HIGH for data */
}

/**
 * @brief  Clear the display and wait the required time.
 */
void Lcd_Clear(void)
{
    Lcd_SendCommand(LCD_CMD_CLEAR);
    delay_ms(2U);
}

/**
 * @brief  Set cursor position by driving the DDRAM address.
 */
void Lcd_SetCursor(uint8 row, uint8 col)
{
    uint8 ddramAddr;

    if (row == 0U)
    {
        ddramAddr = LCD_DDRAM_ROW0 + col;
    }
    else
    {
        ddramAddr = LCD_DDRAM_ROW1 + col;
    }

    Lcd_SendCommand((uint8)(LCD_SET_DDRAM_CMD | ddramAddr));
}

/**
 * @brief  Print a null-terminated string at the current cursor position.
 */
void Lcd_Print(const char *str)
{
    uint8 idx = 0U;

    while (str[idx] != '\0')
    {
        Lcd_SendData((uint8)str[idx]);
        idx++;
    }
}

/**
 * @brief  Print a signed integer right-aligned in <width> characters.
 *         Handles negative values and pads with spaces on the left.
 */
void Lcd_PrintInt(sint32 value, uint8 width)
{
    /* Local buffer: max sint32 has 10 digits + sign + null = 12 chars */
    uint8  buf[12U];
    uint8  pos    = 11U;
    uint8  isNeg  = 0U;
    sint32 absVal = value;
    uint8  numLen;
    sint32 padCount;
    sint32 i;

    buf[pos] = '\0';

    if (value < 0)
    {
        isNeg  = 1U;
        absVal = -value;
    }

    /* Extract digits from least significant to most significant */
    if (absVal == 0)
    {
        pos--;
        buf[pos] = '0';
    }
    else
    {
        while (absVal > 0)
        {
            pos--;
            buf[pos] = (uint8)('0' + (absVal % 10));
            absVal /= 10;
        }
    }

    if (isNeg)
    {
        pos--;
        buf[pos] = '-';
    }

    /* Calculate actual number length (11 - pos, since null is at index 11) */
    numLen   = (uint8)(11U - pos);
    padCount = (sint32)width - (sint32)numLen;

    /* Print leading spaces for right-alignment */
    for (i = 0; i < padCount; i++)
    {
        Lcd_SendData((uint8)' ');
    }

    /* Print the number string */
    Lcd_Print((const char *)&buf[pos]);
}

/**
 * @brief  Print temperature in format "XX.X" using integer math.
 *         Example: temp_x10 = 276 → displays "27.6".
 */
void Lcd_PrintTemp(sint32 temp_x10)
{
    sint32 intPart;
    sint32 decPart;

    intPart = temp_x10 / 10;
    decPart = temp_x10 % 10;

    /* Ensure decimal part is positive (handles edge case of negative temps) */
    if (decPart < 0)
    {
        decPart = -decPart;
    }

    /* Print integer part (no leading spaces — use exact digits) */
    Lcd_PrintInt(intPart, 2U);

    /* Print decimal separator */
    Lcd_SendData((uint8)'.');

    /* Print single decimal digit */
    Lcd_SendData((uint8)('0' + (uint8)decPart));
}
