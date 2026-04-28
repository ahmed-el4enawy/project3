#ifndef SYSTICK_H
#define SYSTICK_H

#include "Std_Types.h"

void SysTick_Init(void);
void SysTick_Delay_ms(uint32 ms);
void SysTick_Delay_us(uint32 us);

#endif /* SYSTICK_H */