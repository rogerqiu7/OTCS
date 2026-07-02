#include <iostream>
#include <string_view>

#include "spacecraft_types.hpp"

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view message)
{
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

void test_mode_conversion()
{
    // Modes are serialized as stable protocol-facing names, not C++ enum names.
    expect(otcs::to_string(otcs::SpacecraftMode::Boot) == "BOOT", "BOOT mode formats");
    expect(otcs::to_string(otcs::SpacecraftMode::Normal) == "NORMAL", "NORMAL mode formats");
    expect(otcs::parse_spacecraft_mode("SAFE") == otcs::SpacecraftMode::Safe, "SAFE mode parses");
    expect(!otcs::parse_spacecraft_mode("SCIENCE").has_value(), "unknown mode is rejected");
}

void test_fault_conversion()
{
    expect(otcs::to_string(otcs::Fault::LowBattery) == "LOW_BATTERY", "LOW_BATTERY fault formats");
    expect(otcs::parse_fault("HIGH_TEMPERATURE") == otcs::Fault::HighTemperature,
           "HIGH_TEMPERATURE fault parses");
    expect(!otcs::parse_fault("BATTERY_IS_A_MYSTERY").has_value(), "unknown fault is rejected");
}

void test_fault_flags()
{
    // Fault flags let telemetry report multiple active faults in one integer.
    otcs::FaultFlags flags = 0;

    flags = otcs::add_fault(flags, otcs::Fault::LowBattery);
    flags = otcs::add_fault(flags, otcs::Fault::CommsTimeout);

    expect(otcs::has_fault(flags, otcs::Fault::LowBattery), "LOW_BATTERY flag is set");
    expect(otcs::has_fault(flags, otcs::Fault::CommsTimeout), "COMMS_TIMEOUT flag is set");
    expect(!otcs::has_fault(flags, otcs::Fault::SensorOffline), "SENSOR_OFFLINE flag is not set");

    flags = otcs::clear_fault(flags, otcs::Fault::LowBattery);

    expect(!otcs::has_fault(flags, otcs::Fault::LowBattery), "LOW_BATTERY flag is cleared");
    expect(otcs::has_fault(flags, otcs::Fault::CommsTimeout), "other flags remain set");
}

void test_command_conversion()
{
    expect(otcs::to_string(otcs::CommandType::SetMode) == "SET_MODE", "SET_MODE command formats");
    expect(otcs::parse_command_type("REQUEST_STATUS") == otcs::CommandType::RequestStatus,
           "REQUEST_STATUS command parses");
    expect(!otcs::parse_command_type("LAUNCH").has_value(), "unknown command is rejected");

    expect(otcs::to_string(otcs::CommandResult::Ok) == "OK", "OK result formats");
    expect(otcs::parse_command_result("REJECTED") == otcs::CommandResult::Rejected,
           "REJECTED result parses");
    expect(!otcs::parse_command_result("MAYBE").has_value(), "unknown result is rejected");
}

void test_default_telemetry_snapshot()
{
    // Defaults describe a freshly booted simulated spacecraft.
    const otcs::TelemetrySnapshot snapshot;

    expect(snapshot.spacecraft_id == 1, "default spacecraft id is SAT-001");
    expect(snapshot.mode == otcs::SpacecraftMode::Boot, "default mode is BOOT");
    expect(snapshot.battery_percent == 100, "default battery starts full");
    expect(snapshot.fault_flags == 0, "default telemetry has no faults");
}

} // namespace

int main()
{
    test_mode_conversion();
    test_fault_conversion();
    test_fault_flags();
    test_command_conversion();
    test_default_telemetry_snapshot();

    if (failures != 0) {
        std::cerr << failures << " common test expectation(s) failed.\n";
        return 1;
    }

    std::cout << "All common tests passed.\n";
    return 0;
}
