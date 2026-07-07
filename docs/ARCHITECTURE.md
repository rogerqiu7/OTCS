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

## Simple USB Serial Data Flow

The easiest way to think about the current OTCS connection is:

```text
Pico <---- USB serial ----> Ground Station on the PC
```

This is one connection with traffic moving in both directions.

On Windows, that connection appears as a COM port such as `COM3`.

## What The Pico Is Actually Polling

The Pico is not polling something literally named `COM3`.

`COM3` is only the Windows name for the USB serial connection on the PC side.

On the Pico side, the firmware initializes USB serial with `stdio_init_all()`
and then repeatedly checks whether any characters have arrived on its USB stdio
input stream.

In simple terms, the Pico keeps asking:

```text
Did the computer send me any characters yet?
```

If characters are available, the Pico reads them until it reaches a newline.
That full line becomes one command message.

So the command path is:

```text
Ground Station writes text to COM3
        |
        v
USB cable carries those bytes
        |
        v
Pico polls its USB serial input
        |
        v
Pico reads one full line such as CMD PING
```

## Where Pico Telemetry Goes

Every second, the Pico generates a telemetry snapshot and formats it into one
text line such as:

```text
TM SAT=1 TIME=2000 SEQ=2 MODE=NORMAL TEMP=22 BAT=99 FAULTS=0 UPTIME=2000
```

The firmware then sends that line with `printf(...)`.

In this project, `printf(...)` is not just printing to a local screen. Because
USB stdio is enabled, that text is sent out over the Pico's USB serial
connection.

So the telemetry path is:

```text
FlightComputer creates telemetry
        |
        v
protocol formats TM text
        |
        v
Pico printf(...) sends bytes over USB serial
        |
        v
Windows receives those bytes on COM3
        |
        v
Ground Station reads and parses the TM line
```

So yes, the Pico "prints" telemetry, but that print output is the real serial
transport that the Ground Station reads.

## What The Ground Station Reads

The Ground Station opens the Windows COM port that belongs to the Pico, for
example `COM3`.

It then repeatedly:

* checks whether bytes are waiting on that COM port
* reads them
* collects them until newline
* treats the result as one message
* parses the message as either telemetry (`TM ...`) or acknowledgement (`ACK ...`)

So in simple terms:

```text
Pico sends text
        |
        v
Windows exposes that text on COM3
        |
        v
Ground Station reads COM3
```

## Ground Station To Pico Example

If the operator types:

```text
PING
```

the Ground Station turns that into:

```text
CMD PING
```

and writes it to the COM port.

Then:

```text
Ground Station writes CMD PING to COM3
        |
        v
Pico sees incoming bytes during its poll loop
        |
        v
Pico reads the full line
        |
        v
shared protocol parses CMD PING
        |
        v
FlightComputer handles the command
        |
        v
Pico sends ACK PING OK back over USB serial
```

## Pico To Ground Station Example

Once per second, the Pico creates and sends telemetry:

```text
TM SAT=1 TIME=3000 SEQ=3 MODE=NORMAL TEMP=22 BAT=98 FAULTS=0 UPTIME=3000
```

Then:

```text
Pico sends TM line over USB serial
        |
        v
Windows makes that data available on COM3
        |
        v
Ground Station reads one full line from COM3
        |
        v
shared protocol parses the telemetry
        |
        v
dashboard and logs update
```

## Why Polling Matters

The Pico is not currently using interrupts or a separate communications task
for command input.

Instead, the firmware follows this simple pattern:

1. generate telemetry
2. send telemetry
3. spend about one second checking for command bytes
4. repeat

So "polling" just means the Pico keeps checking whether new command bytes have
shown up on the USB serial input.

## If OTCS Moves To Wi-Fi Later

If the transport changes to Wi-Fi, the protocol text can stay the same:

```text
TM ...
CMD ...
ACK ...
```

But the delivery path would change.

Current transport:

```text
USB serial
PC endpoint = COM3
```

Possible future transport:

```text
network socket
endpoint = IP address + port
```

That would mean:

* the Pico would send telemetry to a socket instead of USB serial
* the Ground Station would read from a socket instead of `COM3`
* the Ground Station would send commands back to the Pico over the network

So the language would stay the same, but the transport would change from serial
bytes on a COM port to network bytes on a socket.
