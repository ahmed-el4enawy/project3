#ifndef GPIO_PRIVATE_H
#define GPIO_PRIVATE_H

#include "Std_Types.h"

#define GPIOA_BASE 0x40020000UL
#define GPIOB_BASE 0x40020400UL

typedef struct {
    volatile uint32 MODER;
    volatile uint32 OTYPER;
    volatile uint32 OSPEEDR;
    volatile uint32 PUPDR;
    volatile uint32 IDR;
    volatile uint32 ODR;
    volatile uint32 BSRR;
    volatile uint32 LCKR;
    volatile uint32 AFRL;
    volatile uint32 AFRH;
} GpioType;

#endif /* GPIO_PRIVATE_H */