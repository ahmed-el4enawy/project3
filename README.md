# Auto-Cooler Temperature Control System

A professional, bare-metal closed-loop temperature control system implemented on the STM32F401xE microcontroller. Developed for **Project 3: Medical Equipment II**, this system monitors ambient temperature using an LM35 sensor, dynamically adjusts DC fan speed via PWM, and provides real-time operational feedback on an LCD1602 display.

**Developed by Team 27:** Ahmed Salah Geoshy Elshenawy & Ahmed Ahmed Mokhtar

## System Architecture & Description

This system is governed by an event-driven **Mealy Finite State Machine (FSM)**. It continuously samples thermal data and seamlessly transitions between `IDLE`, `COOLING`, and `OVERHEAT` states.

To meet strict real-time medical equipment standards, the application completely eliminates polling and busy-wait loops. It relies on an **asynchronous, interrupt-driven ADC** for sensor acquisition, allowing the CPU to remain unblocked. FSM transitions are executed in O(1) time complexity using a highly efficient function-pointer dispatch table.

## Key Engineering Features

- **Asynchronous ADC:** Utilizes hardware interrupts via `ADC1` for continuous, non-blocking sampling of the LM35 sensor, dropping the heavy overhead of DMA while maintaining autonomous execution.
- **High-Frequency PWM Control:** Configures `TIM3 Channel 1` to generate a smooth **10 kHz PWM signal**, ensuring silent and efficient DC motor operation without acoustic noise.
- **Hardware Abstraction Layer (HAL):** All pinout mappings and thermal policies are heavily decoupled from logic and isolated in a central Master Configuration Panel (`Lib/App_Config.h`).
- **Conflict-Free LCD Driver:** Custom 4-bit parallel driver mapped to the "High Pins" of `PORTB` to completely avoid hardware conflicts with the STM32 `BOOT1` state and `SWD` debugger lines.
- **Mealy Dispatch Table:** Explicit state-based logic mapping where dynamic outputs (Fan Speed, LED, LCD text) instantly respond to combined state-event vectors.

## Hardware Pinout (Team 27 Configuration)

*Note: This specific layout routes the LCD data bus safely away from system debug pins.*

| Peripheral / Signal | STM32 Pin | Description / Notes |
| :--- | :--- | :--- |
| **LM35 Analog Temp**| **PA1** | `ADC1_IN1` (Floating Input) |
| **DC Fan Control** | **PA6** | `TIM3_CH1` (Alternate Function 2 - PWM) |
| **Overheat Alarm** | **PD14**| Digital Output (Active High via 330Ω Resistor)|
| **LCD RS** | **PB8** | Register Select |
| **LCD EN** | **PB9** | Enable Latch |
| **LCD D4 - D7** | **PB12-PB15**| 4-bit Data Bus (Push-Pull Output) |

## Thermal Control Logic

The Mealy FSM applies the following operational hysteresis and performance scaling:

- **T < 25.0 °C:** Fan 0% (`IDLE`)
- **25.0 ≤ T < 30.0 °C:** Fan 33% (`COOLING`)
- **30.0 ≤ T < 35.0 °C:** Fan 66% (`COOLING`)
- **35.0 ≤ T < 40.0 °C:** Fan 100% (`COOLING`)
- **T ≥ 40.0 °C:** **!! OVERHEAT !!** System Lock, Fan 100%, Alarm LED ON (`OVERHEAT`)
- *Recovery:* The system remains in `OVERHEAT` until the temperature explicitly drops below 40.0 °C.

## Build Instructions

### Prerequisites
- ARM GNU Toolchain (`arm-none-eabi-gcc`)
- CMake (3.30 or higher)
- Ninja Build System
- CLion IDE (Recommended)

### Compiling via CMake

1. Create and configure the build directory:
   ```bash
   cmake -S . -B build -G Ninja
   ```

2. Compile the executable:
   ```bash
   cmake --build build
   ```

The build process will generate the `.elf`, `.hex`, and `.bin` binaries inside the `build/` directory, ready to be flashed to the STM32F401xE or loaded into Proteus.

## Modular Source Structure

The repository is organized by strict Separation of Concerns (SoC):

- `App/` - Core FSM logic, FSM Dispatch Tables, and the main initialization sequence.
- `Lib/` - Master Hardware Configuration (`App_Config.h`), standard types, and utility delays.
- `Adc/` - Interrupt-driven analog-to-digital converter peripheral driver.
- `Pwm/` & `Timer/` - High-frequency signal generation for the actuator.
- `Lcd/` - Raw HD44780 4-bit communication instructions.
- `Rcc/` & `Gpio/` & `Nvic/` - Foundational low-level MCAL drivers.
- `src/main.c` - Orchestrator and non-blocking superloop.