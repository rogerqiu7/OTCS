#include "text_protocol.hpp"

#include <charconv>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace otcs {
namespace {

std::vector<std::string_view> split_on_spaces(const std::string_view text)
{
    std::vector<std::string_view> tokens;
    std::size_t start = 0;

    while (start < text.size()) {
        while (start < text.size() && text[start] == ' ') {
            ++start;
        }

        const std::size_t end = text.find(' ', start);
        if (end == std::string_view::npos) {
            if (start < text.size()) {
                tokens.push_back(text.substr(start));
            }
            break;
        }

        if (end > start) {
            tokens.push_back(text.substr(start, end - start));
        }
        start = end + 1;
    }

    return tokens;
}

std::optional<std::string_view> field_value(const std::string_view token, const std::string_view expected_name)
{
    const std::size_t equals = token.find('=');
    if (equals == std::string_view::npos) {
        return std::nullopt;
    }

    if (token.substr(0, equals) != expected_name) {
        return std::nullopt;
    }

    return token.substr(equals + 1);
}

template <typename Integer>
std::optional<Integer> parse_integer(const std::string_view text)
{
    Integer value{};
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);

    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }

    return value;
}

std::optional<std::uint8_t> parse_u8_field(const std::string_view token, const std::string_view expected_name)
{
    const auto value_text = field_value(token, expected_name);
    if (!value_text.has_value()) {
        return std::nullopt;
    }

    const auto parsed = parse_integer<unsigned int>(*value_text);
    if (!parsed.has_value() || *parsed > std::numeric_limits<std::uint8_t>::max()) {
        return std::nullopt;
    }

    return static_cast<std::uint8_t>(*parsed);
}

std::optional<std::uint32_t> parse_u32_field(const std::string_view token, const std::string_view expected_name)
{
    const auto value_text = field_value(token, expected_name);
    if (!value_text.has_value()) {
        return std::nullopt;
    }

    return parse_integer<std::uint32_t>(*value_text);
}

std::optional<std::int16_t> parse_i16_field(const std::string_view token, const std::string_view expected_name)
{
    const auto value_text = field_value(token, expected_name);
    if (!value_text.has_value()) {
        return std::nullopt;
    }

    return parse_integer<std::int16_t>(*value_text);
}

std::optional<SpacecraftMode> parse_mode_field(const std::string_view token)
{
    const auto value_text = field_value(token, "MODE");
    if (!value_text.has_value()) {
        return std::nullopt;
    }

    return parse_spacecraft_mode(*value_text);
}

std::optional<FaultFlags> parse_fault_flags_field(const std::string_view token)
{
    const auto value_text = field_value(token, "FAULTS");
    if (!value_text.has_value()) {
        return std::nullopt;
    }

    return parse_integer<FaultFlags>(*value_text);
}

} // namespace

std::string format_telemetry(const TelemetrySnapshot& snapshot)
{
    // Keep the first text telemetry shape stable and easy to inspect in a
    // serial monitor. Every field is named so logs remain understandable.
    std::ostringstream output;
    output << "TM"
           << " SAT=" << static_cast<unsigned int>(snapshot.spacecraft_id)
           << " TIME=" << snapshot.timestamp_ms
           << " SEQ=" << snapshot.sequence
           << " MODE=" << to_string(snapshot.mode)
           << " TEMP=" << snapshot.temperature_c
           << " BAT=" << static_cast<unsigned int>(snapshot.battery_percent)
           << " FAULTS=" << snapshot.fault_flags
           << " UPTIME=" << snapshot.uptime_ms;
    return output.str();
}

std::optional<TelemetrySnapshot> parse_telemetry(const std::string_view message)
{
    // This parser is intentionally strict: exact message type, exact field
    // order, and no missing fields. That makes failures obvious during bring-up.
    const auto tokens = split_on_spaces(message);
    if (tokens.size() != 9 || tokens[0] != "TM") {
        return std::nullopt;
    }

    const auto spacecraft_id = parse_u8_field(tokens[1], "SAT");
    const auto timestamp_ms = parse_u32_field(tokens[2], "TIME");
    const auto sequence = parse_u32_field(tokens[3], "SEQ");
    const auto mode = parse_mode_field(tokens[4]);
    const auto temperature_c = parse_i16_field(tokens[5], "TEMP");
    const auto battery_percent = parse_u8_field(tokens[6], "BAT");
    const auto fault_flags = parse_fault_flags_field(tokens[7]);
    const auto uptime_ms = parse_u32_field(tokens[8], "UPTIME");

    if (!spacecraft_id.has_value() || !timestamp_ms.has_value() || !sequence.has_value() || !mode.has_value()
        || !temperature_c.has_value() || !battery_percent.has_value() || !fault_flags.has_value()
        || !uptime_ms.has_value()) {
        return std::nullopt;
    }

    if (*battery_percent > 100u) {
        return std::nullopt;
    }

    return TelemetrySnapshot{
        *spacecraft_id,
        *timestamp_ms,
        *sequence,
        *mode,
        *battery_percent,
        *temperature_c,
        *fault_flags,
        *uptime_ms,
    };
}

std::string format_command(const Command& command)
{
    std::ostringstream output;
    output << "CMD " << to_string(command.type);

    if (command.type == CommandType::SetMode && command.requested_mode.has_value()) {
        output << ' ' << to_string(*command.requested_mode);
    } else if ((command.type == CommandType::InjectFault || command.type == CommandType::ClearFault)
               && command.requested_fault.has_value()) {
        output << ' ' << to_string(*command.requested_fault);
    }

    return output.str();
}

std::optional<Command> parse_command(const std::string_view message)
{
    const auto tokens = split_on_spaces(message);
    if (tokens.size() < 2 || tokens[0] != "CMD") {
        return std::nullopt;
    }

    const auto command_type = parse_command_type(tokens[1]);
    if (!command_type.has_value()) {
        return std::nullopt;
    }

    Command command{*command_type};

    if (*command_type == CommandType::SetMode) {
        if (tokens.size() != 3) {
            return std::nullopt;
        }

        const auto requested_mode = parse_spacecraft_mode(tokens[2]);
        if (!requested_mode.has_value()) {
            return std::nullopt;
        }

        command.requested_mode = *requested_mode;
        return command;
    }

    if (*command_type == CommandType::InjectFault || *command_type == CommandType::ClearFault) {
        if (tokens.size() != 3) {
            return std::nullopt;
        }

        const auto requested_fault = parse_fault(tokens[2]);
        if (!requested_fault.has_value() || *requested_fault == Fault::None) {
            return std::nullopt;
        }

        command.requested_fault = *requested_fault;
        return command;
    }

    if (tokens.size() != 2) {
        return std::nullopt;
    }

    return command;
}

std::string format_acknowledgement(const Acknowledgement& acknowledgement)
{
    std::ostringstream output;
    output << "ACK " << to_string(acknowledgement.command_type) << ' ' << to_string(acknowledgement.result);
    return output.str();
}

std::optional<Acknowledgement> parse_acknowledgement(const std::string_view message)
{
    const auto tokens = split_on_spaces(message);
    if (tokens.size() != 3 || tokens[0] != "ACK") {
        return std::nullopt;
    }

    const auto command_type = parse_command_type(tokens[1]);
    const auto result = parse_command_result(tokens[2]);
    if (!command_type.has_value() || !result.has_value()) {
        return std::nullopt;
    }

    return Acknowledgement{*command_type, *result};
}

} // namespace otcs
