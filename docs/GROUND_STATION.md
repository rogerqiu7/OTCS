# Ground Station Walkthrough

## Purpose

The Ground Station is the desktop side of OTCS. It acts like early Mission
Control: it opens the Pico's USB serial port, receives telemetry lines, parses
them with the shared OTCS text protocol, prints decoded spacecraft status, and
sends operator commands back to the Pico.

The main entry point is:

```text
ground_station/main.cpp
```

Run it on Windows with:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Replace `COM3` with the serial port assigned to the Pico. Close Python miniterm
first because Windows allows only one program to own the serial port at a time.

## Current Two-Way Behavior

The Ground Station now supports the live command-and-control loop:

```text
Pico -> TM telemetry -> Ground Station
Ground Station -> CMD command -> Pico
Pico -> ACK acknowledgement -> Ground Station
```

Supported operator inputs include:

```text
PING
RESET
SET_MODE SAFE
SET_MODE NORMAL
INJECT_FAULT LOW_BATTERY
CLEAR_FAULT LOW_BATTERY
```

The Ground Station normalizes typed input to uppercase and adds the `CMD`
prefix before sending. For example:

```text
> ping
TX: CMD PING
RX: ACK PING OK
```

The verified fault demo is:

```text
> INJECT_FAULT LOW_BATTERY
TX: CMD INJECT_FAULT LOW_BATTERY
RX: ACK INJECT_FAULT OK
RX: TM ... MODE=SAFE ... FAULTS=1 ...

> SET_MODE NORMAL
TX: CMD SET_MODE NORMAL
RX: ACK SET_MODE REJECTED

> CLEAR_FAULT LOW_BATTERY
TX: CMD CLEAR_FAULT LOW_BATTERY
RX: ACK CLEAR_FAULT OK
RX: TM ... MODE=NORMAL ... FAULTS=0 ...
```

## Important Split

The Ground Station does not create or run the spacecraft brain.

This line exists in the Pico firmware, not the Ground Station:

```cpp
otcs::FlightComputer flight_computer;
```

That code lives in:

```text
firmware/pico_satellite_node/main.cpp
```

The Pico runs the flight computer logic, sends telemetry, receives commands,
and returns acknowledgements. The Ground Station displays telemetry and acts as
the operator command terminal.

## Includes

Ground Station `main.cpp` starts with standard C++ headers:

```cpp
#include <exception>
#include <iostream>
#include <string>
```

`<exception>` lets the program catch errors such as "COM3 could not be opened".
`<iostream>` provides `std::cout` and `std::cerr` for terminal output.
`<string>` provides `std::string`, which stores serial port names and received
telemetry lines.

Then it includes OTCS headers:

```cpp
#include "serial_port.hpp"
#include "status_banner.hpp"
#include "text_protocol.hpp"
```

`serial_port.hpp` declares `otcs::SerialPort`, the Windows COM-port wrapper.
`status_banner.hpp` declares `print_status_banner()`.
`text_protocol.hpp` declares `otcs::parse_telemetry(...)` and the shared
telemetry model types.

## File-Private Helpers

The helper functions are wrapped in an unnamed namespace:

```cpp
namespace {
```

That means the helpers are private to `main.cpp`. Other files cannot
accidentally call them or collide with their names.

### print_usage

```cpp
void print_usage(const char* executable_name)
```

This function prints how to run the Ground Station when the user forgets the
serial port argument.

Example output:

```text
Usage: otcs_ground_station.exe <serial-port>
Example: otcs_ground_station.exe COM3
```

### print_telemetry_status

```cpp
void print_telemetry_status(const otcs::TelemetrySnapshot& telemetry)
```

This function receives a parsed telemetry object and prints a readable status
summary.

`otcs::TelemetrySnapshot` is defined in:

```text
common/spacecraft_types.hpp
```

It contains fields such as `spacecraft_id`, `timestamp_ms`, `sequence`, `mode`,
`battery_percent`, `temperature_c`, `fault_flags`, and `uptime_ms`.

The `&` means the telemetry object is passed by reference, avoiding a copy. The
`const` means this function promises not to modify it.

This line prints the spacecraft ID:

```cpp
std::cout << "SAT-" << static_cast<unsigned int>(telemetry.spacecraft_id) << '\n'
```

`spacecraft_id` is stored as an 8-bit integer. Casting it to `unsigned int`
makes it print as `1` instead of being treated like a character.

This line prints the spacecraft mode:

```cpp
<< "  Mode:    " << otcs::to_string(telemetry.mode) << '\n'
```

`otcs::to_string(...)` is defined in:

```text
common/spacecraft_types.cpp
```

It turns enum values such as `SpacecraftMode::Normal` into text such as
`NORMAL`.

The remaining lines print battery, temperature, fault flags, uptime, and
telemetry sequence number.

## Program Entry

The program starts here:

```cpp
int main(int argc, char* argv[])
```

`argc` is the argument count. `argv` is the argument list.

If you run:

```powershell
otcs_ground_station.exe COM3
```

then `argc` is `2`, `argv[0]` is the executable path, and `argv[1]` is `COM3`.

The first check is:

```cpp
if (argc != 2) {
    print_usage(argv[0]);
    return 1;
}
```

If the user does not provide exactly one serial port argument, the app prints
usage and exits with error code `1`.

Then the serial port name is stored:

```cpp
const std::string port_name = argv[1];
```

For example, this stores `COM3`.

## Opening The Serial Link

The main work is inside a `try` block:

```cpp
try {
```

If opening or reading the serial port fails, the program catches the error at
the bottom and prints a useful message.

First it prints the banner:

```cpp
print_status_banner();
```

That function is defined in:

```text
ground_station/status_banner.cpp
```

It currently prints:

```text
OTCS Ground Station
```

Then it opens the serial port:

```cpp
otcs::SerialPort serial_port{port_name, 115200};
```

`otcs::SerialPort` is declared in `ground_station/serial_port.hpp` and
implemented in `ground_station/serial_port.cpp`.

`port_name` is something like `COM3`. `115200` is the baud rate. It matches the
USB serial rate used by the Pico testing workflow.

Internally, on Windows, `SerialPort` uses APIs such as `CreateFileA`,
`GetCommState`, `SetCommState`, `SetCommTimeouts`, and `ReadFile`.

Those APIs open and configure the COM port as `115200 baud`, `8 data bits`, `no
parity`, and `1 stop bit`.

## Reading Telemetry Forever

After connecting, the Ground Station enters an infinite loop:

```cpp
while (true) {
```

This loop continues until the user stops it with `EXIT`, `QUIT`, or `Ctrl+C`.

Each pass polls the serial port for a full line from the Pico:

```cpp
serial_port.try_read_line(line)
```

`try_read_line(...)` returns immediately when no bytes are waiting. On Windows,
the implementation checks the COM port receive queue before calling `ReadFile`.
That keeps the Ground Station responsive to keyboard input while telemetry is
streaming.

The Pico sends lines like:

```text
TM SAT=1 TIME=3000 SEQ=3 MODE=NORMAL TEMP=22 BAT=98 FAULTS=0 UPTIME=3000
```

The Ground Station stores each complete received line in `line`.

Blank lines are skipped:

```cpp
if (line.empty()) {
    continue;
}
```

Then the raw received line is printed:

```cpp
std::cout << "RX: " << line << '\n';
```

`RX` means received.

## Sending Commands

The Ground Station also polls the keyboard for typed commands. When the user
presses Enter, it parses the command with the shared protocol code and sends the
canonical command line over serial.

For example, this user input:

```text
inject_fault low_battery
```

is sent as:

```text
CMD INJECT_FAULT LOW_BATTERY
```

The terminal prints:

```text
TX: CMD INJECT_FAULT LOW_BATTERY
```

`TX` means transmitted.

## Parsing The Protocol

The key protocol step is:

```cpp
const auto acknowledgement = otcs::parse_acknowledgement(line);
const auto telemetry = otcs::parse_telemetry(line);
```

`otcs::parse_acknowledgement(...)` and `otcs::parse_telemetry(...)` are defined in:

```text
protocol/text_protocol.cpp
```

The Ground Station first checks whether a received line is an `ACK` message. If
not, it checks whether the line is telemetry. Valid telemetry returns a
`TelemetrySnapshot`. Invalid text is ignored safely.

That is why the next check is:

```cpp
if (telemetry.has_value()) {
    print_telemetry_status(*telemetry);
}
```

`has_value()` means parsing succeeded. `*telemetry` means "give me the actual
`TelemetrySnapshot` stored inside the optional".

If parsing fails, the line is ignored safely:

```cpp
else {
    std::cout << "Ignored: message is not valid OTCS telemetry or acknowledgement.\n\n";
}
```

The Ground Station does not crash on unexpected text. It reports the problem and
keeps listening.

## Error Handling

Errors are caught here:

```cpp
} catch (const std::exception& error) {
    std::cerr << "Ground Station error: " << error.what() << '\n';
    return 1;
}
```

If the serial port cannot open, or a serial read fails, the program prints a
clear error and exits with code `1`.

## Big Picture

The Ground Station flow is:

```text
read COM port name
open serial port
loop forever:
    poll serial for one line from Pico
    if a line arrived:
        print raw RX line
        parse line as OTCS acknowledgement or telemetry
    if telemetry is valid:
        print decoded spacecraft status
    if acknowledgement is valid:
        print command result
    poll keyboard for user command
    if a command was typed:
        normalize and validate command
        send CMD line to Pico
    if input or received text is invalid:
        ignore it and keep listening
```

The Pico is running the spacecraft logic. The Ground Station is Mission
Control's ears, display, and command console.
