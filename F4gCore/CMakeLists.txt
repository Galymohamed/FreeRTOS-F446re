#===========================================================================================================
# @file        F4gCore/CMakeLists.txt
# @author      Mohamed GALY
# @brief       CMake configuration for the F4gCore module.
# @date        2026
# @version     1.0.0
#===========================================================================================================

add_library(f4gCoreLib OBJECT)

target_sources(f4gCoreLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/GPIO/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Interrupt/gpio_intrpt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Interrupt/uart_intrpt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/coreIntrpt/fault_handlers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIMER/delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/UART/uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/UART/uartCB.c
)

target_include_directories(f4gCoreLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/GPIO
    ${CMAKE_CURRENT_SOURCE_DIR}/Interrupt
    ${CMAKE_CURRENT_SOURCE_DIR}/Interrupt/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/coreIntrpt/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/TIMER
    ${CMAKE_CURRENT_SOURCE_DIR}/UART/Inc
)

target_link_libraries(f4gCoreLib PUBLIC f4board utilitiesLib)
