# OTCS — Orbital Telemetry & Command System

## Overview

OTCS (Orbital Telemetry & Command System) is a modern C++ and embedded systems project that simulates a spacecraft communicating with a ground control station. Implemented shared C++ flight-computer logic for both host tests and embedded firmware, including spacecraft modes, command validation, fault handling, and telemetry generation.

Unlike a simple software-only simulation, OTCS combines:

* Modern C++20
* Cross-platform desktop development
* Embedded firmware development
* Hardware communication
* USB serial communication
* Custom telemetry and command protocols
* Binary packet protocol design
* State machines
* Fault detection and recovery
* Telemetry processing
* Logging and diagnostics
* Mission simulation

The embedded "spacecraft" is implemented using a **Raspberry Pi Pico 2 W**, while the Ground Station runs as a native C++ application on a MacBook, Windows desktop, or Linux environment.

The project is inspired by the software architecture used in satellites, unmanned vehicles, robotics platforms, industrial control systems, and other mission-critical embedded systems.

The goal is not to build an actual satellite.

The goal is to build a realistic command-and-control platform that demonstrates the kinds of software engineering found in aerospace, defense, robotics, embedded systems, and industrial automation.

---

# Current Status

OTCS is a complete first-version embedded command-and-control system:

```text
Pico firmware runs by itself
Pico emits OTCS telemetry over USB serial
Ground Station opens the Windows COM port
Ground Station reads, parses, and displays telemetry in a live terminal dashboard
Ground Station sends text commands back to the Pico
Pico parses commands, applies them to FlightComputer, and returns ACK messages
Ground Station records mission events and parsed telemetry to log files
Ground Station warns if telemetry stops for more than 3 seconds
```

Verified live commands include:

```text
PING
RESET
SET_MODE SAFE
SET_MODE NORMAL
INJECT_FAULT LOW_BATTERY
CLEAR_FAULT LOW_BATTERY
```

The fault injection and recovery demo is working:

```text
INJECT_FAULT LOW_BATTERY
ACK INJECT_FAULT OK
TM ... MODE=SAFE ... FAULTS=1 ...

SET_MODE NORMAL
ACK SET_MODE REJECTED

CLEAR_FAULT LOW_BATTERY
ACK CLEAR_FAULT OK
TM ... MODE=NORMAL ... FAULTS=0 ...
```

The Ground Station also creates mission logs on startup:

```text
logs/events_001.log
logs/telemetry_001.csv
```

The event log records connection, RX, TX, ignored messages, and operator exit
events. The telemetry CSV records parsed telemetry rows for later review.
Link health changes such as `LINK_STALE NO_TELEMETRY_3S` and `LINK_RECOVERED`
are logged as mission events.

For the full demonstration path, see [docs/DEMO.md](docs/DEMO.md).

---

# Engineering Stance

OTCS targets **C++20** as the project language standard.

However, the project should use a **conservative embedded-friendly subset** of modern C++.

The intent is not to turn OTCS into a language feature showcase.

The intent is to demonstrate the engineering qualities hiring teams usually care about most:

* Clear architecture
* Good separation of concerns
* Deterministic behavior
* Portable code
* Explicit ownership and resource handling
* Testable components
* Readable state machines
* Defensive command and protocol handling
* Logging and diagnosability

In practice, this means:

* Prefer simple, explicit code over clever abstractions
* Use C++20 where it improves clarity or portability
* Avoid unnecessary template complexity
* Avoid depending on niche or flashy features unless they clearly help the design
* Keep host-side and embedded-side responsibilities separate
* Keep protocol definitions separate from UI or transport details

---

# Daily Workflow

Use these commands from the repo root during normal development.

## Host Build Prerequisites

OTCS currently builds host-side prototype executables for the Ground Station and
Flight Computer demo.

For macOS, install:

* Apple Command Line Tools
* Homebrew
* CMake
* Ninja
* Git

For Windows, install:

* Visual Studio with the Desktop development with C++ workload
* CMake
* Ninja
* Git

On Windows, run builds from a Developer PowerShell for Visual Studio, or
initialize the Visual Studio developer environment before configuring:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -HostArch amd64
cd C:\Users\roger\OneDrive\Documents\projects\OTCS
```

## First-time configure for a fresh build directory

Run this once when:

* creating a new build directory
* switching generators or compilers
* deleting the old build directory

macOS:

```bash
cmake --preset macos-debug
```

Windows:

```powershell
cmake --preset windows-debug
```

## Ground Station: rebuild and run after code changes

If you changed files under `ground_station/`, usually just run:

macOS:

```bash
cmake --build --preset build-macos-debug
./build/macos-debug/ground_station/otcs_ground_station
```

Windows:

```powershell
cmake --build --preset build-windows-debug
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Replace `COM3` with the Windows serial port assigned to the Pico. The Ground
Station opens the port at `115200` baud, reads OTCS telemetry lines, parses them
with the shared protocol code, and renders the latest spacecraft status in a
live terminal dashboard. It also accepts typed commands while telemetry is
streaming.

Dashboard sections:

```text
Connection: ONLINE or STALE
Spacecraft: SAT, mode, battery, temperature, faults, uptime, sequence
Activity: last ACK, last TX, last RX, latest event
Logs: event and telemetry CSV paths
Command prompt
```

On startup, it prints the log files for the current run:

```text
Event log: logs/events_001.log
Telemetry log: logs/telemetry_001.csv
```

If valid telemetry stops for more than 3 seconds after the first packet, the
Ground Station reports a stale link:

```text
WARNING: No telemetry received for 3 seconds. Connection is STALE.
```

When valid telemetry resumes, it reports recovery:

```text
Connection recovered.
```

Example command session:

```text
> ping
Last TX: CMD PING
Last ACK: PING OK

> inject_fault low_battery
Last TX: CMD INJECT_FAULT LOW_BATTERY
Last ACK: INJECT_FAULT OK
Mode: SAFE
Faults: 1
```

User input is normalized to uppercase before being sent, so `ping` and `PING`
both produce `CMD PING`.

For a detailed walkthrough of the Ground Station code, see
[docs/GROUND_STATION.md](docs/GROUND_STATION.md).

## Flight Computer host demo: rebuild and run after code changes

If you changed files under `flight_computer/`, run:

macOS:

```bash
cmake --build build/macos-debug --target otcs_flight_computer_host_demo
./build/macos-debug/flight_computer/otcs_flight_computer_host_demo
```

Windows:

```powershell
cmake --build build/windows-debug --target otcs_flight_computer_host_demo
.\build\windows-debug\flight_computer\otcs_flight_computer_host_demo.exe
```

## Tests

Tests are enabled in the Windows debug preset and can be run with:

```powershell
ctest --test-dir build/windows-debug --output-on-failure
```

The current test suite covers:

* shared spacecraft type conversion and fault flags
* text telemetry, command, and acknowledgement parsing/formatting
* FlightComputer mode transitions, fault injection, command rejection, recovery,
  and reset behavior

Recent Windows verification:

```text
100% tests passed, 0 tests failed out of 3
```

## When you do not need to reconfigure

You usually do not need to rerun the full `cmake -S . -B ...` command for normal `.cpp` or `.hpp` edits.

`cmake --build ...` is usually enough, and it may automatically rerun CMake if a `CMakeLists.txt` file changed.

---

# Project Goals

OTCS is a complete end-to-end embedded command-and-control system consisting of:

* An embedded Flight Computer running on a Raspberry Pi Pico 2 W
* A desktop Ground Station running on Windows
* A structured telemetry protocol
* A text command and acknowledgement protocol
* Fault injection and recovery
* A spacecraft state machine
* Mission logging
* Link-health monitoring
* A repeatable demo script

The project should demonstrate skills commonly expected from:

* Embedded Software Engineers
* C++ Engineers
* Robotics Software Engineers
* Aerospace Software Engineers
* Defense Systems Engineers
* Linux/System Software Engineers

The strongest hiring signal should come from code quality and system design, not just language version choice.

---

# Repository Structure

Repository structure:

```text
otcs/

README
    Main project overview and setup guide

docs/
    Additional documentation

docs/ARCHITECTURE.md
    Detailed software architecture

docs/PROTOCOL.md
    Telemetry and command packet definitions

docs/SETUP.md
    Platform-specific setup instructions

docs/FIRMWARE.md
    Pico firmware build, flash, and serial testing workflow

docs/GROUND_STATION.md
    Desktop Ground Station serial reader and main.cpp walkthrough

ground_station/
    Desktop C++ Ground Station application

flight_computer/
    Platform-independent spacecraft flight logic

protocol/
    Shared protocol definitions

common/
    Shared utilities and types

firmware/pico_satellite_node/
    Raspberry Pi Pico 2 W firmware wrapper and VS Code Pico project

tests/
    Unit tests and integration tests

tools/
    Helper tools and scripts

logs/
    Runtime logs

scripts/
    Build, flash, and utility scripts
```

---

# High Level Architecture

```text
                 Ground Station
          (macOS / Windows / Linux C++)

          Displays Telemetry
          Logs Events
          Sends Commands
          Detects Faults
          Runs Mission Scenarios

                   |
                   | USB Serial
                   |

        Raspberry Pi Pico 2 W
        Embedded Flight Computer

          Runs Flight Software
          Generates Telemetry
          Processes Commands
          Maintains Spacecraft State
          Handles Faults
          Controls Onboard LED / GPIO
````

OTCS uses **USB serial communication** between the computer and the Pico 2 W.

This is the simplest and most reliable way to begin because the same USB cable provides:

* Power
* Firmware upload
* Serial communication

Wi-Fi, TCP/UDP networking, and external sensors are intentionally outside this
first completed version.

---

# What OTCS Does

In simple terms, OTCS is a miniature spacecraft control system.

The **Raspberry Pi Pico 2 W** acts like a small satellite flight computer.

Your **computer** acts like mission control.

The Pico 2 W repeatedly sends health data to your computer.

The computer displays that data and can send commands back.

Example telemetry from the Pico:

```text
SAT_ID: 1
MODE: NORMAL
BATTERY: 95%
TEMP: 72F
FAULTS: NONE
UPTIME: 120s
```

Example command from the Ground Station:

```text
SET_MODE SAFE
```

The Pico receives the command, validates it, updates its internal state, and sends an acknowledgement.

Example acknowledgement:

```text
ACK: SET_MODE SAFE
RESULT: OK
```

This same pattern exists in many real systems:

* Mission Control ↔ Satellite
* Drone Controller ↔ Drone
* Robot Controller ↔ Robot
* Diagnostic Tool ↔ Vehicle ECU
* Factory Computer ↔ Industrial Machine

OTCS is a small, understandable version of that pattern.

---

# Project Philosophy

This project intentionally mirrors a simplified spacecraft.

Instead of writing random embedded code that only blinks LEDs, every component exists for a reason.

The Pico 2 W represents the onboard Flight Computer.

The desktop application represents the Ground Station.

The Ground Station monitors spacecraft health.

The Flight Computer periodically reports telemetry.

The Ground Station sends commands.

The Flight Computer executes those commands and reports back.

The system starts simple and becomes more realistic over time.

The first version may only print text over USB serial.

The completed version includes structured telemetry, command acknowledgements,
state-machine behavior, fault injection, recovery, logging, link-health
monitoring, and a repeatable demo flow.

The implementation preserves:

* Clear module boundaries
* Host and firmware separation
* Protocol stability
* Easy local testing
* Cross-platform buildability for host-side code

---

# Firmware Development Workflow

OTCS uses the official **Raspberry Pi Pico VS Code extension** for firmware
bring-up and flashing.

The active Pico firmware project is:

```text
firmware/pico_satellite_node/
```

The extension provides Pico-specific actions such as:

* New C/C++ Project
* Compile Project
* Run Project (USB)
* Switch Board
* Switch SDK

For early firmware work, open `firmware/pico_satellite_node/` in VS Code and
use:

```text
Compile Project
Run Project (USB)
```

`Run Project (USB)` automates the normal UF2 flashing loop:

1. Build the C++ firmware project.
2. Create a `.uf2` firmware image.
3. Detect the Pico in USB/BOOTSEL mode.
4. Copy the `.uf2` onto the Pico.
5. Let the Pico reboot and run the new firmware.

This mirrors the real embedded workflow at a small scale: engineers keep source
code and tests on development machines, compile a flight image, upload that
image to the vehicle or device, then monitor telemetry from the running system.

## Pico Project Settings

When creating or recreating the Pico firmware project, use:

* Board: `Pico 2 W`
* Architecture: `ARM`, not `RISC-V`
* SDK: `2.3.0`
* Console over USB: checked
* Console over UART: unchecked
* Wireless: none
* Generate C++ code: checked
* Run from flash: enabled

Use these project options:

* Run the program from RAM rather than flash: unchecked
* Use project name as entry point file name: checked
* Generate C++ code: checked
* Enable C++ RTTI: unchecked
* Enable C++ exceptions: unchecked
* Debugger: DebugProbe selected
* Enable CMake-Tools extension integration: unchecked

Leave optional hardware features unchecked until the project actually needs
them: SPI, I2C, UART, PIO, DMA, hardware interpolation, watchdog, timer, and
clocks.

`Console over USB` is the key setting for current testing. It sends
`printf(...)` output to the Windows COM port that Python miniterm reads.

## Firmware Serial Test

Install the Python serial dependency:

```powershell
python -m pip install -r requirements-dev.txt
```

Monitor the Pico:

```powershell
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the COM port Windows assigns to the Pico.

The current firmware prints OTCS text telemetry once per second:

```text
TM SAT=1 TIME=1000 SEQ=1 MODE=BOOT TEMP=22 BAT=100 FAULTS=0 UPTIME=1000
TM SAT=1 TIME=2000 SEQ=2 MODE=NORMAL TEMP=22 BAT=99 FAULTS=0 UPTIME=2000
```

It also polls USB serial for command lines such as:

```text
CMD PING
CMD SET_MODE SAFE
CMD INJECT_FAULT LOW_BATTERY
CMD CLEAR_FAULT LOW_BATTERY
```

For each valid command, the Pico returns an acknowledgement:

```text
ACK PING OK
ACK SET_MODE OK
ACK INJECT_FAULT OK
```

---

# Hardware Already Ordered

The ordered hardware is:

## Raspberry Pi Pico 2 W with Pre-Soldered Headers

This is the main embedded board for the project.

The Pico 2 W serves as the **Flight Computer**.

It runs the embedded firmware responsible for:

* Booting the spacecraft system
* Generating telemetry
* Receiving commands
* Updating spacecraft mode
* Managing fault states
* Communicating with the Ground Station
* Controlling onboard or external hardware

The ordered board includes:

* Raspberry Pi Pico 2 W
* RP2350 microcontroller
* Dual-core Arm Cortex-M33 processor
* 520 KB SRAM
* 4 MB flash
* Wi-Fi support
* Bluetooth support
* Pre-soldered headers
* USB programming support
* GPIO pins for future expansion
* UART, SPI, I2C, ADC, and PWM support

The pre-soldered headers are useful because they allow the Pico 2 W to be connected to a breadboard or jumper wires later without needing to solder.

For the early phases, no breadboard or external sensor is required.

---

# Required Hardware

For the first complete version of OTCS, the only required hardware is:

* Raspberry Pi Pico 2 W
* USB cable
* MacBook, Windows computer, or Linux machine

That is enough to build:

* Pico firmware
* Serial telemetry
* Ground Station communication
* Command processing
* State machine behavior
* Fault simulation
* Logging
* Mission simulation

No physical sensors are required at the beginning.

Telemetry values can be simulated in software.

---

# Optional Hardware Extensions

The following hardware is optional and outside the completed first version.

These are not required for the core OTCS project.

## Optional Breadboard

A breadboard allows external components to be connected without soldering.

Useful if we later add:

* LEDs
* Sensors
* Buttons
* Buzzers
* Displays

## Optional Jumper Wires

Jumper wires connect the Pico 2 W pins to a breadboard or external modules.

Because the ordered Pico 2 W has pre-soldered headers, jumper wires can be attached directly.

## Optional LEDs

External LEDs can show spacecraft status.

Example:

* Green LED = NORMAL mode
* Red LED = SAFE or FAULT mode

The Pico 2 W also has an onboard LED, so external LEDs are not required at first.

## Optional Resistors

220 ohm resistors are typically used with external LEDs.

These are only needed if external LEDs are added.

## Optional Sensors

Sensors can be added later to replace simulated telemetry with real readings.

Possible sensors:

* BME280

  * Temperature
  * Humidity
  * Pressure

* MPU6050

  * Accelerometer
  * Gyroscope

These would allow the Pico 2 W to send real environmental or motion data.

However, sensors are optional.

The project can still be strong and complete using simulated spacecraft telemetry.

## Optional Display

An OLED display could show onboard status directly from the Pico.

This is optional and not part of the initial scope.

## Optional Buzzer

A passive buzzer could indicate alarms or fault states.

This is optional and not part of the initial scope.

---

# Computer Requirements

Development will occur on both:

* macOS
* Windows

The Ground Station should compile and run on both systems.

The Pico firmware should also be buildable from either macOS or Windows.

A Linux environment may also be used, especially through:

* Native Linux
* WSL on Windows
* Docker
* CI/CD

---

# Development Tools

The project will use common C++ and embedded development tools.

## Required Tools

### Git

Used for version control.

### Python

Used for small developer tools around the C++ workflow. The preferred Pico USB
serial monitor is Python's `pyserial` miniterm:

```powershell
python -m pip install -r requirements-dev.txt
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the COM port Windows assigns to the Pico.

### CMake

Used to configure and generate builds.

### Ninja

Used as a fast build system.

### C++ Compiler

Depending on the platform:

* Clang on macOS
* MSVC or MinGW on Windows
* GCC or Clang on Linux

### VS Code

Recommended editor.

Useful extensions:

* C/C++
* Raspberry Pi Pico
* Cortex-Debug
* Serial Monitor

### Raspberry Pi Pico SDK

Official SDK for building firmware for the Pico 2 W.

The Pico SDK provides:

* Startup code
* Hardware APIs
* USB support
* GPIO support
* UART support
* CMake integration
* Examples

## C++ Standard

OTCS uses **C++20**.

That choice keeps the project current for portfolio and hiring purposes while still allowing the implementation style to remain conservative and portable.

Recommended usage style:

* Use straightforward modern C++ first
* Prefer value types and explicit ownership
* Keep memory behavior understandable
* Favor simple interfaces between modules
* Reach for newer language/library features only when they materially improve clarity

---

# Physical Setup

The initial physical setup is very simple.

```text
MacBook / Windows PC

        |

    USB Cable

        |

Raspberry Pi Pico 2 W
```

The USB cable does three jobs:

* Powers the Pico 2 W
* Allows firmware to be uploaded
* Provides USB serial communication

No breadboard is required in the first phases.

No sensors are required in the first phases.

No external LEDs are required in the first phases.

The Pico 2 W can generate simulated telemetry entirely in firmware.

---

# What Runs Where

## On the Pico 2 W

The Pico 2 W runs embedded C/C++ firmware.

This firmware acts as the spacecraft Flight Computer.

The firmware image contains compiled copies of the shared OTCS logic it links
against. At runtime, the Pico does not read source code from the development
computer; it runs the compiled firmware stored in its own flash memory.

Its responsibilities include:

* Boot sequence
* Maintaining spacecraft state
* Generating telemetry
* Sending telemetry over USB serial
* Receiving commands from the Ground Station
* Validating commands
* Sending acknowledgements
* Managing faults
* Entering SAFE mode when necessary
* Controlling onboard LED or future GPIO devices

Example firmware loop:

```cpp
while (true)
{
    update_spacecraft_state();
    generate_telemetry();
    send_telemetry_packet();
    receive_and_process_commands();
    sleep_until_next_cycle();
}
```

## On the Computer

The computer runs the Ground Station application.

This will be a native C++ application.

Its responsibilities include:

* Opening the serial connection
* Receiving telemetry from the Pico 2 W
* Decoding telemetry
* Displaying spacecraft status
* Logging telemetry to disk
* Sending commands
* Receiving acknowledgements
* Detecting communication timeouts
* Running mission scenarios

Example Ground Station behavior:

```text
OTCS Ground Station

Connection: ONLINE
Spacecraft: SAT-001
Mode: NORMAL
Battery: 95%
Temperature: 72F
Faults: NONE
Last Telemetry: 1 second ago
```

---

# Communication

The initial communication method is:

```text
Ground Station <-> USB Serial <-> Pico 2 W
```

At first, messages may be plain text because they are easier to debug.

Example telemetry:

```text
TM SAT=1 MODE=NORMAL TEMP=72 BAT=95 FAULTS=0
```

Example command:

```text
CMD SET_MODE SAFE
```

Example acknowledgement:

```text
ACK SET_MODE OK
```

Later, the text protocol will be replaced with a binary protocol.

---

# Text Protocol

The first protocol will be human-readable.

This makes it easier to debug early behavior.

Example telemetry message:

```text
TM SAT=1 TIME=12345 MODE=NORMAL TEMP=72 BAT=95 FAULTS=0
```

Example command message:

```text
CMD SET_MODE SAFE
```

Example acknowledgement message:

```text
ACK SET_MODE OK
```

The text protocol is temporary.

It allows us to prove that the system works before adding binary packet complexity.

---

# Binary Protocol Extension

The completed OTCS first version uses a human-readable text protocol. A binary
packet protocol is a natural extension point, but it is intentionally outside
the completed first version.

Binary packets are more realistic for embedded systems because they are:

* Smaller
* Faster to parse
* Less ambiguous
* Easier to validate
* Better suited for fixed packet formats
* Closer to real telemetry systems

Example packet structure:

```cpp
struct TelemetryPacket
{
    uint16_t sync;
    uint8_t version;
    uint8_t packet_type;
    uint32_t timestamp_ms;
    uint8_t spacecraft_id;
    uint8_t mode;
    uint8_t battery_percent;
    int16_t temperature_centi_c;
    uint16_t fault_flags;
    uint16_t crc;
};
```

Such a protocol could include:

* Sync bytes
* Version
* Packet type
* Payload length
* Sequence number
* Timestamp
* Payload
* CRC

---

# Flight Computer Responsibilities

The Flight Computer is the embedded system running on the Pico 2 W.

It is responsible for the onboard behavior of the simulated spacecraft.

Responsibilities:

* Start up cleanly
* Initialize communication
* Maintain spacecraft mode
* Generate telemetry
* Accept commands
* Reject invalid commands
* Send acknowledgements
* Detect simulated faults
* Enter SAFE mode when needed
* Recover from faults when commanded
* Report health status

The Flight Computer should not contain desktop UI code.

It should not know about terminal formatting, charts, or host-specific behavior.

It should behave like a small embedded device.

---

# Ground Station Responsibilities

The Ground Station is the desktop application running on the MacBook, Windows PC, or Linux environment.

It is responsible for monitoring and controlling the Flight Computer.

Responsibilities:

* Connect to the Pico 2 W
* Read telemetry messages
* Decode telemetry
* Display current spacecraft status
* Log telemetry
* Send commands
* Receive acknowledgements
* Detect missing telemetry
* Detect invalid packets
* Report faults
* Run mission scripts
* Save mission logs

The Ground Station acts like Mission Control.

The current Ground Station implementation is documented line by line in
[docs/GROUND_STATION.md](docs/GROUND_STATION.md).

## Shared Flight Logic

The `flight_computer/` directory is source code for the spacecraft brain. It is
kept platform-independent so it can be tested on Windows/macOS and later linked
into the Pico firmware.

The `firmware/pico_satellite_node/` directory is the Pico-specific wrapper. It
initializes the board, USB serial, timing, and future GPIO, then calls into
shared flight logic.

---

# Spacecraft States

The Flight Computer behaves as a state machine.

Core states:

```text
BOOT
NORMAL
SAFE
FAULT
```

## BOOT

Startup mode.

The system initializes hardware and software.

Possible boot actions:

* Initialize USB serial
* Initialize timers
* Initialize telemetry counters
* Initialize simulated sensors
* Set default spacecraft mode
* Send boot message

## NORMAL

Standard operating mode.

In NORMAL mode:

* Telemetry is sent at the normal rate
* Faults are monitored
* Commands are accepted
* The spacecraft is considered healthy

Example:

```text
Mode: NORMAL
Telemetry Rate: 1 Hz
Faults: NONE
```

## SAFE

Reduced-functionality mode.

The spacecraft enters SAFE mode when something abnormal happens.

In SAFE mode:

* Telemetry may be sent at a different rate
* Certain commands may be restricted
* Fault state is clearly reported
* The system waits for recovery commands

Example:

```text
Mode: SAFE
Reason: LOW_BATTERY
Telemetry Rate: 0.2 Hz
```

## FAULT

Critical fault mode.

The spacecraft enters FAULT mode when the simulated issue is severe.

In FAULT mode:

* The system reports the fault
* Some commands may be rejected
* Recovery may require RESET_FAULT or RESET

Example:

```text
Mode: FAULT
Fault: COMMS_TIMEOUT
Recovery Required: RESET_FAULT
```

## Optional Additional States

Additional states can be added without changing the current demo:

* SCIENCE
* CALIBRATION
* LOW_POWER
* SHUTDOWN

These are optional extensions, not required by the completed OTCS demo.

---

# Commands

The Ground Station can send commands to the Flight Computer.

Initial commands:

```text
PING
RESET
SET_MODE NORMAL
SET_MODE SAFE
RESET_FAULT
SET_TELEMETRY_RATE
INJECT_FAULT
CLEAR_FAULT
REQUEST_STATUS
```

## PING

Checks whether the Flight Computer is alive.

Expected response:

```text
ACK PING OK
```

## RESET

Resets the Flight Computer state.

Expected behavior:

```text
BOOT -> NORMAL
```

## SET_MODE NORMAL

Requests normal operation.

May be rejected if a critical fault is active.

## SET_MODE SAFE

Requests SAFE mode.

Useful during simulated failures.

## RESET_FAULT

Clears recoverable faults.

## SET_TELEMETRY_RATE

Changes how often telemetry is sent.

Example:

```text
CMD SET_TELEMETRY_RATE 1000
```

This may mean telemetry every 1000 milliseconds.

## INJECT_FAULT

Creates a simulated fault for testing.

Example:

```text
CMD INJECT_FAULT LOW_BATTERY
```

## CLEAR_FAULT

Clears a simulated fault.

## REQUEST_STATUS

Requests an immediate telemetry update.

---

# Telemetry

Telemetry represents spacecraft health and status.

Initial telemetry fields may include:

* Spacecraft ID
* Timestamp
* Sequence number
* Mode
* Battery percentage
* Temperature
* Uptime
* Fault flags
* Last command status
* Telemetry rate
* Communication status

Initially, telemetry values can be simulated.

Example simulated values:

```text
Battery: starts at 100% and slowly decreases
Temperature: starts at 72F and changes slightly over time
Mode: changes based on command or fault
Fault flags: set by injected faults
Uptime: time since boot
```

Later, optional sensors may replace some simulated values.

For example:

* Simulated temperature can be replaced with BME280 temperature
* Simulated motion can be replaced with MPU6050 acceleration

But real sensors are optional.

---

# Fault Injection

OTCS intentionally supports simulated faults.

This is one of the most important parts of the project because it shows how the system behaves under abnormal conditions.

Possible faults:

* LOW_BATTERY
* HIGH_TEMPERATURE
* SENSOR_OFFLINE
* COMMS_TIMEOUT
* INVALID_COMMAND
* PACKET_CRC_ERROR
* WATCHDOG_RESET
* UNKNOWN_MODE

Example:

```text
CMD INJECT_FAULT HIGH_TEMPERATURE
```

The Flight Computer may respond by entering SAFE mode:

```text
ACK INJECT_FAULT OK
TM MODE=SAFE FAULTS=HIGH_TEMPERATURE
```

The Ground Station should display a warning.

Example:

```text
WARNING: HIGH_TEMPERATURE fault detected.
Spacecraft entered SAFE mode.
```

Fault injection allows the project to demonstrate:

* Defensive programming
* Recovery behavior
* State transitions
* Mission operations
* Logging
* Diagnostics

---

# Logging

The Ground Station logs important system activity.

Logs may include:

* Incoming telemetry
* Sent commands
* Command acknowledgements
* Fault events
* Mode transitions
* Packet errors
* Connection events
* Mission scenario events

Example log:

```text
[12:00:01] CONNECTED Pico 2 W on COM5
[12:00:02] TM SAT=1 MODE=NORMAL BAT=99 TEMP=72 FAULTS=0
[12:00:10] CMD SET_MODE SAFE
[12:00:10] ACK SET_MODE OK
[12:00:11] TM SAT=1 MODE=SAFE BAT=99 TEMP=72 FAULTS=0
```

Logs help debug the system and make the project feel more professional.

Current implementation:

```text
logs/events_001.log
logs/telemetry_001.csv
```

Example event log:

```text
[15:09:05] CONNECTED COM3 115200
[15:09:06] RX TM SAT=1 TIME=205000 SEQ=205 MODE=NORMAL TEMP=22 BAT=0 FAULTS=0 UPTIME=205000
[15:09:20] TX CMD PING
[15:09:20] RX ACK PING OK
[15:09:35] TX CMD INJECT_FAULT LOW_BATTERY
[15:09:35] RX ACK INJECT_FAULT OK
[15:10:01] LINK_STALE NO_TELEMETRY_3S
[15:10:05] LINK_RECOVERED
```

Example telemetry CSV:

```text
host_time,timestamp_ms,sequence,spacecraft_id,mode,battery_percent,temperature_c,fault_flags,uptime_ms
15:09:06,205000,205,1,NORMAL,0,22,0,205000
15:09:36,220000,220,1,SAFE,0,22,1,220000
```

---

# Completed Capabilities

OTCS includes:

* Pico 2 W firmware running the shared Flight Computer logic
* Structured text telemetry over USB serial
* A native C++ Ground Station for Windows host testing
* A live terminal dashboard
* Two-way command and acknowledgement traffic
* Command validation and rejection
* Spacecraft modes: `BOOT`, `NORMAL`, `SAFE`, and `FAULT`
* Simulated fault injection and recovery
* Mission event logs
* Parsed telemetry CSV logs
* Telemetry link-health detection
* Host-side tests for common types, protocol parsing, and flight behavior

The completed demo demonstrates:

* Modern C++
* Embedded firmware
* Cross-platform desktop development
* USB serial communication
* Command handling
* Telemetry processing
* Binary protocol design
* State machines
* Fault injection
* Fault handling
* Logging
* Diagnostics
* Mission simulation
* Hardware-in-the-loop behavior over USB serial

The result is a simplified spacecraft command-and-control platform that is easy
to explain during technical interviews and provides a strong portfolio piece for
embedded systems, aerospace, robotics, defense, and systems software engineering
roles.

---

# Final Project Description

OTCS is a spacecraft-inspired embedded command-and-control system built with modern C++ and a Raspberry Pi Pico 2 W.

The system consists of a desktop Ground Station and an embedded Flight Computer. The Flight Computer generates telemetry, maintains spacecraft state, receives commands, reports faults, and simulates mission behavior. The Ground Station receives telemetry, displays spacecraft health, sends commands, logs events, and detects abnormal behavior.

The project demonstrates embedded firmware development, C++ systems programming, USB serial communication, custom protocol design, binary packet serialization, state machines, fault detection, and mission simulation.

OTCS is designed as a practical portfolio project for embedded systems, aerospace software, defense software, robotics, and C++ systems engineering roles.
