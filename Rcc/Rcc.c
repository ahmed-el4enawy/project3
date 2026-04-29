#include "Rcc.h"
#include "Rcc_Private.h"

void Rcc_EnableGPIOA(void) {
    RCC->AHB1ENR |= (1UL << 0);
}

void Rcc_EnableGPIOB(void) {
    RCC->AHB1ENR |= (1UL << 1);
}

void Rcc_EnableTIM3(void) {
    RCC->APB1ENR |= (1UL << 1);
}

void Rcc_EnableADC1(void) {
    RCC->APB2ENR |= (1UL << 8); /* Enable ADC1 Clock (This was missing!) */
}

void Rcc_EnableDMA2(void) {
    RCC->AHB1ENR |= (1UL << 22); /* Enable DMA2 Clock (This was missing!) */
}