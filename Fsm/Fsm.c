#include "Fsm.h"
#include "Timer.h"
#include "Lcd.h"
#include "Led.h"

/* Temperature thresholds in 0.1 C units */
#define THRESH_LOW   250U   /* 25.0 C */
#define THRESH_MID   300U   /* 30.0 C */
#define THRESH_HIGH  350U   /* 35.0 C */
#define THRESH_OVER  400U   /* 40.0 C */

static FSM_State_t current_state = STATE_IDLE;

static uint8 GetDutyCycle(uint16 temp) {
    if (temp < THRESH_LOW)  return 0;
    if (temp < THRESH_MID)  return 33;
    if (temp < THRESH_HIGH) return 66;
    return 100;
}

/* دالة لكتابة القيم على الشاشة بدون استخدام sprintf الثقيلة */
static void UpdateDisplay(uint16 temp, uint8 duty, uint8 is_overheat) {
    char buffer[16];
    uint8 int_part = temp / 10U;
    uint8 frac_part = temp % 10U;

    Lcd_Clear();
    
    /* Line 1: Temperature */
    Lcd_SetCursor(0, 0);
    buffer[0] = 'T'; buffer[1] = 'e'; buffer[2] = 'm'; buffer[3] = 'p'; buffer[4] = ':'; buffer[5] = ' ';
    buffer[6] = (int_part / 10) + '0';
    buffer[7] = (int_part % 10) + '0';
    buffer[8] = '.';
    buffer[9] = frac_part + '0';
    buffer[10] = ' '; buffer[11] = 'C'; buffer[12] = '\0';
    Lcd_Print(buffer);

    /* Line 2: Fan / Overheat */
    Lcd_SetCursor(1, 0);
    if (is_overheat) {
        Lcd_Print("SYSTEM OVERHEAT!");
    } else {
        buffer[0] = 'F'; buffer[1] = 'a'; buffer[2] = 'n'; buffer[3] = ':'; buffer[4] = ' ';
        if (duty == 100) {
            buffer[5] = '1'; buffer[6] = '0'; buffer[7] = '0'; buffer[8] = '%'; buffer[9] = '\0';
        } else if (duty >= 10) {
            buffer[5] = (duty / 10) + '0'; buffer[6] = (duty % 10) + '0'; buffer[7] = '%'; buffer[8] = '\0';
        } else {
            buffer[5] = (duty % 10) + '0'; buffer[6] = '%'; buffer[7] = '\0';
        }
        Lcd_Print(buffer);
    }
}

void FSM_Update(uint16 temp_x10) {
    uint8 duty = 0;

    switch (current_state) {
        case STATE_IDLE:
            if (temp_x10 >= THRESH_LOW) {
                current_state = STATE_COOLING;
                duty = GetDutyCycle(temp_x10);
                Timer_SetPWMDuty(duty);
                Led_AlarmSetState(0);
                UpdateDisplay(temp_x10, duty, 0);
            } else {
                Timer_SetPWMDuty(0);
                Led_AlarmSetState(0);
                UpdateDisplay(temp_x10, 0, 0);
            }
            break;

        case STATE_COOLING:
            if (temp_x10 >= THRESH_OVER) {
                current_state = STATE_OVERHEAT;
                Timer_SetPWMDuty(100);
                Led_AlarmSetState(1);
                UpdateDisplay(temp_x10, 100, 1);
            } else if (temp_x10 < THRESH_LOW) {
                current_state = STATE_IDLE;
                Timer_SetPWMDuty(0);
                Led_AlarmSetState(0);
                UpdateDisplay(temp_x10, 0, 0);
            } else {
                duty = GetDutyCycle(temp_x10);
                Timer_SetPWMDuty(duty);
                Led_AlarmSetState(0);
                UpdateDisplay(temp_x10, duty, 0);
            }
            break;

        case STATE_OVERHEAT:
            if (temp_x10 < THRESH_OVER) {
                current_state = STATE_COOLING;
                duty = GetDutyCycle(temp_x10);
                Timer_SetPWMDuty(duty);
                Led_AlarmSetState(0);
                UpdateDisplay(temp_x10, duty, 0);
            } else {
                Timer_SetPWMDuty(100);
                Led_AlarmSetState(1);
                UpdateDisplay(temp_x10, 100, 1);
            }
            break;
    }
}