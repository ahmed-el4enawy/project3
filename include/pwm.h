#ifndef PWM_H
#define PWM_H

#include "STD_TYPES.h"

/* ==================================================================== */
/* TIM3 Base Address (STM32F401xE)                                      */
/* ==================================================================== */
#define TIM3_BASE               (0x40000400U)

/* TIM3 Register Offsets */
#define TIM_CR1_OFFSET          (0x00U)
#define TIM_EGR_OFFSET          (0x14U)
#define TIM_CCMR1_OFFSET        (0x18U)
#define TIM_CCER_OFFSET         (0x20U)
#define TIM_PSC_OFFSET          (0x28U)
#define TIM_ARR_OFFSET          (0x2CU)
#define TIM_CCR1_OFFSET         (0x34U)

/* Macro to access TIM3 registers */
#define TIM3_REG(OFFSET)        (*((volatile u32 *)((TIM3_BASE) + (OFFSET))))

/* TIM3 CR1 Bits */
#define TIM_CR1_CEN             (1U << 0)

/* TIM3 EGR Bits */
#define TIM_EGR_UG              (1U << 0)

/* TIM3 CCMR1 Bits */
#define TIM_CCMR1_OC1PE         (1U << 3)
#define TIM_CCMR1_OC1M_POS      (4U)

/* TIM3 CCER Bits */
#define TIM_CCER_CC1E           (1U << 0)

void PWM_Init(void);
void PWM_SetDuty(u8 percent);

#endif /* PWM_H */
