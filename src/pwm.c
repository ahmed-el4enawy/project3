#include "pwm.h"
#include "gpio.h"
#include "rcc.h"

#define PWM_ARR  999U   /* Auto-reload value: 16 MHz / (15+1) / (999+1) = 1 kHz */

void PWM_Init(void)
{
    /* 1. Enable TIM3 clock on the APB1 bus via explicit RCC driver. */
    RCC_voidEnableTIM3();

    /* 2. Configure PA6 as Alternate Function 2 (TIM3_CH1) to output the PWM signal. */
    RCC_voidEnableGPIOA();
    GPIO_SetMode(GPIOA_BASE, 6, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA_BASE, 6, 2);

    /* 3. Setup Timer frequency. 
     * F_pwm = F_clock / ((PSC+1) * (ARR+1))
     * 16MHz / (16 * 1000) = 1kHz. */
    TIM3_REG(TIM_PSC_OFFSET)  = 15;                       /* Prescaler: 1MHz clock tick */
    TIM3_REG(TIM_ARR_OFFSET)  = PWM_ARR;                  /* Period: 1000 ticks = 1ms */
    TIM3_REG(TIM_CCR1_OFFSET) = 0;                        /* Initial Duty Cycle: 0% */

    /* Set Channel 1 to PWM Mode 1 (High then Low, bits 110 = 6) and enable preload. */
    TIM3_REG(TIM_CCMR1_OFFSET) = (6U << TIM_CCMR1_OC1M_POS) | TIM_CCMR1_OC1PE;

    /* Enable the output for Channel 1. */
    TIM3_REG(TIM_CCER_OFFSET) = TIM_CCER_CC1E;

    /* Update event to load configuration, then start the timer. */
    TIM3_REG(TIM_EGR_OFFSET) = TIM_EGR_UG;
    TIM3_REG(TIM_CR1_OFFSET) = TIM_CR1_CEN;
}

void PWM_SetDuty(u8 percent)
{
    /* Saturate percentage at 100. */
    if (percent > 100) percent = 100;
    /* Map 0-100% to 0-ARR range. */
    TIM3_REG(TIM_CCR1_OFFSET) = ((u32)percent * PWM_ARR) / 100U;
}
