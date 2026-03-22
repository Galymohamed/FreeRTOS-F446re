#===========================================================================================================
# @file        Middlewares/CMakeLists.txt
# @author      Mohamed GALY
# @brief       CMake configuration for local FreeRTOS Kernel (Cortex-M4F, GCC port, heap_4).
# @details     Defines three targets:
#                 freertos_config      – INTERFACE target exposing FreeRTOSConfig.h + MCU ABI flags
#                 freertos_kernel      – core kernel sources + heap_4
#                 freertos_kernel_port – Cortex-M4F GCC port (port.c)
#              The root CMakeLists.txt links freertos_kernel and freertos_kernel_port
#              to the final executable.
#
# @target      STM32F446RE  (Cortex-M4F, GCC_ARM_CM4F port)
# @date        15-06-2026
# @version     1.0.0
#===========================================================================================================

set(FREERTOS_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/Third_Party/FreeRTOS/Source")
set(FREERTOS_PORT_DIR "${FREERTOS_SRC_DIR}/portable/GCC/ARM_CM4F")
set(FREERTOS_HEAP_DIR "${FREERTOS_SRC_DIR}/portable/MemMang")

#################################################################################################
# freertos_config — exposes FreeRTOSConfig.h location and MCU ABI flags to the kernel.
#################################################################################################
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE
    "${CMAKE_SOURCE_DIR}/Inc"
)
target_compile_options(freertos_config INTERFACE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)
#################################################################################################


#################################################################################################
# freertos_kernel — core kernel sources + heap implementation
#################################################################################################
add_library(freertos_kernel STATIC
    "${FREERTOS_SRC_DIR}/croutine.c"
    "${FREERTOS_SRC_DIR}/event_groups.c"
    "${FREERTOS_SRC_DIR}/list.c"
    "${FREERTOS_SRC_DIR}/queue.c"
    "${FREERTOS_SRC_DIR}/stream_buffer.c"
    "${FREERTOS_SRC_DIR}/tasks.c"
    "${FREERTOS_SRC_DIR}/timers.c"
    "${FREERTOS_HEAP_DIR}/heap_4.c"
)

target_include_directories(freertos_kernel PUBLIC
    "${FREERTOS_SRC_DIR}/include"
    "${FREERTOS_PORT_DIR}"
)

target_link_libraries(freertos_kernel PUBLIC
    freertos_config
    f4board
)
#################################################################################################


#################################################################################################
# freertos_kernel_port — Cortex-M4F GCC port layer
#################################################################################################
add_library(freertos_kernel_port STATIC
    "${FREERTOS_PORT_DIR}/port.c"
)

target_link_libraries(freertos_kernel_port PUBLIC freertos_kernel)
#################################################################################################
