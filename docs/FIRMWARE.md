# Firmware Workflow

## Purpose

The Pico firmware is the software package that runs on the physical Raspberry Pi
Pico 2 W. In OTCS, the Pico represents the onboard spacecraft computer.

The preferred firmware project is:

```text
firmware/pico_satellite_node/
```

This project was created with the Raspberry Pi Pico VS Code extension and has
already been verified on hardware by printing over USB serial.

## Mental Model

The repository is the factory.

The firmware build produces a `.uf2` package.

Flashing copies that package onto the Pico.

After flashing, the Pico runs its own compiled copy of the flight software. It
does not call back into the computer for `flight_computer/` code at runtime.

In real spacecraft and embedded systems, engineers usually keep the mission
logic in source control on Earth, test as much as possible on normal computers,
compile a flight image, and upload that image to the vehicle or device. OTCS
simulates that pattern at a small scale:

```text
source code on PC
        |
        v
compiled UF2 firmware image
        |
        v
Pico 2 W flash memory
        |
        v
USB serial telemetry back to Ground Station
```

## Why This Is Embedded Engineering

This firmware workflow is a small version of a real embedded-software workflow.

The important embedded ideas are:

* the software runs on a separate physical target, not on the engineer's laptop
* the software is cross-compiled into a target-specific image
* the image is flashed into non-volatile memory on the device
* the device interacts with hardware and timing directly
* engineers usually observe behavior indirectly through telemetry, logs, and test interfaces
* the source code on the development computer is not what runs at runtime; the compiled image is

That is the core difference between a normal desktop application and embedded
firmware.

For OTCS, the laptop is the development environment and Ground Station. The
Pico is the target. The firmware image is the deployed onboard software.

## How This Maps To Real Aerospace And Defense Work

OTCS is a teaching-scale version of the same basic loop used in larger embedded
programs.

At places like NASA, Lockheed Martin, or similar aerospace and defense teams,
the exact tools, review rules, simulation stack, and certification burden are
much more rigorous, but the development shape is still recognizable:

```text
requirements
    ->
design
    ->
implementation
    ->
host-side test
    ->
target build
    ->
integration test
    ->
hardware test
    ->
operations with telemetry and commanding
```

OTCS models that flow in simplified form.

### Typical onboard software split

Real flight or vehicle software is usually split into layers such as:

* mission logic or application logic
* flight-software services or middleware
* drivers and hardware interfaces
* communications interfaces
* startup and boot code

OTCS mirrors that split in a simple way:

* `flight_computer/`: mission logic
* `protocol/`: communications message format
* `firmware/pico_satellite_node/`: target-specific hardware wrapper
* `ground_station/`: operator-side tooling

### Typical development process

A common pattern in aerospace-style embedded work looks like this:

1. Engineers define expected behavior.
2. They implement logic in a testable form on a normal development machine.
3. They compile that logic for the target processor.
4. They load the resulting image onto a board, box, avionics computer, or spacecraft computer.
5. They observe telemetry and command responses to verify that the target behaves as expected.
6. They fix issues on the development machine, rebuild, and deploy a new target image.

That is exactly the loop OTCS demonstrates with:

* edit source on the PC
* build firmware image
* flash Pico
* observe telemetry
* send commands
* inspect acknowledgements and behavior

## Development, Build, And Test Flow In Practice

### 1. Development on the engineer's computer

Engineers usually write and review the code on a workstation first.

For OTCS, this includes:

* editing `flight_computer/`, `protocol/`, and `common/`
* running host-side tests in `tests/`
* running the Ground Station as the operator tool

This matters because host-side work is faster, easier to inspect, and easier to
debug than constantly reflashing target hardware for every small change.

### 2. Cross-compiling for the target

In embedded work, the target processor is usually different from the engineer's
development machine.

That means:

* the engineer writes code on a PC or Mac
* a target-specific compiler produces machine code for the embedded processor
* the output is a deployable firmware image

In OTCS, the result is a Pico `.uf2` image. In larger aerospace programs, the
output might be a different image format, but the concept is the same: build a
deployable target binary from source.

### 3. Loading software onto hardware

After the build, the firmware image is loaded onto the hardware target.

In OTCS, that means flashing the Pico.

In a larger aerospace or defense setting, that might mean:

* loading software onto a lab avionics box
* deploying to an engineering model of a flight computer
* updating a hardware-in-the-loop rack
* preparing a flight-approved image for a vehicle or spacecraft computer

The key point is that the running target is executing the compiled image, not
reaching back to source files on the engineer's computer.

### 4. Testing in layers

Real embedded programs usually do not jump straight from source code to mission
operations. They test in layers.

Typical layers look like:

* unit tests for small logic pieces
* host-side functional tests
* target-board smoke tests
* integration tests across interfaces
* hardware-in-the-loop testing
* long-duration or stress testing

OTCS has smaller versions of these ideas:

* `tests/` validates protocol parsing and flight-computer behavior on the host
* Pico serial testing checks that the real board emits telemetry and accepts commands
* the Ground Station plus Pico demo is a small integration test

### 5. Observing the system through telemetry

In embedded, aerospace, robotics, and defense systems, engineers often do not
have direct access to internal state while the target is running.

Instead, they rely on:

* telemetry
* status words
* logs
* debug ports
* fault reports
* command acknowledgements

OTCS follows that pattern. The Ground Station does not reach inside the Pico's
memory directly. It learns what the Pico is doing by reading:

* `TM ...` telemetry lines
* `ACK ...` command results
* link-health behavior

That is a very embedded way to work.

## What Back-And-Forth Communication Looks Like In Real Programs

The communication pattern in OTCS also mirrors real operational thinking, even
though OTCS uses a much simpler transport.

### OTCS pattern

```text
engineer command
    ->
Ground Station serial command
    ->
Pico command parser
    ->
flight logic update
    ->
acknowledgement
    ->
next telemetry reflects new state
```

That same high-level pattern shows up in larger systems:

* an operator or engineer sends a command
* the vehicle or spacecraft software validates it
* the software either accepts or rejects it
* the system emits status and telemetry showing the current state afterward

### Spacecraft-style example

A simplified spacecraft-style sequence might look like:

```text
Ground sends command: switch subsystem to SAFE
    ->
spacecraft validates command packet
    ->
flight software routes command to the right application
    ->
subsystem changes mode if allowed
    ->
spacecraft emits command acceptance/rejection status
    ->
later telemetry shows SAFE mode is now active
```

OTCS does the same thing in miniature:

```text
Ground Station sends CMD SET_MODE SAFE
    ->
Pico parses command
    ->
FlightComputer sets SAFE mode
    ->
Pico sends ACK SET_MODE OK
    ->
next TM line shows MODE=SAFE
```

### Aircraft or defense-system-style example

A simplified aircraft, missile, or defense-electronics style loop might look
like:

```text
engineering station sends command or configuration request
    ->
embedded computer validates message format and authority
    ->
control software applies the requested change if safe
    ->
status bus or telemetry channel reports result
    ->
engineers verify the expected state through instrumentation and status messages
```

The transport there may be very different from OTCS, such as Ethernet, CAN,
MIL-STD-1553, SpaceWire, or a mission-specific radio link, but the logic is
similar:

* command goes in
* software validates it
* software updates state if allowed
* telemetry confirms the outcome

## What Is Simpler In OTCS Than In A Real Program

OTCS is intentionally small. Real flight or defense software usually adds many
more concerns:

* stronger fault management
* watchdogs and reset handling
* redundancy and failover
* packet checksums or CRCs
* command authentication or authority checks
* stricter timing guarantees
* real sensor and actuator drivers
* bootloaders and version control for deployed images
* formal verification, certification, or mission-assurance processes
* large test campaigns and hardware-in-the-loop labs

OTCS does not try to fully reproduce that environment. It gives you the core
mental model in a form that is easy to understand:

* portable mission logic
* target-specific firmware wrapper
* explicit telemetry
* explicit commanding
* deploy-test-observe-repeat

## Code Ownership

Use this split:

```text
flight_computer/
    Platform-independent spacecraft logic:
    modes, telemetry state, command handling, faults, recovery behavior.

firmware/pico_satellite_node/
    Pico-specific wrapper:
    stdio_init_all(), USB serial, timing, future GPIO, and calls into the
    shared flight computer logic.

protocol/
    Shared telemetry, command, and acknowledgement text formats.

common/
    Shared enums, structs, and helpers used by host and firmware code.

ground_station/
    Desktop mission-control application.
```

The long-term goal is for `firmware/pico_satellite_node/main.cpp` to stay thin:
initialize Pico hardware, create a `FlightComputer`, call `tick()`, format
telemetry, print it over USB serial, read commands, and send acknowledgements.
That text-protocol command loop is now implemented.

## VS Code Pico Extension

Use the official Raspberry Pi Pico VS Code extension for Pico firmware work.

The extension provides buttons and commands such as:

* New C/C++ Project
* Compile Project
* Run Project (USB)
* Switch Board
* Switch SDK

The extension also manages the Pico SDK and toolchain under the user's profile.
That is why firmware can build through VS Code even if `arm-none-eabi-gcc` is
not globally available in a normal PowerShell session.

## New Project Settings

When creating a Pico project for OTCS, use:

* Board: `Pico 2 W`
* Architecture: `ARM`, not `RISC-V`
* SDK: `2.3.0`
* Console over USB: checked
* Console over UART: unchecked
* Wireless: none
* Generate C++ code: checked
* Run from flash: enabled

For the project options screen:

* Run the program from RAM rather than flash: unchecked
* Use project name as entry point file name: checked
* Generate C++ code: checked
* Enable C++ RTTI: unchecked
* Enable C++ exceptions: unchecked
* Debugger: DebugProbe selected
* Enable CMake-Tools extension integration: unchecked

The extension may initially create an entry point file named after the project,
such as `pico_satellite_node.cpp`. In this repo, the firmware entry point is
standardized as `main.cpp` after project creation.

For optional hardware features, leave these unchecked until needed:

* SPI
* I2C interface
* UART
* PIO interface
* DMA support
* HW interpolation
* HW watchdog
* HW timer
* HW clocks

## Why These Settings

`ARM`, not `RISC-V`: ARM is the common/default Pico C/C++ path and keeps early
bring-up close to the official examples.

`Console over USB`: makes `printf(...)` appear on the Windows COM port, which is
what Python miniterm reads.

`Console over UART` off: no external USB-to-UART adapter or jumper wiring is
needed for early OTCS work.

`No wireless`: the board supports Wi-Fi, but USB serial is simpler and more
reliable for first telemetry and command work.

`Run from flash`: normal firmware behavior. The Pico keeps running the program
after reboot.

`Run from RAM` off: RAM execution is useful for some temporary/debug workflows,
but it is not the normal deployment path.

`C++ RTTI` and `C++ exceptions` off: these are useful in some desktop C++ code,
but they add overhead and are unnecessary for the early embedded firmware.

`DebugProbe`: fine as the default debugger selection even before a physical
Debug Probe is available. Early OTCS testing uses USB serial output.

`CMake-Tools integration` off: the Pico extension already handles the firmware
workflow for now. The host-side repo still uses normal CMake presets.

## Build And Flash

From VS Code, open the Pico project:

```text
firmware/pico_satellite_node/
```

Use:

```text
Compile Project
Run Project (USB)
```

`Run Project (USB)` builds the project, creates a `.uf2`, detects the Pico in
USB bootloader mode, copies the `.uf2` to the Pico, and lets the Pico reboot
into the new firmware.

Manual flashing still works:

1. Unplug the Pico.
2. Hold `BOOTSEL`.
3. Plug it back in.
4. Release `BOOTSEL` when the `RP2350` drive appears.
5. Copy the generated `.uf2` file to that drive.

The drive disappears after flashing because the Pico has rebooted out of
bootloader mode and into the new firmware.

## Serial Test

Install pyserial once from the repo root:

```powershell
python -m pip install -r requirements-dev.txt
```

Watch the Pico:

```powershell
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the port assigned by Windows.

The current Pico firmware prints OTCS text telemetry once per second:

```text
TM SAT=1 TIME=1000 SEQ=1 MODE=BOOT TEMP=22 BAT=100 FAULTS=0 UPTIME=1000
TM SAT=1 TIME=2000 SEQ=2 MODE=NORMAL TEMP=22 BAT=99 FAULTS=0 UPTIME=2000
```

It also polls USB serial for text commands while waiting between telemetry
cycles. Valid command lines include:

```text
CMD PING
CMD RESET
CMD SET_MODE SAFE
CMD SET_MODE NORMAL
CMD INJECT_FAULT LOW_BATTERY
CMD CLEAR_FAULT LOW_BATTERY
```

The firmware parses each command, applies it to `otcs::FlightComputer`, and
prints an acknowledgement:

```text
ACK PING OK
ACK RESET OK
ACK SET_MODE OK
ACK INJECT_FAULT OK
ACK CLEAR_FAULT OK
```

Invalid command text produces:

```text
ERR CMD INVALID
```

For the single combined setup, protocol, and demo guide, see
[docs/SETUP.md](SETUP.md).

For the high-level explanation of the portable spacecraft logic that this
firmware calls into, see [docs/FLIGHT_COMPUTER.md](FLIGHT_COMPUTER.md).

Quit miniterm with `Ctrl+]`.

The C++ Ground Station can read the telemetry stream and send commands:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Close miniterm before starting the Ground Station because Windows serial ports
are opened exclusively.

Verified live command sequence:

```text
RESET
PING
SET_MODE SAFE
SET_MODE NORMAL
INJECT_FAULT LOW_BATTERY
SET_MODE NORMAL
CLEAR_FAULT LOW_BATTERY
PING
```

Expected recovery behavior:

```text
INJECT_FAULT LOW_BATTERY -> ACK INJECT_FAULT OK -> telemetry enters SAFE with FAULTS=1
SET_MODE NORMAL while faulted -> ACK SET_MODE REJECTED
CLEAR_FAULT LOW_BATTERY -> ACK CLEAR_FAULT OK -> telemetry returns NORMAL with FAULTS=0
```
