add_library(otcs_flight_computer
    flight_computer.cpp
)

target_include_directories(otcs_flight_computer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(otcs_flight_computer
    PUBLIC
        otcs_protocol
)

add_executable(otcs_flight_computer_host_demo
    main.cpp
)

target_link_libraries(otcs_flight_computer_host_demo
    PRIVATE
        otcs_flight_computer
        otcs_protocol
)
