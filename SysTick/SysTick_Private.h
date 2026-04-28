#ifndef SYSTICK_PRIVATE_H
#define SYSTICK_PRIVATE_H

#include "Std_Types.h"

#define SYSTICK_BASE_ADDR 0xE000E010UL

typedef struct {
    volatile uint32 CTRL;
    volatile uint32 LOAD;
    volatile uint32 VAL;
    volatile uint32 CALIB;
} SysTickType;

#define SYSTICK ((SysTickType *)SYSTICK_BASE_ADDR)

#endif /* SYSTICK_PRIVATE_H */