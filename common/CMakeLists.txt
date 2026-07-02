add_library(otcs_common
    spacecraft_types.cpp
)

target_include_directories(otcs_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
