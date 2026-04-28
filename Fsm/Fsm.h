#ifndef FSM_H
#define FSM_H

#include "Std_Types.h"

typedef enum {
    STATE_IDLE,
    STATE_COOLING,
    STATE_OVERHEAT
} FSM_State_t;

void FSM_Update(uint16 temp_x10);

#endif /* FSM_H */