# Setup, Environment, Protocol, and Demo

## Purpose

This is the single operational guide for OTCS.

It replaces the old separate setup, environment, protocol, and demo docs. Use
this file when you need to:

* set up the machine
* build the host code
* flash the Pico
* understand the serial message format
* run the end-to-end hardware demo

## What This Repo Is

OTCS is split into a few small modules:

* `common/`: shared enums, structs, and helpers
* `protocol/`: text telemetry, command, and acknowledgement parsing/formatting
* `flight_computer/`: platform-independent spacecraft state logic
* `ground_station/`: Windows desktop operator app
* `firmware/pico_satellite_node/`: Pico 2 W firmware wrapper that runs the shared flight computer on hardware
* `tests/`: host-side validation

The Pico runs the spacecraft side. The Ground Station is the operator side.

## Environment

### macOS

Use Homebrew packages from [Brewfile](../Brewfile):

```bash
xcode-select --install
brew bundle
```

Current Brewfile contents:

* `cmake`
* `ninja`
* `llvm` as an optional local tool

Verify the basics:

```bash
clang++ --version
cmake --version
ninja --version
git --version
python3 --version
```

### Windows

The hardware demo path is centered on Windows. Install or verify:

* Visual Studio with the Desktop development with C++ workload
* CMake
* Ninja
* Git
* Python 3
* `pyserial` from [requirements-dev.txt](../requirements-dev.txt)
* VS Code with the Raspberry Pi Pico extension

Verified toolchain notes already captured in the repo:

* Visual Studio Community 2026
* MSVC 19.50
* CMake 4.3.4
* Ninja 1.13.2
* Git 2.54.0.windows.1
* Python 3.11
* Pico SDK 2.3.0 through the VS Code extension

Open a Developer PowerShell for Visual Studio, or run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -HostArch amd64
```

Verify the host toolchain:

```powershell
cl
cmake --version
ninja --version
git --version
python --version
```

### Python serial tooling

Install the serial helper once from the repo root:

Windows:

```powershell
python -m pip install -r requirements-dev.txt
```

macOS:

```bash
python3 -m pip install -r requirements-dev.txt
```

## Build

### macOS host build

```bash
cmake --preset macos-debug
cmake --build --preset build-macos-debug
```

### Windows host build

```powershell
cmake --preset windows-debug
cmake --build --preset build-windows-debug
cmake --build build/windows-debug --target otcs_flight_computer_host_demo
```

The root [CMakePresets.json](../CMakePresets.json) defines:

* `macos-debug`
* `macos-release`
* `windows-debug`
* `windows-release`

Host tests are enabled in the Windows presets and disabled in the macOS presets.

### Run tests

```powershell
ctest --test-dir build/windows-debug --output-on-failure
```

Current tests cover:

* shared type conversion in `common/`
* text protocol parsing/formatting in `protocol/`
* command and fault behavior in `flight_computer/`

## Pico firmware

The active firmware project is [firmware/pico_satellite_node](../firmware/pico_satellite_node).

Open that folder in VS Code and use the Raspberry Pi Pico extension:

```text
Compile Project
Run Project (USB)
```

That project compiles:

* `firmware/pico_satellite_node/main.cpp`
* `common/spacecraft_types.cpp`
* `protocol/text_protocol.cpp`
* `flight_computer/flight_computer.cpp`

So the Pico is not just running a one-off sketch. It is running the shared
flight computer and protocol code inside a Pico-specific wrapper.

For more detail on what the Pico actually runs, see
[docs/FLIGHT_COMPUTER.md](FLIGHT_COMPUTER.md) and
[docs/FIRMWARE.md](FIRMWARE.md).

## Serial protocol

The current wire format is newline-delimited text over USB serial.

### Telemetry from Pico to Ground Station

Example:

```text
TM SAT=1 TIME=12345 SEQ=7 MODE=NORMAL TEMP=22 BAT=95 FAULTS=0 UPTIME=12345
```

Required fields, in order:

```text
TM
SAT=<spacecraft id>
TIME=<timestamp milliseconds>
SEQ=<telemetry sequence number>
MODE=<BOOT|NORMAL|SAFE|FAULT>
TEMP=<temperature Celsius>
BAT=<battery percent 0-100>
FAULTS=<fault bitmask>
UPTIME=<uptime milliseconds>
```

`parse_telemetry(...)` is strict. Wrong field order, missing fields, unknown
mode names, or impossible battery values are rejected.

### Commands from Ground Station to Pico

Example:

```text
CMD SET_MODE SAFE
```

Supported command shapes in the current code:

```text
CMD PING
CMD RESET
CMD SET_MODE BOOT
CMD SET_MODE NORMAL
CMD SET_MODE SAFE
CMD SET_MODE FAULT
CMD RESET_FAULT
CMD INJECT_FAULT LOW_BATTERY
CMD INJECT_FAULT HIGH_TEMPERATURE
CMD INJECT_FAULT SENSOR_OFFLINE
CMD INJECT_FAULT COMMS_TIMEOUT
CMD INJECT_FAULT INVALID_COMMAND
CMD CLEAR_FAULT LOW_BATTERY
CMD CLEAR_FAULT HIGH_TEMPERATURE
CMD CLEAR_FAULT SENSOR_OFFLINE
CMD CLEAR_FAULT COMMS_TIMEOUT
CMD CLEAR_FAULT INVALID_COMMAND
CMD REQUEST_STATUS
```

The Ground Station lets the operator type plain commands like `ping` or
`inject_fault low_battery`, normalizes them to uppercase, and sends the
canonical `CMD ...` line.

### Acknowledgements from Pico to Ground Station

Example:

```text
ACK SET_MODE OK
```

Results:

```text
OK
REJECTED
INVALID
```

Malformed command text does not produce an `ACK`. The firmware prints:

```text
ERR CMD INVALID
```

## Running the host tools

### Ground Station

Run the desktop app with the Pico COM port:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Replace `COM3` with the actual port. Close Python miniterm first because
Windows only lets one process own the serial port at a time.

The Ground Station:

* reads Pico telemetry and acknowledgements
* redraws a terminal dashboard
* logs events to `logs/events_###.log`
* logs parsed telemetry to `logs/telemetry_###.csv`
* marks the connection stale after 3 seconds without valid telemetry

For the code walkthrough, see [docs/GROUND_STATION.md](GROUND_STATION.md).

### Flight Computer host demo

Run:

```powershell
.\build\windows-debug\flight_computer\otcs_flight_computer_host_demo.exe
```

This is a tiny host-side demo of the portable spacecraft logic without the Pico
wrapper around it.

## Pico serial monitor

For quick bring-up, use Python miniterm:

```powershell
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the assigned port. Quit miniterm with `Ctrl+]`.

With the OTCS firmware loaded, you should see telemetry like:

```text
TM SAT=1 TIME=1000 SEQ=1 MODE=BOOT TEMP=22 BAT=100 FAULTS=0 UPTIME=1000
TM SAT=1 TIME=2000 SEQ=2 MODE=NORMAL TEMP=22 BAT=99 FAULTS=0 UPTIME=2000
```

You can type commands like:

```text
CMD PING
CMD RESET
CMD SET_MODE SAFE
CMD INJECT_FAULT LOW_BATTERY
CMD CLEAR_FAULT LOW_BATTERY
```

## End-to-end demo

### 1. Flash the Pico

Open [firmware/pico_satellite_node](../firmware/pico_satellite_node) in VS
Code and use:

```text
Compile Project
Run Project (USB)
```

### 2. Build the Ground Station

```powershell
cmake --build --preset build-windows-debug
```

### 3. Start the Ground Station

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

### 4. Confirm the link

Type:

```text
PING
```

Expected result:

```text
ACK PING OK
```

### 5. Reset the spacecraft

```text
RESET
```

Expected result:

```text
ACK RESET OK
Mode returns through BOOT then back to NORMAL
Battery returns to 100%
```

### 6. Change mode manually

```text
SET_MODE SAFE
SET_MODE NORMAL
```

Expected result:

```text
ACK SET_MODE OK
```

### 7. Inject a fault and confirm rejection logic

```text
INJECT_FAULT LOW_BATTERY
SET_MODE NORMAL
```

Expected result:

```text
ACK INJECT_FAULT OK
Mode becomes SAFE
FAULTS becomes 1

ACK SET_MODE REJECTED
Mode stays SAFE
```

### 8. Clear the fault

```text
CLEAR_FAULT LOW_BATTERY
```

Expected result:

```text
ACK CLEAR_FAULT OK
Mode returns to NORMAL
FAULTS becomes 0
```

### 9. Inspect logs

Exit with:

```text
EXIT
```

Then inspect:

```powershell
Get-Content logs\events_001.log
Import-Csv logs\telemetry_001.csv | Select-Object -First 5
```

The exact numeric suffix depends on previous runs.

### 10. Observe link health behavior

If valid telemetry stops for more than 3 seconds after the first good packet,
the Ground Station reports the link as stale and logs:

```text
LINK_STALE NO_TELEMETRY_3S
```

When valid telemetry resumes, it logs:

```text
LINK_RECOVERED
```

## Summary

This one guide now covers the full OTCS operating path:

```text
Build host tools
Flash Pico
Watch telemetry
Send commands
Observe ACK results
Inject and clear faults
Review mission logs
Verify stale-link detection
```
