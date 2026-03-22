#===========================================================================================================
# @file        Bsp/CMakeLists.txt
# @author      Mohamed GALY
# @brief       CMake configuration for the Bsp module.
# @date        2026
# @version     1.0.0
#===========================================================================================================

add_library(bspLib OBJECT)

target_sources(bspLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/HardwareUtils/brd_HWU.c
    ${CMAKE_CURRENT_SOURCE_DIR}/BspUart/bsp_uart2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/BspItm/bsp_itm.c
)

target_include_directories(bspLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/HardwareUtils
    ${CMAKE_CURRENT_SOURCE_DIR}/BspUart
    ${CMAKE_CURRENT_SOURCE_DIR}/BspItm
)

target_link_libraries(bspLib PUBLIC f4gCoreLib)
