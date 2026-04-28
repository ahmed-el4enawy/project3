#include "STD_TYPES.h"
#include "gpio.h"
#include "rcc.h"
#include "adc.h"
#include "pwm.h"
#include "lcd.h"
#include "fsm.h"

/* Alarm LED on PA5 */
#define ALARM_PIN 5

int main(void)
{
    /* Enable GPIO clocks for Port A (Sensors/PWM) and Port B (LCD). */
    RCC_voidEnableGPIOA();
    RCC_voidEnableGPIOB();

    /* Initialize all hardware modules. */
    ADC_Init();    /* Start ADC + DMA for sensor */
    PWM_Init();    /* Start TIM3 for fan control */
    LCD_Init();    /* Initialize LCD in 4-bit mode */

    /* Configure PA5 as a standard push-pull output for the Alarm LED. */
    GPIO_SetMode(GPIOA_BASE, ALARM_PIN, GPIO_MODE_OUTPUT);
    GPIO_WritePin(GPIOA_BASE, ALARM_PIN, 0);

    while (1) {
        /* 1. Read the latest raw 12-bit value from the DMA buffer. */
        u16 raw     = ADC_Read();

        /* 2. Convert raw ADC value to Temperature in 0.1C units.
         * Formula: (ADC_Value * Vref_mV) / ADC_Resolution / LM35_Slope
         * (raw * 3300) / 4096 / 10mV/C = (raw * 330) / 4096.
         * We use (raw * 3300) / 4096 to get 0.1C resolution (e.g., 255 = 25.5C). */
        u16 temp_x10 = (u16)(((u32)raw * 3300U) / 4096U);

        /* 3. Run the Mealy State Machine with the fresh temperature reading. */
        FSM_Update(temp_x10);
    }

    return 0;
}