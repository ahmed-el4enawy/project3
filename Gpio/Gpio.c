#include "Gpio.h"
#include "Gpio_Private.h"

void Gpio_SetMode(uint32 port_base, uint8 pin, uint8 mode) {
    GpioType *Port = (GpioType *)port_base;
    Port->MODER &= ~(3UL << (pin * 2U));
    Port->MODER |=  ((uint32)mode << (pin * 2U));
}

void Gpio_SetAF(uint32 port_base, uint8 pin, uint8 af) {
    GpioType *Port = (GpioType *)port_base;
    if (pin < 8U) {
        Port->AFRL &= ~(0xFUL << (pin * 4U));
        Port->AFRL |=  ((uint32)af << (pin * 4U));
    } else {
        uint8 pos = (pin - 8U) * 4U;
        Port->AFRH &= ~(0xFUL << pos);
        Port->AFRH |=  ((uint32)af << pos);
    }
}

void Gpio_WritePin(uint32 port_base, uint8 pin, uint8 val) {
    GpioType *Port = (GpioType *)port_base;
    /* Atomic Pin Write using BSRR */
    if (val) {
        Port->BSRR = (1UL << pin);
    } else {
        Port->BSRR = (1UL << (pin + 16U));
    }
}