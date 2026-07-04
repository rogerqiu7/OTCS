#include "flight_computer.hpp"

namespace otcs {

TelemetrySnapshot FlightComputer::telemetry() const
{
    return TelemetrySnapshot{
        1,
        timestamp_ms_,
        sequence_,
        mode_,
        battery_percent_,
        temperature_c_,
        fault_flags_,
        uptime_ms_,
    };
}

TelemetrySnapshot FlightComputer::tick(const std::uint32_t elapsed_ms)
{
    // The first tick reports BOOT before moving into normal operation. That
    // gives the Ground Station a visible startup state instead of silently
    // beginning in NORMAL.
    timestamp_ms_ += elapsed_ms;
    uptime_ms_ += elapsed_ms;
    ++sequence_;

    if (boot_complete_) {
        drain_battery();
        enter_safe_if_faulted();
    }

    const TelemetrySnapshot snapshot = telemetry();

    if (!boot_complete_) {
        boot_complete_ = true;
        mode_ = SpacecraftMode::Normal;
    }

    return snapshot;
}

Acknowledgement FlightComputer::handle_command(const Command& command)
{
    switch (command.type) {
    case CommandType::Ping:
    case CommandType::RequestStatus:
        return Acknowledgement{command.type, CommandResult::Ok};

    case CommandType::Reset:
        mode_ = SpacecraftMode::Boot;
        fault_flags_ = 0;
        timestamp_ms_ = 0;
        uptime_ms_ = 0;
        sequence_ = 0;
        battery_percent_ = 100;
        temperature_c_ = 22;
        boot_complete_ = false;
        return Acknowledgement{command.type, CommandResult::Ok};

    case CommandType::SetMode:
        if (!command.requested_mode.has_value()) {
            return Acknowledgement{command.type, CommandResult::Invalid};
        }
        if (fault_flags_ != 0 && *command.requested_mode == SpacecraftMode::Normal) {
            return Acknowledgement{command.type, CommandResult::Rejected};
        }
        mode_ = *command.requested_mode;
        return Acknowledgement{command.type, CommandResult::Ok};

    case CommandType::ResetFault:
        fault_flags_ = 0;
        if (mode_ == SpacecraftMode::Safe || mode_ == SpacecraftMode::Fault) {
            mode_ = SpacecraftMode::Normal;
        }
        return Acknowledgement{command.type, CommandResult::Ok};

    case CommandType::InjectFault:
        if (!command.requested_fault.has_value() || *command.requested_fault == Fault::None) {
            return Acknowledgement{command.type, CommandResult::Invalid};
        }
        fault_flags_ = add_fault(fault_flags_, *command.requested_fault);
        enter_safe_if_faulted();
        return Acknowledgement{command.type, CommandResult::Ok};

    case CommandType::ClearFault:
        if (!command.requested_fault.has_value() || *command.requested_fault == Fault::None) {
            return Acknowledgement{command.type, CommandResult::Invalid};
        }
        fault_flags_ = clear_fault(fault_flags_, *command.requested_fault);
        if (fault_flags_ == 0 && mode_ == SpacecraftMode::Safe) {
            mode_ = SpacecraftMode::Normal;
        }
        return Acknowledgement{command.type, CommandResult::Ok};
    }

    return Acknowledgement{command.type, CommandResult::Invalid};
}

void FlightComputer::enter_safe_if_faulted()
{
    if (fault_flags_ != 0 && mode_ == SpacecraftMode::Normal) {
        mode_ = SpacecraftMode::Safe;
    }
}

void FlightComputer::drain_battery()
{
    // Keep the host simulator deterministic: one percent per simulated second,
    // never below zero. This is intentionally simple until real power behavior
    // matters.
    if (uptime_ms_ != 0 && uptime_ms_ % 1000u == 0u && battery_percent_ > 0u) {
        --battery_percent_;
    }
}

} // namespace otcs
