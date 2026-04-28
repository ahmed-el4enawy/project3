#include "Led.h"
#include "Gpio.h"
#include "Rcc.h"

#define LED_ALARM_PORT GPIOA_BASE
#define LED_ALARM_PIN  5

void Led_AlarmInit(void)
{
    Rcc_EnableGPIOA();
    Gpio_SetMode(LED_ALARM_PORT, LED_ALARM_PIN, GPIO_MODE_OUTPUT);
    Gpio_WritePin(LED_ALARM_PORT, LED_ALARM_PIN, 0); /* Start OFF */
}

void Led_AlarmSetState(uint8 state)
{
    Gpio_WritePin(LED_ALARM_PORT, LED_ALARM_PIN, state);
}