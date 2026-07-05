#pragma once

#include <cstdint>
#include <string>

namespace otcs {

// SerialPort is the Ground Station's transport boundary. It hides the
// operating-system handle code so main.cpp can focus on telemetry parsing and
// display.
class SerialPort {
public:
    SerialPort(std::string port_name, std::uint32_t baud_rate);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    SerialPort(SerialPort&& other) noexcept;
    SerialPort& operator=(SerialPort&& other) noexcept;

    [[nodiscard]] const std::string& port_name() const;
    [[nodiscard]] std::string read_line();

private:
    void close();

    std::string port_name_;

#ifdef _WIN32
    void* handle_{nullptr};
#else
    int handle_{-1};
#endif
};

} // namespace otcs
