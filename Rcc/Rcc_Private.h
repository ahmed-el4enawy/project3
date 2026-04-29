#ifndef RCC_PRIVATE_H
#define RCC_PRIVATE_H

#include "Std_Types.h"

#define RCC_BASE_ADDR 0x40023800UL

typedef struct {
    volatile uint32 CR;
    volatile uint32 PLLCFGR;
    volatile uint32 CFGR;
    volatile uint32 CIR;
    volatile uint32 AHB1RSTR;
    volatile uint32 AHB2RSTR;
    uint32 RESERVED_0[2];
    volatile uint32 APB1RSTR;
    volatile uint32 APB2RSTR;
    uint32 RESERVED_1[2];
    volatile uint32 AHB1ENR;
    volatile uint32 AHB2ENR;
    uint32 RESERVED_2[2];
    volatile uint32 APB1ENR;
    volatile uint32 APB2ENR;
} RccType;

#define RCC ((RccType *)RCC_BASE_ADDR)

#endif /* RCC_PRIVATE_H */