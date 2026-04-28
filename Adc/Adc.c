#include "Adc.h"
#include "Adc_Private.h"
#include "Rcc.h"
#include "Gpio.h"
#include "Dma.h"

/* The buffer updated automatically by DMA */
static volatile uint16 Adc_DmaBuffer;

void Adc_Init(void)
{
    Rcc_EnableGPIOA();
    Rcc_EnableADC1();

    Gpio_SetMode(GPIOA_BASE, 0, GPIO_MODE_ANALOG);

    /* Pass the ADC Data Register address and our local buffer address to the DMA */
    Dma_InitAdcStream((uint32)&(ADC1->DR), (uint32)&Adc_DmaBuffer);

    ADC1->CR1 = 0; 
    
    /* DMA Enable (Bit 8), DDS Enable (Bit 9), Continuous Mode (Bit 1) */
    ADC1->CR2 = (1UL << 8) | (1UL << 9) | (1UL << 1); 
    
    ADC1->SQR1 = 0; /* 1 conversion in sequence */
    ADC1->SQR3 = 0; /* Channel 0 */
    ADC1->SMPR2 = (7UL << 0); /* 480 cycles sampling time */

    ADC1->CR2 |= (1UL << 0); /* Power ON */
    
    for (volatile uint32 i = 0; i < 1000; i++) {} /* Stabilization Delay */

    ADC1->CR2 |= (1UL << 30); /* SWSTART */
}

uint16 Adc_Read(void)
{
    return Adc_DmaBuffer;
}