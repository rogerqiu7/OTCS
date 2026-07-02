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
* `flight_computer/`: embedded state machine, telemetry generation, command execution, fault handling
* `protocol/`: shared command, acknowledgement, packet, and parsing/serialization rules
* `common/`: shared types, enums, utility helpers that are safe to share across targets
* `tests/`: host-side validation of parsing, state transitions, and protocol behavior

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
