# Architecture

## Purpose

This document will describe the OTCS system architecture in implementation-level detail.

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

## Planned Module Boundaries

* `ground_station/`: desktop control, display, logging, command entry, transport integration
* `flight_computer/`: platform-independent spacecraft state machine, telemetry generation, command execution, fault handling
* `firmware/pico_satellite_node/`: Pico 2 W hardware wrapper, USB serial, timing, future GPIO, firmware image
* `protocol/`: shared command, acknowledgement, packet, and parsing/serialization rules
* `common/`: shared types, enums, utility helpers that are safe to share across targets
* `tests/`: host-side validation of parsing, state transitions, and protocol behavior

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
* read future command input from USB serial
* control future GPIO or onboard indicators

Code in `flight_computer/` should stay portable and testable:

* spacecraft mode transitions
* simulated telemetry state
* command validation and execution
* fault injection and recovery
* acknowledgement decisions

## Planned Sections

* Repository layout
* Process boundaries
* Ground Station responsibilities
* Flight Computer responsibilities
* Shared protocol layer
* State machine design
* Fault handling model
* Logging design
* Test strategy

## Placeholder Notes

Initial implementation should keep the architecture simple:

* Host-side simulator and Ground Station run on macOS first
* Pico firmware is added once hardware arrives
* Text protocol is implemented before binary packets
* Shared code is kept small and intentionally scoped
* UI/presentation logic should not leak into protocol or state-machine code
