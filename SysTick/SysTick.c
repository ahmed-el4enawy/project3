#include "SysTick.h"
#include "SysTick_Private.h"

void SysTick_Init(void) {
    SYSTICK->CTRL = 0;
}

void SysTick_Delay_us(uint32 us) {
    SYSTICK->LOAD = (16U * us) - 1U;
    SYSTICK->VAL = 0;
    SYSTICK->CTRL = 5;
    while ((SYSTICK->CTRL & 0x10000) == 0) {}
    SYSTICK->CTRL = 0;
}

void SysTick_Delay_ms(uint32 ms) {
    for (uint32 i = 0; i < ms; i++) {
        SysTick_Delay_us(1000);
    }
}