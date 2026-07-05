#include <cstdio>
#include <string>

#include "flight_computer.hpp"
#include "pico/stdlib.h"
#include "text_protocol.hpp"

namespace {

bool poll_usb_line(std::string& line)
{
    static std::string pending_line;

    while (true) {
        const int input = getchar_timeout_us(0);
        if (input == PICO_ERROR_TIMEOUT) {
            return false;
        }

        const char byte = static_cast<char>(input);
        if (byte == '\n') {
            line = pending_line;
            pending_line.clear();
            return true;
        }

        if (byte != '\r') {
            pending_line.push_back(byte);
        }
    }
}

void process_command_line(otcs::FlightComputer& flight_computer, const std::string& line)
{
    const auto command = otcs::parse_command(line);
    if (!command.has_value()) {
        std::printf("ERR CMD INVALID\n");
        return;
    }

    const otcs::Acknowledgement acknowledgement = flight_computer.handle_command(*command);
    const std::string message = otcs::format_acknowledgement(acknowledgement);
    std::printf("%s\n", message.c_str());
}

} // namespace

int main()
{
    stdio_init_all();

    // USB serial appears a moment after firmware starts. Waiting here keeps the
    // first boot telemetry from being printed before the PC has opened the COM
    // port.
    sleep_ms(2000);

    otcs::FlightComputer flight_computer;

    while (true) {
        // One loop is one spacecraft telemetry cycle. The shared flight logic
        // advances by one second, then the shared protocol formats the snapshot
        // into the same text line the future Ground Station will parse.
        const otcs::TelemetrySnapshot telemetry = flight_computer.tick(1000);
        const std::string message = otcs::format_telemetry(telemetry);

        std::printf("%s\n", message.c_str());

        for (int poll_count = 0; poll_count < 100; ++poll_count) {
            std::string command_line;
            if (poll_usb_line(command_line) && !command_line.empty()) {
                process_command_line(flight_computer, command_line);
            }

            sleep_ms(10);
        }
    }
}
