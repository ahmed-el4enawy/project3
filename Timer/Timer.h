#ifndef TIMER_H
#define TIMER_H

#include "Std_Types.h"

#define TIM3_BASE_ADDR 0x40000400UL

void Timer_InitPWM(void);
void Timer_SetPWMDuty(uint8 duty);

#endif /* TIMER_H */