#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h>

/* Fixed-width unsigned types */
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;

/* Fixed-width signed types */
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;

/* Boolean type */
typedef u8        boolean;
#define FALSE     ((boolean)0U)
#define TRUE      ((boolean)1U)

/* Standard function return type */
typedef enum {
    E_OK,
    E_NOT_OK
} Std_ReturnType;

/* Generic pointer for raw memory access */
typedef void* ptr_t;

#endif /* STD_TYPES_H */