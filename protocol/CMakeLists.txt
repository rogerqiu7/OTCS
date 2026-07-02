add_library(otcs_protocol
    text_protocol.cpp
)

target_include_directories(otcs_protocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(otcs_protocol
    PUBLIC
        otcs_common
)
