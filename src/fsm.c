#include "fsm.h"
#include "pwm.h"
#include "lcd.h"
#include "gpio.h"

/* Alarm LED on PA5 */
#define ALARM_PORT_BASE  GPIOA_BASE
#define ALARM_PIN        5

/* Temperature thresholds in 0.1 °C units */
#define THRESH_LOW   250U   /* 25.0 °C */
#define THRESH_MID   300U   /* 30.0 °C */
#define THRESH_HIGH  350U   /* 35.0 °C */
#define THRESH_OVER  400U   /* 40.0 °C */

static FSM_State_t state = STATE_IDLE;

/* Return the fan duty cycle for normal (non-overheat) operation based on project requirements. */
static u8 GetDuty(u16 temp_x10)
{
    if (temp_x10 < THRESH_LOW)  return 0;      /* T < 25: Fan OFF */
    if (temp_x10 < THRESH_MID)  return 33;     /* 25 <= T < 30: 33% */
    if (temp_x10 < THRESH_HIGH) return 66;     /* 30 <= T < 35: 66% */
    return 100;                                /* T >= 35: 100% */
}

/* Format and display temperature + fan info on the LCD. 
 * Integer math is used to show one decimal place without using float. */
static void UpdateLCD(u16 temp_x10, u8 duty, u8 overheat)
{
    char buf[17]; 

    /* Line 1: "Temp: XX.X C" */
    u16 whole = temp_x10 / 10;
    u16 frac  = temp_x10 % 10;
    buf[0]  = 'T'; buf[1] = 'e'; buf[2] = 'm'; buf[3] = 'p';
    buf[4]  = ':'; buf[5] = ' ';
    buf[6]  = (whole / 10) ? ('0' + (whole / 10)) : ' ';
    buf[7]  = '0' + (whole % 10);
    buf[8]  = '.';
    buf[9]  = '0' + frac;
    buf[10] = ' ';
    buf[11] = (char)0xDF; /* Degree symbol */
    buf[12] = 'C';
    buf[13] = ' '; buf[14] = ' '; buf[15] = ' ';
    buf[16] = '\0';

    LCD_SetCursor(0, 0);
    LCD_Print(buf);

    /* Line 2 logic: Show "OVERHEAT!" if T >= 40, otherwise show Fan Speed. */
    LCD_SetCursor(1, 0);
    if (overheat) {
        LCD_Print("  OVERHEAT!     ");
    } else {
        buf[0]  = 'F'; buf[1] = 'a'; buf[2] = 'n'; buf[3] = ':';
        buf[4]  = ' '; buf[5] = ' ';
        if (duty == 100) {
            buf[6] = '1'; buf[7] = '0'; buf[8] = '0';
        } else {
            buf[6] = (duty / 10) ? ('0' + (duty / 10)) : ' ';
            buf[7] = '0' + (duty % 10);
            buf[8] = ' ';
        }
        buf[9]  = '%';
        buf[10] = ' '; buf[11] = ' '; buf[12] = ' ';
        buf[13] = ' '; buf[14] = ' '; buf[15] = ' ';
        buf[16] = '\0';
        LCD_Print(buf);
    }
}

/**
 * Mealy State Machine Update function.
 * All outputs (PWM, LED, LCD) are decided by both current state AND the input (temp).
 */
void FSM_Update(u16 temp_x10)
{
    u8 duty;

    switch (state) {

    case STATE_IDLE:
        /* Transition from IDLE to COOLING if temp hits 25C. */
        if (temp_x10 >= THRESH_LOW) {
            duty = GetDuty(temp_x10);
            PWM_SetDuty(duty);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 0);
            UpdateLCD(temp_x10, duty, 0);
            state = STATE_COOLING;
        } else {
            /* Outputs for IDLE state (Fan OFF, Alarm OFF) */
            PWM_SetDuty(0);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 0);
            UpdateLCD(temp_x10, 0, 0);
        }
        break;

    case STATE_COOLING:
        /* Transition from COOLING to OVERHEAT if temp hits 40C. */
        if (temp_x10 >= THRESH_OVER) {
            PWM_SetDuty(100);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 1);
            UpdateLCD(temp_x10, 100, 1);
            state = STATE_OVERHEAT;
        } 
        /* Transition back to IDLE if temp drops below 25C. */
        else if (temp_x10 < THRESH_LOW) {
            PWM_SetDuty(0);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 0);
            UpdateLCD(temp_x10, 0, 0);
            state = STATE_IDLE;
        } 
        /* Remain in COOLING, adjusting duty cycle based on input. */
        else {
            duty = GetDuty(temp_x10);
            PWM_SetDuty(duty);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 0);
            UpdateLCD(temp_x10, duty, 0);
        }
        break;

    case STATE_OVERHEAT:
        /* Transition back to COOLING if temp drops below 40C. */
        if (temp_x10 < THRESH_OVER) {
            duty = GetDuty(temp_x10);
            PWM_SetDuty(duty);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 0);
            UpdateLCD(temp_x10, duty, 0);
            state = STATE_COOLING;
        } 
        /* Remain in OVERHEAT: Fan max, Alarm ON. */
        else {
            PWM_SetDuty(100);
            GPIO_WritePin(ALARM_PORT_BASE, ALARM_PIN, 1);
            UpdateLCD(temp_x10, 100, 1);
        }
        break;
    }
}
