#ifndef GPIO_H
#define GPIO_H

#include "STD_TYPES.h"

/* ==================================================================== */
/* GPIO Base Addresses (STM32F401xE)                                    */
/* ==================================================================== */
#define GPIOA_BASE        (0x40020000U)
#define GPIOB_BASE        (0x40020400U)
#define GPIOC_BASE        (0x40020800U)

/* GPIO Register Offsets */
#define GPIO_MODER_OFFSET (0x00U)
#define GPIO_OTYPER_OFFSET (0x04U)
#define GPIO_OSPEEDR_OFFSET (0x08U)
#define GPIO_PUPDR_OFFSET (0x0CU)
#define GPIO_IDR_OFFSET   (0x10U)
#define GPIO_ODR_OFFSET   (0x14U)
#define GPIO_BSRR_OFFSET  (0x18U)
#define GPIO_LCKR_OFFSET  (0x1CU)
#define GPIO_AFRL_OFFSET  (0x20U)
#define GPIO_AFRH_OFFSET  (0x24U)

/* Macro to access GPIO registers */
#define GPIO_REG(PORT, OFFSET) (*((volatile u32 *)((PORT) + (OFFSET))))

/**
 * GPIO mode values (MODER register, 2 bits per pin)
 * 00: Input, 01: Output, 10: Alternate Function, 11: Analog
 */
#define GPIO_MODE_INPUT   0U
#define GPIO_MODE_OUTPUT  1U
#define GPIO_MODE_AF      2U
#define GPIO_MODE_ANALOG  3U

/* Initializes a pin's mode in the MODER register using bit masking */
void GPIO_SetMode(u32 port_base, u8 pin, u8 mode);

/* Configures the Alternate Function (AF) for a specific pin (4 bits per pin in AFR) */
void GPIO_SetAF(u32 port_base, u8 pin, u8 af);

/* Writes a digital high/low value to a pin using the BSRR register (atomic) */
void GPIO_WritePin(u32 port_base, u8 pin, u8 val);

#endif /* GPIO_H */
