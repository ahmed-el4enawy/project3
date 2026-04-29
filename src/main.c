/**
 * @file    main.c
 * @brief   Auto-Cooler entry point — initialises all peripherals and runs the
 *          superloop that dispatches the Mealy state machine on each ADC sample.
 *
 *  Hardware summary:
 *    PA1  — LM35 sensor output      → ADC1 Channel 1 (analogue)
 *    PA6  — TIM3_CH1 PWM (AF2)      → L293D fan enable
 *    PB0  — LCD RS
 *    PB1  — LCD EN
 *    PB4–PB7 — LCD DB4–DB7
 *    PD12 — Overheat alarm LED (active HIGH)
 *
 *  Created on: 2026-04-27
 *  Author    : Team 29
 */

#include "Rcc.h"
#include "Gpio.h"
#include "Adc.h"
#include "Lcd.h"
#include "App.h"

/**
 * @brief  Application entry point.
 */
int main(void)
{
    /* ---- 1. Enable HSI system clock ---- */
    Rcc_Init();

    /* ---- 2. Enable peripheral clocks ---- */
    Rcc_Enable(RCC_GPIOA);   /* PA1 (ADC), PA6 (PWM)         */
    Rcc_Enable(RCC_GPIOB);   /* PB0-PB7 (LCD)                */
    Rcc_Enable(RCC_GPIOC);   /* Available for future use      */
    Rcc_Enable(RCC_GPIOD);   /* PD12 (Alarm LED)              */
    Rcc_Enable(RCC_TIM3);    /* TIM3 -- fan PWM               */
    Rcc_Enable(RCC_ADC1);    /* ADC1 -- temperature sensing   */
    /* DMA2 not needed: new ADC driver uses EOC interrupt, not DMA */

    /* ---- 3. Initialise drivers in dependency order ---- */

    /* PA1: analogue input for LM35 -- new Adc_Init does not touch GPIO */
    Gpio_Init(GPIO_A, 1U, GPIO_ANALOG, GPIO_NO_PULL_DOWN);

    /* Init ADC1 at 12-bit resolution                                    */
    /* Channel config + async callback are set inside App_Init()         */
    Adc_Init(ADC_RES_12BIT);

    Lcd_Init();   /* Initialise LCD1602 in 4-bit mode                   */
    App_Init();   /* Configures PWM, LED, LCD splash, arms ADC callback  */

    /* ---- 4. Superloop ---- */
    /* App_Run() is non-blocking: it checks App_AdcReady (set by the     */
    /* ADC ISR callback) and returns immediately when no new sample is   */
    /* ready. The ADC ISR fires after each conversion started in App_Init */
    /* and re-armed inside App_Run, satisfying Requirement 4 (no         */
    /* busy-loop delays in application code).                            */
    while (1)
    {
        App_Run();
    }

    /* Never reached */
    return 0;
}
