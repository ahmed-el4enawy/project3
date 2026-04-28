#ifndef RCC_H
#define RCC_H

#include "STD_TYPES.h"
#include "BIT_MATH.h"

/* ==================================================================== */
/* RCC Register Base Address (STM32F401xE)                              */
/* ==================================================================== */
#define RCC_BASE_ADDR           (0x40023800U)

/* RCC Register Offsets */
#define RCC_AHB1ENR_OFFSET      (0x30U)
#define RCC_APB1ENR_OFFSET      (0x40U)
#define RCC_APB2ENR_OFFSET      (0x44U)

/* Macro to access RCC registers */
#define RCC_REG(OFFSET)         (*((volatile u32 *)((RCC_BASE_ADDR) + (OFFSET))))

/* AHB1ENR bit positions */
#define RCC_AHB1ENR_GPIOAEN_BIT (0U)   /* GPIOA clock enable bit */
#define RCC_AHB1ENR_GPIOBEN_BIT (1U)   /* GPIOB clock enable bit */
#define RCC_AHB1ENR_DMA2EN_BIT  (22U)  /* DMA2 clock enable bit */

/* APB1ENR bit positions */
#define RCC_APB1ENR_TIM3EN_BIT  (1U)   /* TIM3 clock enable bit */

/* APB2ENR bit positions */
#define RCC_APB2ENR_ADC1EN_BIT  (8U)   /* ADC1 clock enable bit */

/* ==================================================================== */
/* Public Function Prototypes                                            */
/* ==================================================================== */

/**
 * @brief Enables the peripheral clock for GPIOA on the AHB1 bus.
 */
void RCC_voidEnableGPIOA(void);

/**
 * @brief Enables the peripheral clock for GPIOB on the AHB1 bus.
 */
void RCC_voidEnableGPIOB(void);

/**
 * @brief Enables the peripheral clock for DMA2 on the AHB1 bus.
 */
void RCC_voidEnableDMA2(void);

/**
 * @brief Enables the peripheral clock for TIM3 on the APB1 bus.
 */
void RCC_voidEnableTIM3(void);

/**
 * @brief Enables the peripheral clock for ADC1 on the APB2 bus.
 */
void RCC_voidEnableADC1(void);

#endif /* RCC_H */
