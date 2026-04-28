#include "lcd.h"
#include "gpio.h"
#include "rcc.h"

/*
 * LCD1602 in 4-bit mode on GPIOB:
 *   PB0 = RS
 *   PB1 = EN
 *   PB2 = D4
 *   PB3 = D5
 *   PB4 = D6
 *   PB5 = D7
 */

#define LCD_PORT_BASE  GPIOB_BASE
#define LCD_RS         0
#define LCD_EN         1
#define LCD_D4         2

/* ---- internal helpers ---- */

static void delay_us(u32 us)
{
    /* ~16 cycles per iteration at 16 MHz ≈ 1 µs */
    for (volatile u32 i = 0; i < us * 4; i++);
}

static void delay_ms(u32 ms)
{
    delay_us(ms * 1000);
}

static void LCD_EnablePulse(void)
{
    GPIO_WritePin(LCD_PORT_BASE, LCD_EN, 1);
    delay_us(2);
    GPIO_WritePin(LCD_PORT_BASE, LCD_EN, 0);
    delay_us(50);
}

static void LCD_SendNibble(u8 nibble)
{
    /* Write upper nibble of 'nibble' param to D4-D7 (PB2-PB5) */
    GPIO_WritePin(LCD_PORT_BASE, LCD_D4 + 0, (nibble >> 0) & 1);
    GPIO_WritePin(LCD_PORT_BASE, LCD_D4 + 1, (nibble >> 1) & 1);
    GPIO_WritePin(LCD_PORT_BASE, LCD_D4 + 2, (nibble >> 2) & 1);
    GPIO_WritePin(LCD_PORT_BASE, LCD_D4 + 3, (nibble >> 3) & 1);
    LCD_EnablePulse();
}

static void LCD_SendByte(u8 byte, u8 rs)
{
    GPIO_WritePin(LCD_PORT_BASE, LCD_RS, rs);
    LCD_SendNibble(byte >> 4);    /* High nibble first */
    LCD_SendNibble(byte & 0x0F);  /* Low nibble        */
    delay_us(50);
}

static void LCD_Cmd(u8 cmd)
{
    LCD_SendByte(cmd, 0);
}

static void LCD_Data(u8 data)
{
    LCD_SendByte(data, 1);
}

/* ---- public API ---- */

void LCD_Init(void)
{
    /* Enable clock for the port used by LCD (GPIOB). */
    RCC_voidEnableGPIOB();

    /* Set pins PB0-PB5 as output for RS, EN, and Data bits. */
    for (u8 i = 0; i <= 5; i++) {
        GPIO_SetMode(LCD_PORT_BASE, i, GPIO_MODE_OUTPUT);
    }

    /* Standard HD44780 startup sequence for 4-bit mode. 
     * We send specific values to tell the controller we are using 4 bits. */
    delay_ms(50);                 /* Wait >40ms after power-on */

    GPIO_WritePin(LCD_PORT_BASE, LCD_RS, 0);
    LCD_SendNibble(0x03);         /* Function set: 8-bit */
    delay_ms(5);
    LCD_SendNibble(0x03);         /* Repeat */
    delay_us(150);
    LCD_SendNibble(0x03);         /* Repeat */
    delay_us(150);
    LCD_SendNibble(0x02);         /* Switch to 4-bit mode */
    delay_us(150);

    LCD_Cmd(0x28);                /* 4-bit mode, 2 lines, 5x8 font */
    LCD_Cmd(0x0C);                /* Display ON, Cursor OFF */
    LCD_Cmd(0x06);                /* Entry mode: increment cursor */
    LCD_Cmd(0x01);                /* Clear display */
    delay_ms(2);                  /* Clearing is slow (~1.5ms) */
}

void LCD_Clear(void)
{
    LCD_Cmd(0x01);                /* Send clear command */
    delay_ms(2);                  
}

void LCD_SetCursor(u8 row, u8 col)
{
    /* DDRAM addresses: Row 0 starts at 0x00, Row 1 at 0x40. */
    u8 addr = col + (row == 0 ? 0x00 : 0x40);
    LCD_Cmd(0x80 | addr);         /* Set DDRAM Address command */
}

void LCD_Print(const char *str)
{
    /* Send each character one by one as data (RS=1). */
    while (*str) {
        LCD_Data((u8)*str++);
    }
}
