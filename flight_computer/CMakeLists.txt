add_executable(otcs_flight_computer_host_demo
    main.cpp
)

target_link_libraries(otcs_flight_computer_host_demo
    PRIVATE
        otcs_protocol
)
