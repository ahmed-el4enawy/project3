set(INCLUDE_LIST ${INCLUDE_LIST}
        ${ARM_DIR}/arm-none-eabi/include
        ${PROJECT_PATH}/STM32-base/startup
        ${PROJECT_PATH}/STM32-base-STM32Cube/CMSIS/ARM/inc
        ${PROJECT_PATH}/STM32-base-STM32Cube/CMSIS/${SERIES_FOLDER}/inc
        ${PROJECT_PATH}/include
        ${PROJECT_PATH}/Lib
        ${PROJECT_PATH}/RCC
        ${PROJECT_PATH}/Gpio
        ${PROJECT_PATH}/SevenSeg
        ${PROJECT_PATH}/Exti
        ${PROJECT_PATH}/Timer
        ${PROJECT_PATH}/Pwm
        ${PROJECT_PATH}/Nvic
        ${PROJECT_PATH}/Adc
        ${PROJECT_PATH}/Lcd
        ${PROJECT_PATH}/App
)

if (USE_HAL)
    set(INCLUDE_LIST ${INCLUDE_LIST} ${PROJECT_PATH}/STM32-base-STM32Cube/HAL/${SERIES_FOLDER}/inc)
endif ()
