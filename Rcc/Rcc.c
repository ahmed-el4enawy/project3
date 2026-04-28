#include "Rcc.h"
#include "Rcc_Private.h"
#include "Bit_Math.h"

void Rcc_EnableGPIOA(void) { SET_BIT(RCC->AHB1ENR, 0); }
void Rcc_EnableGPIOB(void) { SET_BIT(RCC->AHB1ENR, 1); }
void Rcc_EnableDMA2(void)  { SET_BIT(RCC->AHB1ENR, 22); }
void Rcc_EnableTIM3(void)  { SET_BIT(RCC->APB1ENR, 1); }
void Rcc_EnableADC1(void)  { SET_BIT(RCC->APB2ENR, 8); }