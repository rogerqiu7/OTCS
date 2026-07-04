#include <iostream>
#include <string_view>

#include "flight_computer.hpp"

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view message)
{
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

void test_boot_then_normal()
{
    otcs::FlightComputer flight_computer;

    const auto boot = flight_computer.tick(0);
    expect(boot.mode == otcs::SpacecraftMode::Boot, "first telemetry reports BOOT");
    expect(boot.sequence == 1, "first telemetry has sequence 1");

    const auto normal = flight_computer.tick(1000);
    expect(normal.mode == otcs::SpacecraftMode::Normal, "second telemetry reports NORMAL");
    expect(normal.timestamp_ms == 1000, "second telemetry advances timestamp");
    expect(normal.uptime_ms == 1000, "second telemetry advances uptime");
    expect(normal.sequence == 2, "second telemetry increments sequence");
}

void test_set_mode_command()
{
    otcs::FlightComputer flight_computer;
    flight_computer.tick(0);
    flight_computer.tick(1000);

    otcs::Command command{otcs::CommandType::SetMode};
    command.requested_mode = otcs::SpacecraftMode::Safe;

    const auto acknowledgement = flight_computer.handle_command(command);
    expect(acknowledgement.result == otcs::CommandResult::Ok, "SET_MODE SAFE is accepted");
    expect(flight_computer.telemetry().mode == otcs::SpacecraftMode::Safe, "SET_MODE SAFE changes mode");
}

void test_fault_injection_enters_safe()
{
    otcs::FlightComputer flight_computer;
    flight_computer.tick(0);
    flight_computer.tick(1000);

    otcs::Command command{otcs::CommandType::InjectFault};
    command.requested_fault = otcs::Fault::LowBattery;

    const auto acknowledgement = flight_computer.handle_command(command);
    const auto snapshot = flight_computer.telemetry();

    expect(acknowledgement.result == otcs::CommandResult::Ok, "INJECT_FAULT LOW_BATTERY is accepted");
    expect(snapshot.mode == otcs::SpacecraftMode::Safe, "fault injection moves spacecraft to SAFE");
    expect(otcs::has_fault(snapshot.fault_flags, otcs::Fault::LowBattery), "fault flag is set");
}

void test_fault_blocks_normal_mode()
{
    otcs::FlightComputer flight_computer;
    flight_computer.tick(0);
    flight_computer.tick(1000);

    otcs::Command inject{otcs::CommandType::InjectFault};
    inject.requested_fault = otcs::Fault::LowBattery;
    const auto inject_acknowledgement = flight_computer.handle_command(inject);
    expect(inject_acknowledgement.result == otcs::CommandResult::Ok, "fault setup command is accepted");

    otcs::Command normal{otcs::CommandType::SetMode};
    normal.requested_mode = otcs::SpacecraftMode::Normal;

    const auto acknowledgement = flight_computer.handle_command(normal);
    expect(acknowledgement.result == otcs::CommandResult::Rejected,
           "SET_MODE NORMAL is rejected while a fault is active");
}

void test_clear_fault_recovers_to_normal()
{
    otcs::FlightComputer flight_computer;
    flight_computer.tick(0);
    flight_computer.tick(1000);

    otcs::Command inject{otcs::CommandType::InjectFault};
    inject.requested_fault = otcs::Fault::LowBattery;
    const auto inject_acknowledgement = flight_computer.handle_command(inject);
    expect(inject_acknowledgement.result == otcs::CommandResult::Ok, "fault setup command is accepted");

    otcs::Command clear{otcs::CommandType::ClearFault};
    clear.requested_fault = otcs::Fault::LowBattery;
    const auto acknowledgement = flight_computer.handle_command(clear);
    const auto snapshot = flight_computer.telemetry();

    expect(acknowledgement.result == otcs::CommandResult::Ok, "CLEAR_FAULT LOW_BATTERY is accepted");
    expect(snapshot.fault_flags == 0, "CLEAR_FAULT removes the fault");
    expect(snapshot.mode == otcs::SpacecraftMode::Normal, "clearing the last fault returns to NORMAL");
}

void test_reset_restores_boot_state()
{
    otcs::FlightComputer flight_computer;
    flight_computer.tick(0);
    flight_computer.tick(1000);

    const auto acknowledgement = flight_computer.handle_command(otcs::Command{otcs::CommandType::Reset});
    const auto snapshot = flight_computer.telemetry();

    expect(acknowledgement.result == otcs::CommandResult::Ok, "RESET is accepted");
    expect(snapshot.mode == otcs::SpacecraftMode::Boot, "RESET returns mode to BOOT");
    expect(snapshot.sequence == 0, "RESET clears sequence count");
    expect(snapshot.uptime_ms == 0, "RESET clears uptime");
    expect(snapshot.battery_percent == 100, "RESET restores battery");
}

} // namespace

int main()
{
    test_boot_then_normal();
    test_set_mode_command();
    test_fault_injection_enters_safe();
    test_fault_blocks_normal_mode();
    test_clear_fault_recovers_to_normal();
    test_reset_restores_boot_state();

    if (failures != 0) {
        std::cerr << failures << " flight computer test expectation(s) failed.\n";
        return 1;
    }

    std::cout << "All flight computer tests passed.\n";
    return 0;
}
