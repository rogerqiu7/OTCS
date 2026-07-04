#pragma once

#include <cstdint>

#include "spacecraft_types.hpp"
#include "text_protocol.hpp"

namespace otcs {

// FlightComputer is the host-side version of the spacecraft brain. It owns the
// spacecraft state and exposes small operations that are easy to test on a PC.
// Later, Pico firmware can call similar logic while replacing this demo's
// console output with USB serial I/O.
class FlightComputer {
public:
    [[nodiscard]] TelemetrySnapshot telemetry() const;

    // A tick represents time passing in the simulated spacecraft. It updates
    // clocks, telemetry sequence numbers, mode transitions, and simulated
    // battery drain.
    TelemetrySnapshot tick(std::uint32_t elapsed_ms);

    // Commands are already parsed by the protocol layer. This method applies
    // the command to flight state and returns the acknowledgement that would be
    // sent back to the Ground Station.
    [[nodiscard]] Acknowledgement handle_command(const Command& command);

private:
    void enter_safe_if_faulted();
    void drain_battery();

    SpacecraftMode mode_{SpacecraftMode::Boot};
    FaultFlags fault_flags_{0};
    std::uint32_t timestamp_ms_{0};
    std::uint32_t uptime_ms_{0};
    std::uint32_t sequence_{0};
    std::uint8_t battery_percent_{100};
    std::int16_t temperature_c_{22};
    bool boot_complete_{false};
};

} // namespace otcs
