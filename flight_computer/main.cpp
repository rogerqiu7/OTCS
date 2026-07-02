#include <iostream>

#include "text_protocol.hpp"

int main()
{
    std::cout << "OTCS Flight Computer host demo\n";
    std::cout << "Embedded-side target is wired into CMake.\n";

    otcs::TelemetrySnapshot snapshot;
    snapshot.timestamp_ms = 12000;
    snapshot.sequence = 42;
    snapshot.mode = otcs::SpacecraftMode::Normal;
    snapshot.battery_percent = 96;
    snapshot.temperature_c = 23;
    snapshot.uptime_ms = 12000;

    snapshot.fault_flags = otcs::add_fault(snapshot.fault_flags, otcs::Fault::LowBattery);

    const otcs::Acknowledgement acknowledgement{
        otcs::CommandType::SetMode,
        otcs::CommandResult::Ok,
    };

    std::cout << "\nSample outbound messages from the Flight Computer:\n";
    std::cout << "Telemetry: " << otcs::format_telemetry(snapshot) << '\n';
    std::cout << "Ack:       " << otcs::format_acknowledgement(acknowledgement) << '\n';

    return 0;
}
