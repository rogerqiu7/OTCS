#pragma once

#include <cstdint>
#include <string>
#include <string_view>

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
    [[nodiscard]] bool try_read_line(std::string& line);
    void write_line(std::string_view line);

private:
    void close();

    std::string port_name_;
    std::string pending_line_;

#ifdef _WIN32
    void* handle_{nullptr};
#else
    int handle_{-1};
#endif
};

} // namespace otcs
