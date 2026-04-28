#include "Std_Types.h"
#include "SysTick.h"
#include "Adc.h"
#include "Timer.h"
#include "Lcd.h"
#include "Led.h"
#include "Fsm.h"

int main(void)
{
    /* 1. Initialize Hardware Components */
    SysTick_Init();
    Adc_Init();        /* Starts ADC & DMA background scanning */
    Timer_InitPWM();   /* TIM3 for Fan Control */
    Led_AlarmInit();   /* PA5 for Alarm */
    Lcd_Init();        /* PB0-PB5 for LCD Screen */

    /* 2. Main Infinite Control Loop */
    while (1) {
        /* Read latest background DMA value (No CPU Polling) */
        uint16 raw_adc = Adc_Read();

        /* Convert to 0.1 C format (Fixed-Point Math)
         * Formula: (raw * 3300mV) / 4096 / 10 = (raw * 330) / 4096 */
        uint16 temp_x10 = (raw_adc * 3300U) / 4096U;

        /* Execute State Machine Logic */
        FSM_Update(temp_x10);

        /* System tick to prevent LCD flickering */
        SysTick_Delay_ms(200);
    }
}