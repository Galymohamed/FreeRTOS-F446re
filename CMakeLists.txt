#===========================================================================================================
# @file        ROOT/CMakeLists.txt
# @author      Mohamed GALY
# @brief       CMake configuration for the entire project.
# @date        2026
# @version     1.0.0
#===========================================================================================================
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(freeRtosF4Blink
    VERSION 1.0.0
    DESCRIPTION "STM32 CMake Project template"
    LANGUAGES C CXX ASM
)

set(CMAKE_C_STANDARD            11)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_C_EXTENSIONS          ON)     # Enable GNU C extensions

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        ON)     # Enable GNU C++ extensions
set(EXECUTABLE                  ${CMAKE_PROJECT_NAME})

# Generate compile_commands.json for clangd / IDE language-server support
set(CMAKE_EXPORT_COMPILE_COMMANDS                    TRUE)
option(GEN_DISASM "Generate disassembly after build" ON)
option(GEN_BINHEX "Generate .bin/.hex after build"   ON)
#################################################################################################

include(cmake/buildGuards.cmake)
set(ART_DIR "${CMAKE_BINARY_DIR}/outputBuild")
file(MAKE_DIRECTORY "${ART_DIR}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

if("ob" IN_LIST CMAKE_C_IMPLICIT_LINK_LIBRARIES)
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
endif()

message(STATUS "###################################################################")
message(STATUS "Project     : ${CMAKE_PROJECT_NAME}")
message(STATUS "Description : ${CMAKE_PROJECT_DESCRIPTION}")
message(STATUS "Version     : ${CMAKE_PROJECT_VERSION}")
message(STATUS "Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "###################################################################")
#################################################################################################

if(BOARD STREQUAL "stm32f446re")
    add_subdirectory("${CMAKE_SOURCE_DIR}/F4Board")
    set(OPENOCD_TARGET_CFG "target/stm32f4x.cfg")
else()
    message(FATAL_ERROR "Unknown BOARD='${BOARD}'. Add it to CMakePresets.json and here.")
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/F4Board/Middlewares")
add_subdirectory("${CMAKE_SOURCE_DIR}/F4gCore")
add_subdirectory("${CMAKE_SOURCE_DIR}/Bsp")
add_subdirectory("${CMAKE_SOURCE_DIR}/Utilities")

set(PROJECT_SOURCES
    "${CMAKE_SOURCE_DIR}/Src/main.c"
    "${CMAKE_SOURCE_DIR}/Src/syscalls.c"
    "${CMAKE_SOURCE_DIR}/Src/sysmem.c"
    "${CMAKE_SOURCE_DIR}/Src/system_initialization.c"
    "${CMAKE_SOURCE_DIR}/Src/freertos_hooks.c"
)

set(PROJECT_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/Inc"
)

set(PROJECT_DEFINES
)

add_executable(${EXECUTABLE})
set_target_properties(${EXECUTABLE} PROPERTIES SUFFIX ".elf")

# Place all artifacts in ART_DIR
set_target_properties(${EXECUTABLE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${ART_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${ART_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${ART_DIR}"
    OUTPUT_NAME              "${EXECUTABLE}"
)


target_sources(${EXECUTABLE} PRIVATE
    ${PROJECT_SOURCES}
)

target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_INCLUDE_DIRS})
target_compile_definitions(${EXECUTABLE} PRIVATE ${PROJECT_DEFINES})

target_link_libraries(${EXECUTABLE} PRIVATE
    boardLib
    f4gCoreLib
    bspLib
    utilitiesLib
    freertos_kernel
    freertos_kernel_port
)

target_compile_options(${EXECUTABLE} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -Wno-volatile
        -Wold-style-cast
        -Wuseless-cast
        -Wsuggest-override
    >
    $<$<NOT:$<CONFIG:Debug>>:-Werror>
)

set_target_properties(${EXECUTABLE} PROPERTIES
    ADDITIONAL_CLEAN_FILES
        "${ART_DIR}/${EXECUTABLE}.map"
        "${ART_DIR}/${EXECUTABLE}.hex"
        "${ART_DIR}/${EXECUTABLE}.bin"
        "${ART_DIR}/${EXECUTABLE}.disasm"
        "${ART_DIR}/${EXECUTABLE}.size.txt"
)

# Linker options for this executable.
target_link_options(${EXECUTABLE} PRIVATE
    -Wl,-Map,${ART_DIR}/${EXECUTABLE}.map          
    -Wl,--gc-sections                              
    --specs=nano.specs                              
    --specs=nosys.specs                             
    -Wl,--start-group
        -lc                                         
        -lm                                        
        -lgcc                                       
        -lstdc++                                    
    -Wl,--end-group
    -Wl,--print-memory-usage                       
)

if(GEN_DISASM)
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} -h -S "$<TARGET_FILE:${EXECUTABLE}>"
                > "$<TARGET_FILE_DIR:${EXECUTABLE}>/${EXECUTABLE}.disasm"
        COMMAND ${CMAKE_SIZE} "$<TARGET_FILE:${EXECUTABLE}>"
                > "$<TARGET_FILE_DIR:${EXECUTABLE}>/${EXECUTABLE}.size.txt"
        VERBATIM
        COMMENT "Generating disassembly + size report"
    )
endif()

if(GEN_BINHEX)
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex
                "$<TARGET_FILE:${EXECUTABLE}>"
                "$<TARGET_FILE_DIR:${EXECUTABLE}>/${EXECUTABLE}.hex"
        COMMAND ${CMAKE_OBJCOPY} -O binary
                "$<TARGET_FILE:${EXECUTABLE}>"
                "$<TARGET_FILE_DIR:${EXECUTABLE}>/${EXECUTABLE}.bin"
        VERBATIM
        COMMENT "Generating .hex + .bin"
    )
endif()

add_custom_target(flash
    COMMAND openocd
            -f interface/stlink.cfg
            -f "${OPENOCD_TARGET_CFG}"
            -c "program $<TARGET_FILE:${EXECUTABLE}> verify reset exit"
    DEPENDS ${EXECUTABLE}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    VERBATIM
    COMMENT "Flashing ${BOARD} via OpenOCD"
)

add_custom_target(purge
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${ART_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ART_DIR}"
    COMMENT "Purging all artifacts in ${ART_DIR}"
    VERBATIM
)

include(cmake/toolChainProbe.cmake)
toolchain_probe("${CMAKE_BINARY_DIR}/toolchain_probe.txt")
message(STATUS "Board            : ${BOARD}")
message(STATUS "Artifacts dir    : ${ART_DIR}")
message(STATUS "Linker script    : ${BOARD_LINKER_SCRIPT}")
message(STATUS "Startup (ASM)    : ${BOARD_STARTUP_ASM}")
message(STATUS "System provider  : ${BOARD_SYSTEM_C}")
message(STATUS "Toolchain probe  : ${CMAKE_BINARY_DIR}/toolchain_probe.txt")
message(STATUS "C   flags DEBUG  : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "C   flags RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "C++ flags DEBUG  : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "FreeRTOS port    : GCC_ARM_CM4F (local)")
message(STATUS "FreeRTOS heap    : heap_4 (local)")
#################################################################################################
