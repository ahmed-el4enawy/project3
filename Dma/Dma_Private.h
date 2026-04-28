#ifndef DMA_PRIVATE_H
#define DMA_PRIVATE_H

#include "Std_Types.h"

#define DMA2_STREAM0_BASE_ADDR 0x40026410UL

typedef struct {
    volatile uint32 CR;
    volatile uint32 NDTR;
    volatile uint32 PAR;
    volatile uint32 M0AR;
    volatile uint32 M1AR;
    volatile uint32 FCR;
} DmaStreamType;

#define DMA2_STREAM0 ((DmaStreamType *)DMA2_STREAM0_BASE_ADDR)

#endif /* DMA_PRIVATE_H */