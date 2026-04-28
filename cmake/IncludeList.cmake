set(INCLUDE_LIST ${INCLUDE_LIST}
        ${ARM_DIR}/arm-none-eabi/include
        ${PROJECT_PATH}/STM32-base/startup
        ${PROJECT_PATH}/STM32-base-STM32Cube/CMSIS/ARM/inc
        ${PROJECT_PATH}/STM32-base-STM32Cube/CMSIS/${SERIES_FOLDER}/inc

        # --- Component-Based Architecture Includes ---
        ${PROJECT_PATH}/Lib
        ${PROJECT_PATH}/Rcc
        ${PROJECT_PATH}/Gpio
        ${PROJECT_PATH}/SysTick
        ${PROJECT_PATH}/Timer
        ${PROJECT_PATH}/Dma
        ${PROJECT_PATH}/Adc
        ${PROJECT_PATH}/Lcd
        ${PROJECT_PATH}/Led
        ${PROJECT_PATH}/Fsm
)

if (USE_HAL)
    set(INCLUDE_LIST ${INCLUDE_LIST} ${PROJECT_PATH}/STM32-base-STM32Cube/HAL/${SERIES_FOLDER}/inc)
endif ()