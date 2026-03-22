#===========================================================================================================
# @file        Utilities/CMakeLists.txt
# @author      Mohamed GALY
# @brief       CMake configuration for the Utilities module.
# @date        15-06-2026
# @version     1.0.0
#===========================================================================================================

add_library(utilitiesLib OBJECT)

target_sources(utilitiesLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/DataStructure/CircularBuffer/circBuff.c
)

target_include_directories(utilitiesLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/genUtilis
    ${CMAKE_CURRENT_SOURCE_DIR}/DataStructure/CircularBuffer
)

target_link_libraries(utilitiesLib PUBLIC f4board)
