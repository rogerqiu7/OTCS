# OTCS - Orbital Telemetry & Command System

OTCS is a C++20 embedded command-and-control project that simulates a small spacecraft and its ground control station.

The system runs on real hardware: a Raspberry Pi Pico 2 W acts as the spacecraft flight computer, while a native C++ Ground Station runs on a Windows host. The Pico emits telemetry over USB serial, receives operator commands, updates spacecraft state, and returns acknowledgements. The Ground Station displays live status, sends commands, logs mission data, and detects stale telemetry links.

## Highlights

- Raspberry Pi Pico 2 W firmware with shared C++ flight-computer logic
- Native C++ Ground Station over USB serial
- Human-readable telemetry, command, and acknowledgement protocol
- Live terminal dashboard for spacecraft health
- Two-way command flow with validation and rejection
- Simulated fault injection and recovery
- Mission event logging and telemetry CSV output
- Link-health monitoring for stale telemetry
- Host-side tests for protocol, common types, and flight behavior

## Demo

The completed hardware-in-the-loop demo shows:

```text
Pico firmware runs independently
Pico sends OTCS telemetry over USB serial
Ground Station displays live spacecraft state
Operator sends commands from the Ground Station
Pico applies valid commands and returns ACK messages
Fault injection forces SAFE mode
Fault clearing returns the spacecraft to NORMAL
Ground Station records event and telemetry logs
```

Example command session:

```text
> PING
ACK PING OK

> INJECT_FAULT LOW_BATTERY
ACK INJECT_FAULT OK
TM ... MODE=SAFE ... FAULTS=1 ...

> SET_MODE NORMAL
ACK SET_MODE REJECTED

> CLEAR_FAULT LOW_BATTERY
ACK CLEAR_FAULT OK
TM ... MODE=NORMAL ... FAULTS=0 ...
```

For the full walkthrough, see [docs/DEMO.md](docs/DEMO.md).

## Architecture

```text
Ground Station
Windows C++ application

  - Opens USB serial port
  - Parses telemetry
  - Renders live dashboard
  - Sends operator commands
  - Logs mission events and telemetry
  - Detects stale links

        |
        | USB Serial
        |

Raspberry Pi Pico 2 W
Embedded C++ firmware

  - Runs shared FlightComputer logic
  - Maintains spacecraft mode and fault state
  - Emits telemetry
  - Receives commands
  - Sends ACK responses
```

Core logic is shared between host tests and firmware where practical, while platform-specific code stays isolated in the Ground Station and Pico wrapper.

## Tech Stack

- C++20
- CMake
- MSVC / Visual Studio build tools
- Raspberry Pi Pico SDK
- USB serial
- Python `pyserial` for serial inspection
- CTest for host-side verification

## Repository Layout

```text
common/                    Shared spacecraft types and utilities
flight_computer/           Platform-independent flight-computer logic
protocol/                  Telemetry, command, and ACK parsing/formatting
ground_station/            Native desktop Ground Station
firmware/pico_satellite_node/
                           Raspberry Pi Pico 2 W firmware project
tests/                     Host-side unit and integration tests
docs/                      Architecture, protocol, setup, firmware, and demo docs
logs/                      Runtime mission logs
```

## Build And Run

From a Visual Studio Developer PowerShell:

```powershell
cmake --preset windows-debug
cmake --build --preset build-windows-debug
```

Run the Ground Station:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Replace `COM3` with the serial port assigned to the Pico.

Run tests:

```powershell
ctest --test-dir build/windows-debug --output-on-failure
```

Current verification:

```text
100% tests passed, 0 tests failed out of 3
```

## Documentation

- [Demo Script](docs/DEMO.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Protocol](docs/PROTOCOL.md)
- [Ground Station](docs/GROUND_STATION.md)
- [Firmware](docs/FIRMWARE.md)
- [Setup](docs/SETUP.md)
- [Environment](docs/ENVIRONMENT.md)

## Project Summary

OTCS demonstrates embedded firmware development, C++ systems programming, serial communication, protocol design, state-machine behavior, command validation, fault handling, logging, and hardware-in-the-loop testing.

It is designed as a practical portfolio project for embedded systems, aerospace software, robotics, defense software, and C++ systems engineering roles.
