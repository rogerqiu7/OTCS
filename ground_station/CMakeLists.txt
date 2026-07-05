add_executable(otcs_ground_station
    main.cpp
    serial_port.cpp
    status_banner.cpp
)

target_link_libraries(otcs_ground_station
    PRIVATE
        otcs_protocol
)
