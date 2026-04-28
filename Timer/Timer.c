#include "Timer.h"
#include "Timer_Private.h"
#include "Rcc.h"
#include "Gpio.h"

void Timer_InitPWM(void)
{
    Rcc_EnableTIM3();
    Rcc_EnableGPIOA();
    
    /* PA6 as Alternate Function 2 (TIM3_CH1) */
    Gpio_SetMode(GPIOA_BASE, 6, GPIO_MODE_AF);
    Gpio_SetAF(GPIOA_BASE, 6, 2);

    /* 16MHz / (15+1) = 1MHz tick. ARR = 999 -> 1kHz */
    TIM3->PSC = 15;
    TIM3->ARR = 999;
    TIM3->CCR1 = 0;

    /* PWM Mode 1 on Channel 1 */
    TIM3->CCMR1 = (6UL << 4);
    /* Enable Channel 1 Output */
    TIM3->CCER = (1UL << 0);
    /* Enable Timer */
    TIM3->CR1 = (1UL << 0);
}

void Timer_SetPWMDuty(uint8 duty)
{
    if (duty > 100U) {
        duty = 100U;
    }
    
    uint32 ccr_val = (duty * 1000U) / 100U;
    
    if (ccr_val > 0U) {
        ccr_val -= 1U;
    }
    
    TIM3->CCR1 = ccr_val;
}