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

Quit miniterm with `Ctrl+]`.

The C++ Ground Station can read the same telemetry stream:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Close miniterm before starting the Ground Station because Windows serial ports
are opened exclusively.
