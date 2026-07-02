#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace otcs {

// These types are the shared vocabulary for the whole system. Keeping them in
// common/ lets the Ground Station, Flight Computer, protocol code, and tests
// agree on the same names and meanings.

enum class SpacecraftMode {
    Boot,
    Normal,
    Safe,
    Fault,
};

enum class Fault : std::uint32_t {
    None = 0,
    LowBattery = 1u << 0u,
    HighTemperature = 1u << 1u,
    SensorOffline = 1u << 2u,
    CommsTimeout = 1u << 3u,
    InvalidCommand = 1u << 4u,
};

using FaultFlags = std::uint32_t;

enum class CommandType {
    Ping,
    Reset,
    SetMode,
    ResetFault,
    InjectFault,
    ClearFault,
    RequestStatus,
};

enum class CommandResult {
    Ok,
    Rejected,
    Invalid,
};

// A snapshot is one complete health report from the spacecraft. The protocol
// layer will later turn this struct into text or binary telemetry messages.
struct TelemetrySnapshot {
    std::uint8_t spacecraft_id{1};
    std::uint32_t timestamp_ms{0};
    std::uint32_t sequence{0};
    SpacecraftMode mode{SpacecraftMode::Boot};
    std::uint8_t battery_percent{100};
    std::int16_t temperature_c{22};
    FaultFlags fault_flags{0};
    std::uint32_t uptime_ms{0};
};

// Commands are the Ground Station's requests to the Flight Computer. Some
// commands use optional fields; for example, SetMode uses requested_mode.
struct Command {
    CommandType type{CommandType::Ping};
    std::optional<SpacecraftMode> requested_mode{};
    std::optional<Fault> requested_fault{};
};

std::string_view to_string(SpacecraftMode mode);
std::optional<SpacecraftMode> parse_spacecraft_mode(std::string_view text);

std::string_view to_string(Fault fault);
std::optional<Fault> parse_fault(std::string_view text);

std::string_view to_string(CommandType command_type);
std::optional<CommandType> parse_command_type(std::string_view text);

std::string_view to_string(CommandResult result);
std::optional<CommandResult> parse_command_result(std::string_view text);

FaultFlags to_fault_flag(Fault fault);
bool has_fault(FaultFlags flags, Fault fault);
FaultFlags add_fault(FaultFlags flags, Fault fault);
FaultFlags clear_fault(FaultFlags flags, Fault fault);

} // namespace otcs
