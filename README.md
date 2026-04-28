# Auto-Cooler Temperature Control System

A bare-metal closed-loop temperature control system implemented on the STM32F401xE microcontroller. The system monitors ambient temperature using an LM35 sensor, adjusts a DC fan speed via PWM, and provides real-time feedback on an LCD1602 display.

## Project Description

This system uses a Mealy State Machine to govern its behavior. It continuously samples temperature and transitions between IDLE, COOLING, and OVERHEAT states. The implementation avoids polling loops for timing-sensitive operations by using DMA for sensor data acquisition and hardware timers for PWM generation.

## Key Features

- ADC with DMA: Uses ADC1 and DMA2 Stream 0 for continuous, non-blocking sampling of the LM35 sensor.
- PWM Control: Uses TIM3 Channel 1 to generate a 1 kHz PWM signal for DC fan speed control.
- LCD1602 Driver: Custom 4-bit parallel driver for character display without external libraries.
- Mealy State Machine: Explicit state-based logic where outputs depend on both the current state and the temperature input.
- Safety Alarm: Visual overheat indicator using a dedicated LED.

## Hardware Pinout

| Peripheral | Pin | Description |
| :--- | :--- | :--- |
| LM35 Sensor | PA0 | Analog Input (ADC1 Channel 0) |
| Alarm LED | PA5 | Digital Output (Active High) |
| DC Fan | PA6 | PWM Output (TIM3 Channel 1) |
| LCD RS | PB0 | Register Select |
| LCD EN | PB1 | Enable Signal |
| LCD D4-D7 | PB2-PB5 | Data Bits (4-bit mode) |

## Temperature Logic

- T < 25.0 C: Fan OFF (IDLE)
- 25.0 <= T < 30.0 C: Fan 33% (COOLING)
- 30.0 <= T < 35.0 C: Fan 66% (COOLING)
- T >= 35.0 C: Fan 100% (COOLING)
- T >= 40.0 C: OVERHEAT Warning and Alarm LED ON (OVERHEAT)

## Build Instructions

### Prerequisites

- ARM GNU Toolchain (arm-none-eabi-gcc)
- CMake (3.30 or higher)
- Ninja or Make build system

### Configuration

Ensure the ARM toolchain path is correctly set in cmake/ArmToolchain.cmake:
```cmake
set(ARM_DIR "path/to/your/arm-none-eabi/toolchain")
```

### Compiling

1. Create and configure the build directory:
   ```powershell
   cmake -S . -B build -G Ninja
   ```

2. Build the project:
   ```powershell
   cmake --build build
   ```

The build process generates stm32-template.hex, .bin, and .elf files in the build/ directory.

## Project Structure

- include/: Header files for drivers (ADC, PWM, LCD, GPIO) and FSM logic.
- src/: Source code implementation for all drivers and the main application loop.
- cmake/: Build configuration and device-specific settings.
- STM32-base/: CMSIS and startup files for the STM32F4 series.
