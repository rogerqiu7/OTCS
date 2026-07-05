#include <exception>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#endif

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

void print_acknowledgement_status(const otcs::Acknowledgement& acknowledgement)
{
    std::cout << "ACK: " << otcs::to_string(acknowledgement.command_type) << ' '
              << otcs::to_string(acknowledgement.result) << "\n\n";
}

std::optional<otcs::Command> parse_user_command(const std::string& input)
{
    if (input.empty()) {
        return std::nullopt;
    }

    std::string normalized = input;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](const unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });

    if (normalized.rfind("CMD ", 0) == 0) {
        return otcs::parse_command(normalized);
    }

    return otcs::parse_command("CMD " + normalized);
}

void redraw_prompt(const std::string& input_buffer)
{
    std::cout << "> " << input_buffer << std::flush;
}

bool poll_console_command(std::string& command, std::string& input_buffer)
{
#ifdef _WIN32
    if (!_kbhit()) {
        return false;
    }

    const int key = _getch();
    if (key == 0 || key == 224) {
        (void)_getch();
        return false;
    }

    if (key == '\r' || key == '\n') {
        std::cout << '\n';
        command = input_buffer;
        input_buffer.clear();
        return true;
    }

    if (key == '\b') {
        if (!input_buffer.empty()) {
            input_buffer.pop_back();
            std::cout << "\b \b" << std::flush;
        }
        return false;
    }

    if (key >= 32 && key <= 126) {
        input_buffer.push_back(static_cast<char>(key));
        std::cout << static_cast<char>(key) << std::flush;
    }

    return false;
#else
    (void)command;
    (void)input_buffer;
    return false;
#endif
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
                  << "Waiting for telemetry. Type commands such as SET_MODE SAFE or INJECT_FAULT LOW_BATTERY.\n"
                  << "Type EXIT to stop, or press Ctrl+C.\n\n";

        std::string input_buffer;
        redraw_prompt(input_buffer);

        while (true) {
            std::string line;
            if (serial_port.try_read_line(line) && !line.empty()) {
                std::cout << "\nRX: " << line << '\n';

                const auto acknowledgement = otcs::parse_acknowledgement(line);
                if (acknowledgement.has_value()) {
                    print_acknowledgement_status(*acknowledgement);
                } else {
                    const auto telemetry = otcs::parse_telemetry(line);
                    if (telemetry.has_value()) {
                        print_telemetry_status(*telemetry);
                    } else {
                        std::cout << "Ignored: message is not valid OTCS telemetry or acknowledgement.\n\n";
                    }
                }

                redraw_prompt(input_buffer);
            }

            std::string user_input;
            if (poll_console_command(user_input, input_buffer)) {
                if (user_input == "EXIT" || user_input == "QUIT") {
                    std::cout << "Stopping Ground Station.\n";
                    return 0;
                }

                const auto command = parse_user_command(user_input);
                if (!command.has_value()) {
                    std::cout << "Invalid command. Try PING, SET_MODE SAFE, INJECT_FAULT LOW_BATTERY, or CLEAR_FAULT LOW_BATTERY.\n\n";
                    redraw_prompt(input_buffer);
                    continue;
                }

                const std::string message = otcs::format_command(*command);
                serial_port.write_line(message);
                std::cout << "TX: " << message << "\n\n";
                redraw_prompt(input_buffer);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (const std::exception& error) {
        std::cerr << "Ground Station error: " << error.what() << '\n';
        return 1;
    }
}
