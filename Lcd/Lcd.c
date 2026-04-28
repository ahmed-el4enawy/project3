#include "Lcd.h"
#include "Gpio.h"
#include "Rcc.h"
#include "SysTick.h"
#include "App_Config.h"

static void Lcd_EnablePulse(void)
{
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_EN_PIN, 1);
    SysTick_Delay_us(2);
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_EN_PIN, 0);
    SysTick_Delay_us(50);
}

static void Lcd_SendNibble(uint8 nibble)
{
    
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_D4_PIN + 0, (nibble >> 0) & 1U);
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_D4_PIN + 1, (nibble >> 1) & 1U);
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_D4_PIN + 2, (nibble >> 2) & 1U);
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_D4_PIN + 3, (nibble >> 3) & 1U);
    Lcd_EnablePulse();
}

static void Lcd_Cmd(uint8 cmd)
{
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_RS_PIN, 0);
    Lcd_SendNibble(cmd >> 4);
    Lcd_SendNibble(cmd & 0x0F);
    SysTick_Delay_ms(2);
}

static void Lcd_Data(uint8 data)
{
    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_RS_PIN, 1);
    Lcd_SendNibble(data >> 4);
    Lcd_SendNibble(data & 0x0F);
    SysTick_Delay_us(50);
}

void Lcd_Init(void)
{
    Rcc_EnableGPIOB();


    Gpio_SetMode(CONFIG_LCD_PORT, CONFIG_LCD_RS_PIN, GPIO_MODE_OUTPUT);
    Gpio_SetMode(CONFIG_LCD_PORT, CONFIG_LCD_EN_PIN, GPIO_MODE_OUTPUT);
    for (uint8 i = 0; i < 4; i++) {
        Gpio_SetMode(CONFIG_LCD_PORT, CONFIG_LCD_D4_PIN + i, GPIO_MODE_OUTPUT);
    }

    SysTick_Delay_ms(50);

    Gpio_WritePin(CONFIG_LCD_PORT, CONFIG_LCD_RS_PIN, 0);
    Lcd_SendNibble(0x03);
    SysTick_Delay_ms(5);
    Lcd_SendNibble(0x03);
    SysTick_Delay_us(150);
    Lcd_SendNibble(0x03);
    SysTick_Delay_us(150);
    Lcd_SendNibble(0x02);
    SysTick_Delay_us(150);

    Lcd_Cmd(0x28);
    Lcd_Cmd(0x0C);
    Lcd_Cmd(0x06);
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