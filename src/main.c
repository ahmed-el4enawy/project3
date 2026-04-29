/**
 * @file    main.c
 * @brief   System Entry Point for the Auto-Cooler Project.
 * @author  Team 27 (Ahmed Salah Geoshy Elshenawy & Ahmed Ahmed Mokhtar)
 * @details
 * This file coordinates the initialization of the STM32F401 hardware peripherals
 * and executes the primary asynchronous superloop. The system is designed to be
 * non-blocking, relying on ADC interrupts to drive the Mealy State Machine.
 *
 * Hardware Connectivity:
 * - Analog Input (LM35): PA1 (ADC1_IN1)
 * - Fan Control (PWM):   PA6 (Timer 3, Channel 1)
 * - User Interface:      LCD1602 (PB0, PB1, PB4-PB7)
 * - Critical Alert:      PD14 (Red LED)
 */

#include "Rcc.h"
#include "Gpio.h"
#include "Adc.h"
#include "Lcd.h"
#include "App.h"

/**
 * @brief  Main program body.
 * @return int (Standard C requirement, execution remains in superloop)
 */
int main(void)
{
    /* --- Phase 1: System Clock Configuration --- */
    // Initialize the Reset and Clock Control (RCC) using the internal HSI
    Rcc_Init();

    /* --- Phase 2: Peripheral Bus Clock Enabling --- */
    // Enable clocks for all utilized GPIO ports and internal peripherals
    Rcc_Enable(RCC_GPIOA);   // For LM35 Sensor and Fan PWM output
    Rcc_Enable(RCC_GPIOB);   // For LCD Control and Data lines
    Rcc_Enable(RCC_GPIOC);   // Reserved for expansion
    Rcc_Enable(RCC_GPIOD);   // For the Overheat Alarm LED
    Rcc_Enable(RCC_TIM3);    /* Enable General Purpose Timer 3 for PWM generation */
    Rcc_Enable(RCC_ADC1);    /* Enable ADC1 for analog temperature sampling */

    /* --- Phase 3: Driver & Hardware Initialization --- */

    // Configure PA1 for Analog Input mode (LM35 signal)
    // No internal pull-ups/downs to ensure raw sensor voltage accuracy
    Gpio_Init(GPIO_A, 1U, GPIO_ANALOG, GPIO_NO_PULL_DOWN);

    // Initialize ADC1 with 12-bit precision (4096 steps)
    // Asynchronous behavior and callbacks are managed by the App layer
    Adc_Init(ADC_RES_12BIT);

    // Bootstrap the LCD1602 in 4-bit parallel mode
    Lcd_Init();

    // Initialize logic layer: sets fan to 0%, LED off, and triggers first ADC conversion
    App_Init();

    /* --- Phase 4: Non-Blocking Superloop --- */
    /**
     * The system architecture follows a run-to-completion model.
     * App_Run() checks if a new temperature sample is ready via the ISR flag.
     * This ensures the CPU is never trapped in a polling loop, satisfying the
     * strict real-time requirements of Medical Equipment standards.
     */
    while (1)
    {
        // Execute the State Machine logic
        App_Run();
    }

    /* Program execution should never reach this point */
    return 0;
}