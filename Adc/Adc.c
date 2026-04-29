#include "Adc.h"
#include "Adc_Private.h"
#include "Rcc.h"
#include "Gpio.h"
#include "Dma.h"

static volatile uint16 Adc_DmaBuffer;

void Adc_Init(void)
{
    Rcc_EnableGPIOA();
    Rcc_EnableADC1();

    Gpio_SetMode(GPIOA_BASE, 0, GPIO_MODE_ANALOG);

    Dma_InitAdcStream((uint32)&(ADC1->DR), (uint32)&Adc_DmaBuffer);

    ADC1->CR1 = 0;
    ADC1->CR2 = (1UL << 8) | (1UL << 9) | (1UL << 1);
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 0;
    ADC1->SMPR2 = (7UL << 0);

    ADC1->CR2 |= (1UL << 0);

    for (volatile uint32 i = 0; i < 1000; i++) {}

    ADC1->CR2 |= (1UL << 30);
}

uint16 Adc_Read(void)
{
    return Adc_DmaBuffer;
}