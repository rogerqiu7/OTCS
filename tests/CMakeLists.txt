add_executable(otcs_common_tests
    common_tests.cpp
)

target_link_libraries(otcs_common_tests
    PRIVATE
        otcs_common
)

add_test(NAME otcs_common_tests COMMAND otcs_common_tests)

add_executable(otcs_protocol_tests
    protocol_tests.cpp
)

target_link_libraries(otcs_protocol_tests
    PRIVATE
        otcs_protocol
)

add_test(NAME otcs_protocol_tests COMMAND otcs_protocol_tests)
