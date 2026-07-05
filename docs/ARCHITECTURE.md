# Architecture

## Purpose

This document describes the OTCS system architecture in implementation-level detail.

## Design Principles

OTCS targets `C++20`, but the implementation style should remain conservative and embedded-friendly.

Primary architecture goals:

* Clear separation of concerns
* Deterministic behavior
* Portable host-side code
* Testable components
* Explicit data flow between modules
* Protocol definitions isolated from transport and UI concerns
* Firmware logic isolated from desktop-specific behavior

## Module Boundaries

* `ground_station/`: desktop control, display, logging, command entry, transport integration
* `flight_computer/`: platform-independent spacecraft state machine, telemetry generation, command execution, fault handling
* `firmware/pico_satellite_node/`: Pico 2 W hardware wrapper, USB serial, timing, firmware image
* `protocol/`: shared command, acknowledgement, packet, and parsing/serialization rules
* `common/`: shared types, enums, utility helpers that are safe to share across targets
* `tests/`: host-side validation of parsing, state transitions, and protocol behavior

The current Ground Station serial reader and `main.cpp` control flow are
explained in [docs/GROUND_STATION.md](GROUND_STATION.md).

## Ground Station Boundary

The Ground Station does not create or run the spacecraft brain. It does not
instantiate `otcs::FlightComputer`.

Instead, it:

* opens the Pico's Windows COM port
* reads newline-terminated telemetry and acknowledgement messages
* parses each line with `otcs::parse_telemetry(...)`
  or `otcs::parse_acknowledgement(...)`
* displays decoded spacecraft status
* records raw mission events and parsed telemetry to disk
* tracks telemetry freshness and reports stale/recovered link state
* accepts operator commands from the terminal
* serializes commands with `otcs::format_command(...)`
* sends command lines back to the Pico
* ignores invalid telemetry or acknowledgement text without crashing

The Pico firmware owns the onboard state machine. The Ground Station is the
desktop monitoring and command side of the system.

## Source Versus Firmware Image

The `flight_computer/`, `protocol/`, and `common/` directories are source code
stored on the development computer. When the Pico firmware is built, the needed
parts are compiled into a `.uf2` firmware image and copied to the Pico.

After flashing, the Pico runs its own compiled copy from flash memory. It does
not call back to the PC for source files at runtime.

This mirrors the real embedded workflow: engineers develop and test software on
Earth, build a flight image, upload that image to the vehicle or device, and
then monitor telemetry from the running system.

## Firmware Boundary

Keep the firmware wrapper thin. Code in `firmware/pico_satellite_node/` should
handle Pico-specific responsibilities:

* initialize USB serial with `stdio_init_all()`
* provide timing with Pico SDK calls
* print telemetry over USB serial
* read command input from USB serial
* send acknowledgements over USB serial
* control future GPIO or onboard indicators

Code in `flight_computer/` should stay portable and testable:

* spacecraft mode transitions
* simulated telemetry state
* command validation and execution
* fault injection and recovery
* acknowledgement decisions

## Current Notes

The implementation keeps the architecture simple:

* Host-side simulator and Ground Station run on Windows first
* Pico firmware is running on physical Pico 2 W hardware
* Text telemetry, commands, and acknowledgements are the active wire protocol
* Shared code is kept small and intentionally scoped
* UI/presentation logic does not leak into protocol or state-machine code

## Current Milestone

The current architecture has a working two-way USB serial loop:

```text
Ground Station command input
        |
        v
CMD text line over COM port
        |
        v
Pico firmware command poller
        |
        v
shared protocol parser
        |
        v
FlightComputer::handle_command(...)
        |
        v
ACK text line and updated telemetry
```

This was verified with `PING`, `RESET`, manual mode changes, low-battery fault
injection, command rejection while faulted, and fault clearing back to NORMAL.

The Ground Station also owns mission logging:

```text
Ground Station RX/TX activity -> logs/events_001.log
Parsed telemetry snapshots -> logs/telemetry_001.csv
```

Logging stays on the desktop side. The Pico firmware remains focused on flight
state, telemetry, command handling, and ACK generation.

Link-health monitoring also stays on the Ground Station side:

```text
Valid TM telemetry updates last-telemetry time
No valid telemetry for > 3 seconds -> LINK_STALE NO_TELEMETRY_3S
Valid telemetry after stale state -> LINK_RECOVERED
```

Only telemetry resets the link-health timer. ACK messages are logged and
displayed, but they do not prove that the periodic telemetry stream is healthy.
