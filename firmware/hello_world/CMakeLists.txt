cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "Set PICO_SDK_PATH to your pico-sdk directory before configuring this firmware.")
endif()

include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(otcs_pico_hello VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

pico_sdk_init()

add_executable(otcs_pico_hello
    main.cpp
)

target_link_libraries(otcs_pico_hello
    pico_stdlib
)

pico_enable_stdio_usb(otcs_pico_hello 1)
pico_enable_stdio_uart(otcs_pico_hello 0)

pico_add_extra_outputs(otcs_pico_hello)
