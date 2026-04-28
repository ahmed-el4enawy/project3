#ifndef ADC_PRIVATE_H
#define ADC_PRIVATE_H

#include "Std_Types.h"

#define ADC1_BASE_ADDR 0x40012000UL

typedef struct {
    volatile uint32 SR;
    volatile uint32 CR1;
    volatile uint32 CR2;
    volatile uint32 SMPR1;
    volatile uint32 SMPR2;
    volatile uint32 JOFR1;
    volatile uint32 JOFR2;
    volatile uint32 JOFR3;
    volatile uint32 JOFR4;
    volatile uint32 HTR;
    volatile uint32 LTR;
    volatile uint32 SQR1;
    volatile uint32 SQR2;
    volatile uint32 SQR3;
    volatile uint32 JSQR;
    volatile uint32 JDR1;
    volatile uint32 JDR2;
    volatile uint32 JDR3;
    volatile uint32 JDR4;
    volatile uint32 DR;
} AdcType;

#define ADC1 ((AdcType *)ADC1_BASE_ADDR)

#endif /* ADC_PRIVATE_H */