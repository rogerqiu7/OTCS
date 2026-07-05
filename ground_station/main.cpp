#include <exception>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#endif

#include "mission_logger.hpp"
#include "serial_port.hpp"
#include "status_banner.hpp"
#include "text_protocol.hpp"

namespace {

constexpr auto link_timeout = std::chrono::seconds{3};

enum class ConsolePollResult {
    None,
    Changed,
    Submitted,
};

struct DisplayState {
    std::optional<otcs::TelemetrySnapshot> telemetry;
    std::optional<otcs::Acknowledgement> acknowledgement;
    std::string last_rx{"none"};
    std::string last_tx{"none"};
    std::string last_event{"Waiting for telemetry"};
    bool has_received_telemetry{false};
    bool link_is_stale{false};
    std::chrono::steady_clock::time_point last_telemetry_time{std::chrono::steady_clock::now()};
};

void print_usage(const char* executable_name)
{
    std::cout << "Usage: " << executable_name << " <serial-port>\n"
              << "Example: " << executable_name << " COM3\n";
}

std::string telemetry_age_text(const DisplayState& display)
{
    if (!display.has_received_telemetry) {
        return "never";
    }

    const auto age = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - display.last_telemetry_time);

    std::ostringstream output;
    output << age.count() << "s ago";
    return output.str();
}

std::string acknowledgement_text(const std::optional<otcs::Acknowledgement>& acknowledgement)
{
    if (!acknowledgement.has_value()) {
        return "none";
    }

    std::ostringstream output;
    output << otcs::to_string(acknowledgement->command_type) << ' ' << otcs::to_string(acknowledgement->result);
    return output.str();
}

void render_dashboard(const std::string& port_name,
                      const otcs::MissionLogger& mission_logger,
                      const DisplayState& display,
                      const std::string& input_buffer)
{
    std::cout << "\x1B[2J\x1B[H";
    std::cout << "OTCS Ground Station\n";
    std::cout << "===================\n\n";
    std::cout << "Connection: " << (display.link_is_stale ? "STALE" : "ONLINE") << '\n';
    std::cout << "Port:       " << port_name << " @ 115200\n";
    std::cout << "Last TM:    " << telemetry_age_text(display) << "\n\n";

    std::cout << "Spacecraft\n";
    std::cout << "----------\n";
    if (display.telemetry.has_value()) {
        const auto& telemetry = *display.telemetry;
        std::cout << "SAT:        " << static_cast<unsigned int>(telemetry.spacecraft_id) << '\n';
        std::cout << "Mode:       " << otcs::to_string(telemetry.mode) << '\n';
        std::cout << "Battery:    " << static_cast<unsigned int>(telemetry.battery_percent) << "%\n";
        std::cout << "Temp:       " << telemetry.temperature_c << " C\n";
        std::cout << "Faults:     " << telemetry.fault_flags << '\n';
        std::cout << "Uptime:     " << telemetry.uptime_ms << " ms\n";
        std::cout << "Seq:        " << telemetry.sequence << "\n\n";
    } else {
        std::cout << "No telemetry received yet.\n\n";
    }

    std::cout << "Activity\n";
    std::cout << "--------\n";
    std::cout << "Last ACK:   " << acknowledgement_text(display.acknowledgement) << '\n';
    std::cout << "Last TX:    " << display.last_tx << '\n';
    std::cout << "Last RX:    " << display.last_rx << '\n';
    std::cout << "Event:      " << display.last_event << "\n\n";

    std::cout << "Logs\n";
    std::cout << "----\n";
    std::cout << "Events:     " << mission_logger.events_path().string() << '\n';
    std::cout << "Telemetry:  " << mission_logger.telemetry_path().string() << "\n\n";

    std::cout << "Command examples: PING | RESET | SET_MODE SAFE | INJECT_FAULT LOW_BATTERY | CLEAR_FAULT LOW_BATTERY\n";
    std::cout << "> " << input_buffer << std::flush;
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

ConsolePollResult poll_console_command(std::string& command, std::string& input_buffer)
{
#ifdef _WIN32
    if (!_kbhit()) {
        return ConsolePollResult::None;
    }

    const int key = _getch();
    if (key == 0 || key == 224) {
        (void)_getch();
        return ConsolePollResult::None;
    }

    if (key == '\r' || key == '\n') {
        command = input_buffer;
        input_buffer.clear();
        return ConsolePollResult::Submitted;
    }

    if (key == '\b') {
        if (!input_buffer.empty()) {
            input_buffer.pop_back();
            return ConsolePollResult::Changed;
        }
        return ConsolePollResult::None;
    }

    if (key >= 32 && key <= 126) {
        input_buffer.push_back(static_cast<char>(key));
        return ConsolePollResult::Changed;
    }

    return ConsolePollResult::None;
#else
    (void)command;
    (void)input_buffer;
    return ConsolePollResult::None;
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
        otcs::MissionLogger mission_logger;

        mission_logger.log_connected(serial_port.port_name(), 115200);

        std::string input_buffer;
        DisplayState display;
        display.last_event = "Connected to " + serial_port.port_name();

        render_dashboard(serial_port.port_name(), mission_logger, display, input_buffer);

        while (true) {
            bool should_render = false;
            std::string line;
            if (serial_port.try_read_line(line) && !line.empty()) {
                display.last_rx = line;
                mission_logger.log_rx(line);

                const auto acknowledgement = otcs::parse_acknowledgement(line);
                if (acknowledgement.has_value()) {
                    display.acknowledgement = *acknowledgement;
                    display.last_event = "ACK " + acknowledgement_text(display.acknowledgement);
                } else {
                    const auto telemetry = otcs::parse_telemetry(line);
                    if (telemetry.has_value()) {
                        if (display.link_is_stale) {
                            mission_logger.log_note("LINK_RECOVERED");
                            display.last_event = "Connection recovered";
                            display.link_is_stale = false;
                        } else {
                            display.last_event = "Telemetry updated";
                        }

                        display.has_received_telemetry = true;
                        display.last_telemetry_time = std::chrono::steady_clock::now();
                        display.telemetry = *telemetry;
                        mission_logger.log_telemetry(*telemetry);
                    } else {
                        mission_logger.log_note("IGNORED INVALID_MESSAGE");
                        display.last_event = "Ignored invalid message";
                    }
                }

                should_render = true;
            }

            if (display.has_received_telemetry && !display.link_is_stale
                && std::chrono::steady_clock::now() - display.last_telemetry_time > link_timeout) {
                mission_logger.log_note("LINK_STALE NO_TELEMETRY_3S");
                display.link_is_stale = true;
                display.last_event = "No telemetry received for 3 seconds";
                should_render = true;
            }

            std::string user_input;
            const ConsolePollResult console_result = poll_console_command(user_input, input_buffer);
            if (console_result == ConsolePollResult::Changed) {
                should_render = true;
            } else if (console_result == ConsolePollResult::Submitted) {
                if (user_input == "EXIT" || user_input == "QUIT") {
                    mission_logger.log_note("OPERATOR_EXIT");
                    display.last_event = "Stopping Ground Station";
                    render_dashboard(serial_port.port_name(), mission_logger, display, input_buffer);
                    std::cout << "\n";
                    return 0;
                }

                const auto command = parse_user_command(user_input);
                if (!command.has_value()) {
                    mission_logger.log_note("INVALID_OPERATOR_COMMAND " + user_input);
                    display.last_event = "Invalid command: " + user_input;
                    should_render = true;
                    continue;
                }

                const std::string message = otcs::format_command(*command);
                serial_port.write_line(message);
                mission_logger.log_tx(message);
                display.last_tx = message;
                display.last_event = "Sent " + message;
                should_render = true;
            }

            if (should_render) {
                render_dashboard(serial_port.port_name(), mission_logger, display, input_buffer);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } catch (const std::exception& error) {
        std::cerr << "Ground Station error: " << error.what() << '\n';
        return 1;
    }
}
