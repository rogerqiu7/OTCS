#include <exception>
#include <iostream>
#include <string>

#include "serial_port.hpp"
#include "status_banner.hpp"
#include "text_protocol.hpp"

namespace {

void print_usage(const char* executable_name)
{
    std::cout << "Usage: " << executable_name << " <serial-port>\n"
              << "Example: " << executable_name << " COM3\n";
}

void print_telemetry_status(const otcs::TelemetrySnapshot& telemetry)
{
    std::cout << "SAT-" << static_cast<unsigned int>(telemetry.spacecraft_id) << '\n'
              << "  Mode:    " << otcs::to_string(telemetry.mode) << '\n'
              << "  Battery: " << static_cast<unsigned int>(telemetry.battery_percent) << "%\n"
              << "  Temp:    " << telemetry.temperature_c << " C\n"
              << "  Faults:  " << telemetry.fault_flags << '\n'
              << "  Uptime:  " << telemetry.uptime_ms << " ms\n"
              << "  Seq:     " << telemetry.sequence << "\n\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string port_name = argv[1];

    try {
        print_status_banner();

        otcs::SerialPort serial_port{port_name, 115200};
        std::cout << "Connected to " << serial_port.port_name() << " at 115200 baud.\n"
                  << "Waiting for telemetry. Press Ctrl+C to stop.\n\n";

        while (true) {
            const std::string line = serial_port.read_line();
            if (line.empty()) {
                continue;
            }

            std::cout << "RX: " << line << '\n';

            const auto telemetry = otcs::parse_telemetry(line);
            if (telemetry.has_value()) {
                print_telemetry_status(*telemetry);
            } else {
                std::cout << "Ignored: message is not valid OTCS telemetry.\n\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Ground Station error: " << error.what() << '\n';
        return 1;
    }
}
