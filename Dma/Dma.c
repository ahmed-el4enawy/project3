#include "Dma.h"
#include "Dma_Private.h"
#include "Rcc.h"

void Dma_InitAdcStream(uint32 peripheral_addr, uint32 memory_addr)
{
    Rcc_EnableDMA2();

    DMA2_STREAM0->CR = 0;
    while ((DMA2_STREAM0->CR & 1UL) != 0) { }

    DMA2_STREAM0->PAR = peripheral_addr;
    DMA2_STREAM0->M0AR = memory_addr;
    DMA2_STREAM0->NDTR = 1;

    /* CHSEL=0, MSIZE=16-bit, PSIZE=16-bit, MINC=0, PINC=0, CIRC=1 */
    DMA2_STREAM0->CR = (0UL << 25) | (1UL << 13) | (1UL << 11) | (1UL << 8);

    DMA2_STREAM0->CR |= (1UL << 0);
}