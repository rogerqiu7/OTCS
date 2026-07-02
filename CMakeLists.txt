cmake_minimum_required(VERSION 3.25)

project(
    otcs
    VERSION 0.1.0
    DESCRIPTION "Orbital Telemetry & Command System"
    LANGUAGES CXX
)

# OTCS targets C++20, but code should stay conservative and portable.
# Prefer clear architecture, deterministic behavior, and embedded-friendly
# design over showing off language features.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OTCS_ENABLE_WARNINGS "Enable strict compiler warnings" ON)
option(OTCS_BUILD_TESTS "Build OTCS tests" OFF)

if(OTCS_ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
        add_compile_options(-Wall -Wextra -Wpedantic)
    elseif(MSVC)
        add_compile_options(/W4 /EHsc)
    endif()
endif()

message(STATUS "Configuring OTCS ${PROJECT_VERSION}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Project layout placeholders.
add_subdirectory(common EXCLUDE_FROM_ALL)
add_subdirectory(protocol EXCLUDE_FROM_ALL)
add_subdirectory(flight_computer EXCLUDE_FROM_ALL)
add_subdirectory(ground_station)
add_subdirectory(tests EXCLUDE_FROM_ALL)
