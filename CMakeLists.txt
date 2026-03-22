#===========================================================================================================
# @file        ROOT/CMakeLists.txt
# @author      Mohamed GALY
# @brief       CMake configuration for the entire project.
# @details     This CMakeLists.txt sets up the project structure, including board configuration,
#              support libraries, main executable, and post-build commands for generating artifacts.
# @date        15-06-2026
# @version     1.0.0
#===========================================================================================================

#################################################################################################
# Initial CMake setup                                        
#################################################################################################
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

# Final output binary name matches the top-level project name
set(EXECUTABLE ${CMAKE_PROJECT_NAME})

# Generate compile_commands.json for clangd / IDE language-server support
set(CMAKE_EXPORT_COMPILE_COMMANDS                    TRUE)

# Options: generate disassembly / .bin + .hex files after every build
option(GEN_DISASM "Generate disassembly after build" ON)
option(GEN_BINHEX "Generate .bin/.hex after build"   ON)
#################################################################################################


#################################################################################################
# Verify build environment (C standard, in-source build guard, source-dir move detection)
#################################################################################################
include(cmake/buildGuards.cmake)
#################################################################################################


#################################################################################################
# Create the output directory if it doesn't exist.  
# This is where the final .elf, .hex, .bin, .map, and .disasm files will be placed.
#################################################################################################
set(ART_DIR "${CMAKE_BINARY_DIR}/outputBuild")
file(MAKE_DIRECTORY "${ART_DIR}")
#################################################################################################


#################################################################################################
# Default to Debug if not defined by commmand line or CMakePresets.json
#################################################################################################
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()
#################################################################################################


#################################################################################################
# "ob" implicit link library that does not exist. Remove it if present.
#################################################################################################
if("ob" IN_LIST CMAKE_C_IMPLICIT_LINK_LIBRARIES)
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
endif()
#################################################################################################


#################################################################################################
message(STATUS "###################################################################")
message(STATUS "Project     : ${CMAKE_PROJECT_NAME}")
message(STATUS "Description : ${CMAKE_PROJECT_DESCRIPTION}")
message(STATUS "Version     : ${CMAKE_PROJECT_VERSION}")
message(STATUS "Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "###################################################################")
#################################################################################################


#################################################################################################
# Board configuration                                        
#################################################################################################
# BOARD is set by CMakePresets.json.
if(BOARD STREQUAL "stm32f446re")
    add_subdirectory("${CMAKE_SOURCE_DIR}/F4Board")
    set(OPENOCD_TARGET_CFG "target/stm32f4x.cfg")
else()
    message(FATAL_ERROR "Unknown BOARD='${BOARD}'. Add it to CMakePresets.json and here.")
endif()
#################################################################################################


#################################################################################################
# FreeRTOS Kernel (local sources in Middlewares/)
# Provides: freertos_config, freertos_kernel, freertos_kernel_port
#################################################################################################
add_subdirectory("${CMAKE_SOURCE_DIR}/F4Board/Middlewares")
#################################################################################################

#################################################################################################
# Support Libraries                                          
#################################################################################################
add_subdirectory("${CMAKE_SOURCE_DIR}/F4gCore")
add_subdirectory("${CMAKE_SOURCE_DIR}/Bsp")
add_subdirectory("${CMAKE_SOURCE_DIR}/Utilities")
#################################################################################################


#################################################################################################
# Project Sources / Includes / Defines                              
#################################################################################################
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
    $<$<CONFIG:Debug>:TESTING=1>
)
#################################################################################################


#################################################################################################
# Main Executable                                           
#################################################################################################
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

# Register all generated artifacts so 'cmake --build ... --target clean' removes them
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
    -Wl,-Map,${ART_DIR}/${EXECUTABLE}.map           # Generate memory map with cross-refs
    -Wl,--gc-sections                               # Discard unused code/data sections
    --specs=nano.specs                              # Link newlib-nano (smaller C runtime)
    --specs=nosys.specs                             # Stub syscalls (no semihosting)
    -Wl,--start-group
        -lc                                         # C runtime (newlib)
        -lm                                         # Math library
        -lgcc                                       # GCC runtime helpers (soft-fp, etc.)
        -lstdc++                                    # C++ runtime
    -Wl,--end-group
    -Wl,--print-memory-usage                        # Print flash/RAM usage after link
)
#################################################################################################


#################################################################################################
#                                  Post Build Commands                                          #
#################################################################################################
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
#################################################################################################


#################################################################################################
#                              OpenOCD — Flash target                                           #
#################################################################################################
# OPENOCD_TARGET_CFG is set in the board if/else block above (single source of truth)
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
#################################################################################################


#################################################################################################
#                                  Purge target                                                 #
#################################################################################################
# 'purge' removes every file inside ART_DIR (ELF, hex, bin, map, disasm, size).
# Unlike the built-in 'clean' target (which removes object files and the listed
# ADDITIONAL_CLEAN_FILES), purge wipes the entire artifacts output folder so you
# get a guaranteed clean slate before a release build.
#
# Usage:  cmake --build --preset <preset> --target purge
add_custom_target(purge
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${ART_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ART_DIR}"
    COMMENT "Purging all artifacts in ${ART_DIR}"
    VERBATIM
)
#################################################################################################


#################################################################################################
#                             Build configuration summary                                       #
#################################################################################################

#################################################################################################
# Capture toolchain versions to a file for CI provenance and local debugging
include(cmake/toolChainProbe.cmake)
toolchain_probe("${CMAKE_BINARY_DIR}/toolchain_probe.txt")
#################################################################################################
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
