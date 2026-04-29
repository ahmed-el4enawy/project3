#ifndef GPIO_H
#define GPIO_H

#include "Std_Types.h"

#define GPIOA_BASE 0x40020000UL
#define GPIOB_BASE 0x40020400UL

#define GPIO_MODE_INPUT     0
#define GPIO_MODE_OUTPUT    1
#define GPIO_MODE_AF        2
#define GPIO_MODE_ANALOG    3

void Gpio_SetMode(uint32 port_base, uint8 pin, uint8 mode);
void Gpio_SetAF(uint32 port_base, uint8 pin, uint8 af);
void Gpio_WritePin(uint32 port_base, uint8 pin, uint8 val);

#endif /* GPIO_H */