#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h> /* استخدام المكتبة القياسية لضمان الأمان التام */

/* Fixed-width unsigned types */
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t     uint32;

/* Fixed-width signed types */
typedef int8_t       sint8;
typedef int16_t      sint16;
typedef int32_t      sint32;

/* Boolean Definitions (In case needed by legacy files) */
#ifndef TRUE
#define TRUE  1U
#endif

#ifndef FALSE
#define FALSE 0U
#endif

#endif /* STD_TYPES_H */