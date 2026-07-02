#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "spacecraft_types.hpp"

namespace otcs {

// The text protocol is the first wire format for OTCS. It converts safe C++
// objects into plain text messages that can later travel over USB serial, logs,
// files, or Wi-Fi. A later binary protocol can reuse the same model types.

struct Acknowledgement {
    CommandType command_type{CommandType::Ping};
    CommandResult result{CommandResult::Ok};
};

// Format functions are used before sending. They turn internal C++ objects into
// one-line protocol messages.
std::string format_telemetry(const TelemetrySnapshot& snapshot);
std::string format_command(const Command& command);
std::string format_acknowledgement(const Acknowledgement& acknowledgement);

// Parse functions are used after receiving. They validate text and only return
// an object when the message matches the expected protocol shape.
std::optional<TelemetrySnapshot> parse_telemetry(std::string_view message);
std::optional<Command> parse_command(std::string_view message);
std::optional<Acknowledgement> parse_acknowledgement(std::string_view message);

} // namespace otcs
