#include "SysTick.h"
#include "SysTick_Private.h"

void SysTick_Init(void) {
    /* Select Processor Clock (AHB) which is 16 MHz in our case */
    SYSTICK->CTRL = (1UL << 2);
}

void SysTick_Delay_us(uint32 us) {
    /* 16 MHz clock means 16 ticks = 1 us */
    SYSTICK->LOAD = (16U * us) - 1U;
    SYSTICK->VAL  = 0; /* Clear current value to force reload */
    
    SYSTICK->CTRL |= (1UL << 0); /* Enable Timer */
    
    /* Wait until the COUNTFLAG (bit 16) is set */
    while ((SYSTICK->CTRL & (1UL << 16)) == 0); 
    
    SYSTICK->CTRL &= ~(1UL << 0); /* Disable Timer */
}

void SysTick_Delay_ms(uint32 ms) {
    for (uint32 i = 0; i < ms; i++) {
        SysTick_Delay_us(1000);
    }
}