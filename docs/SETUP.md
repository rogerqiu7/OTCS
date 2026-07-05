# Setup

## Status

Local macOS environment verified:

* `clang++` installed
* `cmake` installed
* `git` installed
* `brew` installed
* `ninja` missing at time of scaffold

Windows host build verified:

* Visual Studio Community 2026 with MSVC 19.50
* CMake 4.3.4
* Ninja 1.13.2
* Git 2.54.0.windows.1
* Python 3.11 with `pyserial` for Pico USB serial testing
* Ground Station host executable builds and runs
* Flight Computer host demo builds and runs

## Required Tools

For local Mac development, install or verify:

* Apple Command Line Tools
* Homebrew
* CMake
* Ninja
* Git
* Python
* VS Code or another C++ editor

For local Windows development, install or verify:

* Visual Studio with the Desktop development with C++ workload
* CMake
* Ninja
* Git
* Python
* `pyserial` from [requirements-dev.txt](../requirements-dev.txt)
* VS Code or another C++ editor
* Raspberry Pi Pico VS Code extension for Pico firmware build/flash workflow

## Project Standard

The project standard is `C++20`.

The code style should still favor conservative embedded-friendly design:

* explicit ownership
* simple interfaces
* limited abstraction depth
* cross-platform host builds
* clean separation between desktop and embedded responsibilities

## Install Commands

```bash
xcode-select --install
brew bundle
```

`brew bundle` uses the repo's [Brewfile](../Brewfile).

## macOS Verification Commands

```bash
clang++ --version
cmake --version
ninja --version
git --version
```

## Windows Verification Commands

Run these from a Developer PowerShell for Visual Studio:

```powershell
cl
cmake --version
ninja --version
git --version
python --version
```

## Python Developer Tooling

Install the Python serial monitor dependency from the repo root.

Windows:

```powershell
python -m pip install -r requirements-dev.txt
```

macOS:

```bash
python3 -m pip install -r requirements-dev.txt
```

## First macOS Build

After `ninja` is installed:

```bash
cmake --preset macos-debug
cmake --build --preset build-macos-debug
```

## Windows Host Build

Install or verify:

* Visual Studio with the Desktop development with C++ workload
* CMake
* Ninja
* Git

If using a normal PowerShell session, initialize the Visual Studio developer
environment before configuring:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -HostArch amd64
cd C:\Users\roger\OneDrive\Documents\projects\OTCS
```

Then configure and build:

```powershell
cmake --preset windows-debug
cmake --build --preset build-windows-debug
cmake --build build/windows-debug --target otcs_flight_computer_host_demo
```

Run the host executables:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe
.\build\windows-debug\flight_computer\otcs_flight_computer_host_demo.exe
```

Run tests:

```powershell
ctest --test-dir build/windows-debug --output-on-failure
```

The current `tests/` directory is a placeholder, so CTest may report that no
tests were found until the first test target is added.

## Pico USB Serial Monitor

For Pico bring-up, use Python's `pyserial` miniterm instead of PuTTY. This keeps
serial testing in the same terminal workflow as the rest of the project.

Install the dependency once:

```powershell
python -m pip install -r requirements-dev.txt
```

Then open the Pico serial port:

```powershell
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the COM port Windows assigns to the Pico. If the port is not
available immediately after flashing a UF2, wait a moment and run the command
again; the Pico may reboot and briefly disconnect while switching from
bootloader mode to firmware mode.

Expected output from the Raspberry Pi Hello World UF2 looks like:

```text
Hello, world!
I'm an RP2350 running RISC-V
```

Quit miniterm with `Ctrl+]`.

## Pico Firmware Workflow

The active Pico firmware project lives in
[firmware/pico_satellite_node](../firmware/pico_satellite_node).

Use the official Raspberry Pi Pico VS Code extension for firmware work. The
extension manages the Pico SDK/toolchain for the project and can build/flash the
Pico even when `arm-none-eabi-gcc` is not globally available in normal
PowerShell.

Open `firmware/pico_satellite_node/` in VS Code and use:

```text
Compile Project
Run Project (USB)
```

`Run Project (USB)` builds the project, creates a `.uf2` firmware image, copies
it to the Pico in BOOTSEL/USB mode, and lets the Pico reboot into the new
firmware.

Then monitor it:

```powershell
python -m serial.tools.miniterm COM3 115200
```

The current expected output is OTCS text telemetry once per second:

```text
TM SAT=1 TIME=1000 SEQ=1 MODE=BOOT TEMP=22 BAT=100 FAULTS=0 UPTIME=1000
TM SAT=1 TIME=2000 SEQ=2 MODE=NORMAL TEMP=22 BAT=99 FAULTS=0 UPTIME=2000
```

For detailed Pico project settings and the reasoning behind them, see
[docs/FIRMWARE.md](FIRMWARE.md).

### Windows troubleshooting

If `cl` is not recognized, or CMake fails with:

```text
LINK : fatal error LNK1104: cannot open file 'kernel32.lib'
```

the Visual Studio compiler or Windows SDK environment is not loaded in the
current shell. Start a Developer PowerShell for Visual Studio, or run
`Launch-VsDevShell.ps1` before configuring the project. After launching the
developer shell, return to the repo directory before running CMake because
Visual Studio may change the current directory.

## Notes

There is no direct C++ equivalent to `requirements.txt`, but the repo does track
small Python developer tools used around the C++ workflow.

For this repo, environment and dependency intent is tracked with:

* `Brewfile` for Mac tool installation
* `requirements-dev.txt` for Python developer tools such as `pyserial`
* `CMakeLists.txt` for build configuration
* `CMakePresets.json` for repeatable local build setup

The build is configured for `C++20` in the root [CMakeLists.txt](../CMakeLists.txt).

Later, if external C++ libraries are added, we can introduce either:

* `vcpkg.json`
* `conanfile.txt` or `conanfile.py`

For now, no third-party C++ libraries are required.
