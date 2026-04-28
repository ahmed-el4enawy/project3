#include "Lcd.h"
#include "Gpio.h"
#include "Rcc.h"
#include "SysTick.h"

/* * LCD1602 in 4-bit mode on GPIOB:
 * PB0=RS, PB1=EN, PB2=D4, PB3=D5, PB4=D6, PB5=D7
 */
#define LCD_PORT_BASE  GPIOB_BASE
#define LCD_RS         0
#define LCD_EN         1
#define LCD_D4         2

static void Lcd_EnablePulse(void)
{
    Gpio_WritePin(LCD_PORT_BASE, LCD_EN, 1);
    SysTick_Delay_us(2); /* Exact hardware 2us delay */
    Gpio_WritePin(LCD_PORT_BASE, LCD_EN, 0);
    SysTick_Delay_us(50);
}

static void Lcd_SendNibble(uint8 nibble)
{
    Gpio_WritePin(LCD_PORT_BASE, LCD_D4 + 0, (nibble >> 0) & 1U);
    Gpio_WritePin(LCD_PORT_BASE, LCD_D4 + 1, (nibble >> 1) & 1U);
    Gpio_WritePin(LCD_PORT_BASE, LCD_D4 + 2, (nibble >> 2) & 1U);
    Gpio_WritePin(LCD_PORT_BASE, LCD_D4 + 3, (nibble >> 3) & 1U);
    Lcd_EnablePulse();
}

static void Lcd_Cmd(uint8 cmd)
{
    Gpio_WritePin(LCD_PORT_BASE, LCD_RS, 0);
    Lcd_SendNibble(cmd >> 4);
    Lcd_SendNibble(cmd & 0x0F);
    SysTick_Delay_ms(2);
}

static void Lcd_Data(uint8 data)
{
    Gpio_WritePin(LCD_PORT_BASE, LCD_RS, 1);
    Lcd_SendNibble(data >> 4);
    Lcd_SendNibble(data & 0x0F);
    SysTick_Delay_us(50);
}

void Lcd_Init(void)
{
    Rcc_EnableGPIOB();

    for (uint8 i = 0; i <= 5; i++) {
        Gpio_SetMode(LCD_PORT_BASE, i, GPIO_MODE_OUTPUT);
    }

    SysTick_Delay_ms(50); /* Power-on hardware delay */

    Gpio_WritePin(LCD_PORT_BASE, LCD_RS, 0);
    Lcd_SendNibble(0x03);
    SysTick_Delay_ms(5);
    Lcd_SendNibble(0x03);
    SysTick_Delay_us(150);
    Lcd_SendNibble(0x03);
    SysTick_Delay_us(150);
    Lcd_SendNibble(0x02); /* Switch to 4-bit mode */
    SysTick_Delay_us(150);

    Lcd_Cmd(0x28); /* 4-bit mode, 2 lines, 5x8 font */
    Lcd_Cmd(0x0C); /* Display ON, Cursor OFF */
    Lcd_Cmd(0x06); /* Entry mode: increment cursor */
    Lcd_Clear();
}

void Lcd_Clear(void)
{
    Lcd_Cmd(0x01);
    SysTick_Delay_ms(2);
}

void Lcd_SetCursor(uint8 row, uint8 col)
{
    uint8 address = (row == 0) ? (0x80 + col) : (0xC0 + col);
    Lcd_Cmd(address);
}

void Lcd_Print(const char *str)
{
    while (*str) {
        Lcd_Data((uint8)*str);
        str++;
    }
}