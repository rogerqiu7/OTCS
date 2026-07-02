#include <iostream>

#include "status_banner.hpp"
#include "text_protocol.hpp"

int main()
{
    std::cout << "OTCS Ground Station prototype\n";
    std::cout << "Build pipeline is working.\n";
    print_status_banner();

    const auto telemetry = otcs::parse_telemetry(
        "TM SAT=1 TIME=12000 SEQ=42 MODE=NORMAL TEMP=23 BAT=96 FAULTS=1 UPTIME=12000");
    const auto command = otcs::parse_command("CMD SET_MODE SAFE");
    const auto acknowledgement = otcs::parse_acknowledgement("ACK SET_MODE OK");

    std::cout << "\nSample inbound messages parsed by the Ground Station:\n";

    if (telemetry.has_value()) {
        std::cout << "Telemetry parsed:\n";
        std::cout << "  SAT:     " << static_cast<unsigned int>(telemetry->spacecraft_id) << '\n';
        std::cout << "  Mode:    " << otcs::to_string(telemetry->mode) << '\n';
        std::cout << "  Battery: " << static_cast<unsigned int>(telemetry->battery_percent) << "%\n";
        std::cout << "  Temp:    " << telemetry->temperature_c << " C\n";
        std::cout << "  Faults:  " << telemetry->fault_flags << '\n';
    } else {
        std::cout << "Telemetry parse failed.\n";
    }

    if (command.has_value()) {
        std::cout << "Command parsed: " << otcs::to_string(command->type);
        if (command->requested_mode.has_value()) {
            std::cout << ' ' << otcs::to_string(*command->requested_mode);
        }
        std::cout << '\n';
    } else {
        std::cout << "Command parse failed.\n";
    }

    if (acknowledgement.has_value()) {
        std::cout << "Ack parsed:     " << otcs::to_string(acknowledgement->command_type) << ' '
                  << otcs::to_string(acknowledgement->result) << '\n';
    } else {
        std::cout << "Ack parse failed.\n";
    }

    return 0;
}
