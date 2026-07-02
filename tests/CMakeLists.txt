add_executable(otcs_common_tests
    common_tests.cpp
)

target_link_libraries(otcs_common_tests
    PRIVATE
        otcs_common
)

add_test(NAME otcs_common_tests COMMAND otcs_common_tests)
