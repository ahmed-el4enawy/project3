#ifndef FSM_H
#define FSM_H

#include "STD_TYPES.h"

typedef enum {
    STATE_IDLE,
    STATE_COOLING,
    STATE_OVERHEAT
} FSM_State_t;

/* Call with temperature in 0.1 °C units (e.g., 253 = 25.3 °C) */
void FSM_Update(u16 temp_x10);

#endif /* FSM_H */
