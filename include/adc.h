#ifndef ADC_H
#define ADC_H

#include "STD_TYPES.h"

/* ==================================================================== */
/* ADC & DMA Base Addresses (STM32F401xE)                               */
/* ==================================================================== */
#define ADC1_BASE               (0x40012000U)
#define DMA2_BASE               (0x40026400U)
#define DMA2_STREAM0_BASE       (DMA2_BASE + 0x10U)

/* ADC Register Offsets */
#define ADC_SR_OFFSET           (0x00U)
#define ADC_CR1_OFFSET          (0x04U)
#define ADC_CR2_OFFSET          (0x08U)
#define ADC_SMPR2_OFFSET        (0x10U)
#define ADC_SQR1_OFFSET         (0x2CU)
#define ADC_SQR3_OFFSET         (0x34U)
#define ADC_DR_OFFSET           (0x4CU)

/* DMA Register Offsets */
#define DMA_CR_OFFSET           (0x00U)
#define DMA_NDTR_OFFSET         (0x04U)
#define DMA_PAR_OFFSET          (0x08U)
#define DMA_M0AR_OFFSET         (0x0CU)

/* Macros to access registers */
#define ADC1_REG(OFFSET)        (*((volatile u32 *)((ADC1_BASE) + (OFFSET))))
#define DMA2_S0_REG(OFFSET)     (*((volatile u32 *)((DMA2_STREAM0_BASE) + (OFFSET))))

/* ADC CR2 Bits */
#define ADC_CR2_ADON            (1U << 0)
#define ADC_CR2_CONT            (1U << 1)
#define ADC_CR2_DMA             (1U << 8)
#define ADC_CR2_DDS             (1U << 9)
#define ADC_CR2_SWSTART         (1U << 30)

/* DMA CR Bits */
#define DMA_CR_EN               (1U << 0)
#define DMA_CR_CIRC             (1U << 8)
#define DMA_CR_PSIZE_16BIT      (1U << 11)
#define DMA_CR_MSIZE_16BIT      (1U << 13)
#define DMA_CR_CHSEL_0          (0U << 25)

void ADC_Init(void);
u16  ADC_Read(void);

#endif /* ADC_H */
