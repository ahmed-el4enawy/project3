#include "Gpio.h"
#include "Gpio_Private.h"

void Gpio_SetMode(uint32 port_base, uint8 pin, uint8 mode) {
    GpioType *port = (GpioType *)port_base;
    port->MODER &= ~((uint32)(3UL << (pin * 2)));
    port->MODER |= ((uint32)mode << (pin * 2));
}

void Gpio_SetAF(uint32 port_base, uint8 pin, uint8 af) {
    GpioType *port = (GpioType *)port_base;
    if (pin < 8) {
        port->AFRL &= ~((uint32)(0x0FUL << (pin * 4)));
        port->AFRL |= ((uint32)af << (pin * 4));
    } else {
        port->AFRH &= ~((uint32)(0x0FUL << ((pin - 8) * 4)));
        port->AFRH |= ((uint32)af << ((pin - 8) * 4));
    }
}

void Gpio_WritePin(uint32 port_base, uint8 pin, uint8 val) {
    GpioType *port = (GpioType *)port_base;
    if (val) {
        port->BSRR = (1UL << pin);
    } else {
        port->BSRR = (1UL << (pin + 16));
    }
}