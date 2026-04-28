#include "gpio.h"

void GPIO_SetMode(u32 port_base, u8 pin, u8 mode)
{
    /* Each pin occupies 2 bits in the MODER register. 
     * We clear the existing 2 bits first, then write the new mode. */
    GPIO_REG(port_base, GPIO_MODER_OFFSET) &= ~(3U << (pin * 2U));
    GPIO_REG(port_base, GPIO_MODER_OFFSET) |=  ((u32)mode << (pin * 2U));
}

void GPIO_SetAF(u32 port_base, u8 pin, u8 af)
{
    /* AFRL (offset 0x20) handles pins 0-7, AFRH (offset 0x24) handles pins 8-15.
     * Each pin has 4 bits for AF selection. */
    u32 offset = (pin < 8U) ? GPIO_AFRL_OFFSET : GPIO_AFRH_OFFSET;
    u8 pos = (pin % 8U) * 4U;
    
    GPIO_REG(port_base, offset) &= ~(0xFU << pos);
    GPIO_REG(port_base, offset) |=  ((u32)af << pos);
}

void GPIO_WritePin(u32 port_base, u8 pin, u8 val)
{
    /* Using BSRR (Bit Set Reset Register) is the standard bare-metal way 
     * to set/clear pins atomically without read-modify-write on ODR. */
    if (val) {
        GPIO_REG(port_base, GPIO_BSRR_OFFSET) = (1U << pin);        /* Set the pin high */
    } else {
        GPIO_REG(port_base, GPIO_BSRR_OFFSET) = (1U << (pin + 16U)); /* Reset the pin low (offset 16) */
    }
}
