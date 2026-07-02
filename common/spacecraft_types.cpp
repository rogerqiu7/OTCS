#include "spacecraft_types.hpp"

namespace otcs {

std::string_view to_string(const SpacecraftMode mode)
{
    switch (mode) {
    case SpacecraftMode::Boot:
        return "BOOT";
    case SpacecraftMode::Normal:
        return "NORMAL";
    case SpacecraftMode::Safe:
        return "SAFE";
    case SpacecraftMode::Fault:
        return "FAULT";
    }

    return "UNKNOWN";
}

std::optional<SpacecraftMode> parse_spacecraft_mode(const std::string_view text)
{
    if (text == "BOOT") {
        return SpacecraftMode::Boot;
    }
    if (text == "NORMAL") {
        return SpacecraftMode::Normal;
    }
    if (text == "SAFE") {
        return SpacecraftMode::Safe;
    }
    if (text == "FAULT") {
        return SpacecraftMode::Fault;
    }

    return std::nullopt;
}

std::string_view to_string(const Fault fault)
{
    switch (fault) {
    case Fault::None:
        return "NONE";
    case Fault::LowBattery:
        return "LOW_BATTERY";
    case Fault::HighTemperature:
        return "HIGH_TEMPERATURE";
    case Fault::SensorOffline:
        return "SENSOR_OFFLINE";
    case Fault::CommsTimeout:
        return "COMMS_TIMEOUT";
    case Fault::InvalidCommand:
        return "INVALID_COMMAND";
    }

    return "UNKNOWN";
}

std::optional<Fault> parse_fault(const std::string_view text)
{
    if (text == "NONE") {
        return Fault::None;
    }
    if (text == "LOW_BATTERY") {
        return Fault::LowBattery;
    }
    if (text == "HIGH_TEMPERATURE") {
        return Fault::HighTemperature;
    }
    if (text == "SENSOR_OFFLINE") {
        return Fault::SensorOffline;
    }
    if (text == "COMMS_TIMEOUT") {
        return Fault::CommsTimeout;
    }
    if (text == "INVALID_COMMAND") {
        return Fault::InvalidCommand;
    }

    return std::nullopt;
}

std::string_view to_string(const CommandType command_type)
{
    switch (command_type) {
    case CommandType::Ping:
        return "PING";
    case CommandType::Reset:
        return "RESET";
    case CommandType::SetMode:
        return "SET_MODE";
    case CommandType::ResetFault:
        return "RESET_FAULT";
    case CommandType::InjectFault:
        return "INJECT_FAULT";
    case CommandType::ClearFault:
        return "CLEAR_FAULT";
    case CommandType::RequestStatus:
        return "REQUEST_STATUS";
    }

    return "UNKNOWN";
}

std::optional<CommandType> parse_command_type(const std::string_view text)
{
    if (text == "PING") {
        return CommandType::Ping;
    }
    if (text == "RESET") {
        return CommandType::Reset;
    }
    if (text == "SET_MODE") {
        return CommandType::SetMode;
    }
    if (text == "RESET_FAULT") {
        return CommandType::ResetFault;
    }
    if (text == "INJECT_FAULT") {
        return CommandType::InjectFault;
    }
    if (text == "CLEAR_FAULT") {
        return CommandType::ClearFault;
    }
    if (text == "REQUEST_STATUS") {
        return CommandType::RequestStatus;
    }

    return std::nullopt;
}

std::string_view to_string(const CommandResult result)
{
    switch (result) {
    case CommandResult::Ok:
        return "OK";
    case CommandResult::Rejected:
        return "REJECTED";
    case CommandResult::Invalid:
        return "INVALID";
    }

    return "UNKNOWN";
}

std::optional<CommandResult> parse_command_result(const std::string_view text)
{
    if (text == "OK") {
        return CommandResult::Ok;
    }
    if (text == "REJECTED") {
        return CommandResult::Rejected;
    }
    if (text == "INVALID") {
        return CommandResult::Invalid;
    }

    return std::nullopt;
}

FaultFlags to_fault_flag(const Fault fault)
{
    return static_cast<FaultFlags>(fault);
}

bool has_fault(const FaultFlags flags, const Fault fault)
{
    return (flags & to_fault_flag(fault)) != 0u;
}

FaultFlags add_fault(const FaultFlags flags, const Fault fault)
{
    return flags | to_fault_flag(fault);
}

FaultFlags clear_fault(const FaultFlags flags, const Fault fault)
{
    return flags & ~to_fault_flag(fault);
}

} // namespace otcs
