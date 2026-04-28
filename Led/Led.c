#include "Led.h"
#include "Gpio.h"
#include "Rcc.h"
#include "App_Config.h"

void Led_AlarmInit(void)
{
    Rcc_EnableGPIOA();

    Gpio_SetMode(CONFIG_LED_ALARM_PORT, CONFIG_LED_ALARM_PIN, GPIO_MODE_OUTPUT);
    Gpio_WritePin(CONFIG_LED_ALARM_PORT, CONFIG_LED_ALARM_PIN, 0);
}

void Led_AlarmSetState(uint8 state)
{
    Gpio_WritePin(CONFIG_LED_ALARM_PORT, CONFIG_LED_ALARM_PIN, state);
}