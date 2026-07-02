#include <iostream>
#include <string_view>

#include "text_protocol.hpp"

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view message)
{
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

void test_format_telemetry()
{
    // Formatting is used by the sender. A telemetry object becomes one line of
    // text that can be printed, logged, or sent over a serial connection.
    const otcs::TelemetrySnapshot snapshot{
        1,
        12345,
        7,
        otcs::SpacecraftMode::Normal,
        95,
        22,
        otcs::add_fault(0, otcs::Fault::LowBattery),
        12345,
    };

    expect(otcs::format_telemetry(snapshot)
               == "TM SAT=1 TIME=12345 SEQ=7 MODE=NORMAL TEMP=22 BAT=95 FAULTS=1 UPTIME=12345",
           "telemetry formats to the expected text message");
}

void test_parse_telemetry()
{
    // Parsing is used by the receiver. Raw text is accepted only if it matches
    // the protocol shape and all field values are valid.
    const auto parsed = otcs::parse_telemetry(
        "TM SAT=2 TIME=5000 SEQ=9 MODE=SAFE TEMP=-4 BAT=87 FAULTS=8 UPTIME=4999");

    expect(parsed.has_value(), "valid telemetry parses");
    expect(parsed->spacecraft_id == 2, "telemetry SAT field parses");
    expect(parsed->timestamp_ms == 5000, "telemetry TIME field parses");
    expect(parsed->sequence == 9, "telemetry SEQ field parses");
    expect(parsed->mode == otcs::SpacecraftMode::Safe, "telemetry MODE field parses");
    expect(parsed->temperature_c == -4, "telemetry TEMP field parses");
    expect(parsed->battery_percent == 87, "telemetry BAT field parses");
    expect(otcs::has_fault(parsed->fault_flags, otcs::Fault::CommsTimeout), "telemetry FAULTS field parses");
    expect(parsed->uptime_ms == 4999, "telemetry UPTIME field parses");

    expect(!otcs::parse_telemetry("TM SAT=1 TIME=1 SEQ=1 MODE=SCIENCE TEMP=22 BAT=95 FAULTS=0 UPTIME=1")
                .has_value(),
           "telemetry rejects unknown modes");
    expect(!otcs::parse_telemetry("TM SAT=1 TIME=1 SEQ=1 MODE=NORMAL TEMP=22 BAT=101 FAULTS=0 UPTIME=1")
                .has_value(),
           "telemetry rejects impossible battery values");
    expect(!otcs::parse_telemetry("TM SAT=1 MODE=NORMAL").has_value(), "telemetry rejects missing fields");
}

void test_format_command()
{
    expect(otcs::format_command(otcs::Command{otcs::CommandType::Ping}) == "CMD PING",
           "PING command formats without arguments");

    otcs::Command set_mode{otcs::CommandType::SetMode};
    set_mode.requested_mode = otcs::SpacecraftMode::Safe;
    expect(otcs::format_command(set_mode) == "CMD SET_MODE SAFE", "SET_MODE command formats with mode");

    otcs::Command inject_fault{otcs::CommandType::InjectFault};
    inject_fault.requested_fault = otcs::Fault::HighTemperature;
    expect(otcs::format_command(inject_fault) == "CMD INJECT_FAULT HIGH_TEMPERATURE",
           "INJECT_FAULT command formats with fault");
}

void test_parse_command()
{
    const auto set_mode = otcs::parse_command("CMD SET_MODE SAFE");
    expect(set_mode.has_value(), "valid SET_MODE command parses");
    expect(set_mode->type == otcs::CommandType::SetMode, "SET_MODE type parses");
    expect(set_mode->requested_mode == otcs::SpacecraftMode::Safe, "SET_MODE argument parses");

    const auto clear_fault = otcs::parse_command("CMD CLEAR_FAULT LOW_BATTERY");
    expect(clear_fault.has_value(), "valid CLEAR_FAULT command parses");
    expect(clear_fault->requested_fault == otcs::Fault::LowBattery, "CLEAR_FAULT argument parses");

    expect(!otcs::parse_command("CMD SET_MODE").has_value(), "SET_MODE requires a mode argument");
    expect(!otcs::parse_command("CMD CLEAR_FAULT NONE").has_value(), "fault commands reject NONE as an argument");
    expect(!otcs::parse_command("CMD LAUNCH").has_value(), "unknown command is rejected");
    expect(!otcs::parse_command("CMD PING EXTRA").has_value(), "extra command arguments are rejected");
}

void test_acknowledgement()
{
    const otcs::Acknowledgement acknowledgement{otcs::CommandType::SetMode, otcs::CommandResult::Ok};

    expect(otcs::format_acknowledgement(acknowledgement) == "ACK SET_MODE OK",
           "acknowledgement formats to the expected text message");

    const auto parsed = otcs::parse_acknowledgement("ACK SET_MODE REJECTED");
    expect(parsed.has_value(), "valid acknowledgement parses");
    expect(parsed->command_type == otcs::CommandType::SetMode, "acknowledgement command type parses");
    expect(parsed->result == otcs::CommandResult::Rejected, "acknowledgement result parses");

    expect(!otcs::parse_acknowledgement("ACK SET_MODE MAYBE").has_value(),
           "acknowledgement rejects unknown results");
}

} // namespace

int main()
{
    test_format_telemetry();
    test_parse_telemetry();
    test_format_command();
    test_parse_command();
    test_acknowledgement();

    if (failures != 0) {
        std::cerr << failures << " protocol test expectation(s) failed.\n";
        return 1;
    }

    std::cout << "All protocol tests passed.\n";
    return 0;
}
