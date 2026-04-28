#ifndef TIMER_PRIVATE_H
#define TIMER_PRIVATE_H

#include "Std_Types.h"

#define TIM3_BASE_ADDR 0x40000400UL

typedef struct {
    volatile uint32 CR1;
    volatile uint32 CR2;
    volatile uint32 SMCR;
    volatile uint32 DIER;
    volatile uint32 SR;
    volatile uint32 EGR;
    volatile uint32 CCMR1;
    volatile uint32 CCMR2;
    volatile uint32 CCER;
    volatile uint32 CNT;
    volatile uint32 PSC;
    volatile uint32 ARR;
    uint32 RESERVED;
    volatile uint32 CCR1;
    volatile uint32 CCR2;
    volatile uint32 CCR3;
    volatile uint32 CCR4;
} TimType;

#define TIM3 ((TimType *)TIM3_BASE_ADDR)

#endif /* TIMER_PRIVATE_H */