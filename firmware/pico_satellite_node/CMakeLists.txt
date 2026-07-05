# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_satellite_node C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# The Pico project lives under firmware/, but the first real satellite behavior
# comes from the shared OTCS modules at the repo root.
set(OTCS_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

# Add executable. Default name is the project name, version 0.1
add_executable(pico_satellite_node
        main.cpp
        ${OTCS_ROOT}/common/spacecraft_types.cpp
        ${OTCS_ROOT}/protocol/text_protocol.cpp
        ${OTCS_ROOT}/flight_computer/flight_computer.cpp
)

pico_set_program_name(pico_satellite_node "pico_satellite_node")
pico_set_program_version(pico_satellite_node "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_satellite_node 0)
pico_enable_stdio_usb(pico_satellite_node 1)

# Add the standard library to the build
target_link_libraries(pico_satellite_node
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico_satellite_node PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${OTCS_ROOT}/common
        ${OTCS_ROOT}/protocol
        ${OTCS_ROOT}/flight_computer
)

target_compile_features(pico_satellite_node PRIVATE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(pico_satellite_node PRIVATE
                $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
                $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        )
endif()

# Add any user requested libraries
target_link_libraries(pico_satellite_node 
        
        )

pico_add_extra_outputs(pico_satellite_node)

