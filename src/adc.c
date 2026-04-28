#include "adc.h"
#include "gpio.h"
#include "rcc.h"

/* DMA writes the latest ADC conversion here continuously */
static volatile u16 adc_dma_buf;

void ADC_Init(void)
{
    /* 1. Enable peripheral clocks using explicit RCC driver. */
    RCC_voidEnableGPIOA();
    RCC_voidEnableADC1();
    RCC_voidEnableDMA2();

    /* 2. Configure PA0 as Analog mode so the ADC can read the voltage. */
    GPIO_SetMode(GPIOA_BASE, 0, GPIO_MODE_ANALOG);

    /* 3. Configure DMA2 to move data from ADC to memory automatically. 
     * This avoids CPU polling and ensures we always have the latest reading. */
    DMA2_S0_REG(DMA_CR_OFFSET) = 0;                        /* Disable stream, reset */
    while (DMA2_S0_REG(DMA_CR_OFFSET) & DMA_CR_EN);        /* Wait until disabled  */

    DMA2_S0_REG(DMA_PAR_OFFSET)  = (u32)(ADC1_BASE + ADC_DR_OFFSET); /* Source: ADC Data Register */
    DMA2_S0_REG(DMA_M0AR_OFFSET) = (u32)&adc_dma_buf;                /* Destination: Local variable */
    DMA2_S0_REG(DMA_NDTR_OFFSET) = 1;                                 /* Number of items: 1 */
    
    DMA2_S0_REG(DMA_CR_OFFSET)   = DMA_CR_CHSEL_0          /* ADC1 is on DMA2 Channel 0 */
                                 | DMA_CR_MSIZE_16BIT      /* 16-bit memory size */
                                 | DMA_CR_PSIZE_16BIT      /* 16-bit peripheral size */
                                 | DMA_CR_CIRC             /* Circular mode (runs forever) */
                                 | DMA_CR_EN;              /* Start the DMA engine */

    /* 4. Configure ADC1 settings. */
    ADC1_REG(ADC_CR1_OFFSET)  = 0;                               /* Default: 12-bit resolution */
    ADC1_REG(ADC_CR2_OFFSET)  = ADC_CR2_DMA                      /* Let ADC trigger DMA */
                              | ADC_CR2_DDS                      /* DMA Request even in continuous mode */
                              | ADC_CR2_CONT;                    /* Continuous: doesn't stop after one conversion */
    ADC1_REG(ADC_SQR1_OFFSET) = 0;                               /* 1 conversion in sequence (L=0 means 1 conv) */
    ADC1_REG(ADC_SQR3_OFFSET) = 0;                               /* Read Channel 0 (PA0) */
    ADC1_REG(ADC_SMPR2_OFFSET) = (7U << 0);                      /* Max sampling time (480 cycles) for accuracy on Ch 0 */

    /* 5. Turn on the ADC and start the hardware trigger. */
    ADC1_REG(ADC_CR2_OFFSET) |= ADC_CR2_ADON;
    for (volatile u32 i = 0; i < 1000; i++);      /* Stabilization delay */
    ADC1_REG(ADC_CR2_OFFSET) |= ADC_CR2_SWSTART;                 /* Kick off the first conversion */
}

u16 ADC_Read(void)
{
    /* Return the value updated by DMA in the background. */
    return adc_dma_buf;
}
