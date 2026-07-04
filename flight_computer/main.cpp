#include <iostream>

#include "flight_computer.hpp"
#include "text_protocol.hpp"

int main()
{
    otcs::FlightComputer flight_computer;

    std::cout << "OTCS Flight Computer host simulator\n\n";

    std::cout << "TX: " << otcs::format_telemetry(flight_computer.tick(0)) << '\n';
    std::cout << "TX: " << otcs::format_telemetry(flight_computer.tick(1000)) << '\n';

    const auto set_safe = otcs::parse_command("CMD SET_MODE SAFE");
    if (set_safe.has_value()) {
        std::cout << "RX: " << otcs::format_command(*set_safe) << '\n';
        std::cout << "TX: " << otcs::format_acknowledgement(flight_computer.handle_command(*set_safe)) << '\n';
    }

    std::cout << "TX: " << otcs::format_telemetry(flight_computer.tick(1000)) << '\n';

    const auto inject_fault = otcs::parse_command("CMD INJECT_FAULT LOW_BATTERY");
    if (inject_fault.has_value()) {
        std::cout << "RX: " << otcs::format_command(*inject_fault) << '\n';
        std::cout << "TX: " << otcs::format_acknowledgement(flight_computer.handle_command(*inject_fault))
                  << '\n';
    }

    std::cout << "TX: " << otcs::format_telemetry(flight_computer.tick(1000)) << '\n';

    return 0;
}
