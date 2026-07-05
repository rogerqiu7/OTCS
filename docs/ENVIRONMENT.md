# Environment

## Goal

Keep local development reproducible for the completed OTCS host and Pico
workflow.

The environment supports a `C++20` workflow for the Ground Station, shared code,
host tests, host demos, and Pico firmware flashing.

## Source Of Truth

* [Brewfile](../Brewfile)
* [requirements-dev.txt](../requirements-dev.txt)
* [CMakeLists.txt](../CMakeLists.txt)
* [CMakePresets.json](../CMakePresets.json)
* [docs/FIRMWARE.md](FIRMWARE.md)

## Current macOS Check

Verified on July 1, 2026:

* `Apple clang version 21.0.0`
* `cmake version 4.0.2`
* `git version 2.50.1`
* `Homebrew 5.1.5`
* `ninja` not installed yet

## Current Windows Check

Verified on July 2, 2026:

* Windows 11 Home 25H2
* PowerShell 7.6.3
* Visual Studio Community 2026
* MSVC 19.50
* CMake 4.3.4
* Ninja 1.13.2
* Git 2.54.0.windows.1
* Python 3.11 with `pyserial` for Pico USB serial monitoring
* Raspberry Pi Pico VS Code extension with Pico SDK 2.3.0 for firmware builds
* Ground Station command, telemetry, logging, dashboard, and link-health flow verified on hardware

The Windows build requires a Visual Studio developer environment so that both
`cl.exe` and the Windows SDK libraries, such as `kernel32.lib`, are available.
Pico firmware builds are currently handled through the Raspberry Pi Pico VS Code
extension, which manages its own SDK/toolchain setup.

## macOS Environment Notes

The primary hardware demo has been verified on Windows. The host-side CMake
project also includes macOS presets. A macOS host build uses:

```bash
brew bundle
cmake --preset macos-debug
cmake --build --preset build-macos-debug
```

## Windows Environment Steps

1. Open a Developer PowerShell for Visual Studio, or run:

   ```powershell
   & 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -HostArch amd64
   ```

2. Return to the repo root:

   ```powershell
   cd C:\Users\roger\OneDrive\Documents\projects\OTCS
   ```

3. Configure and build:

   ```powershell
   cmake --preset windows-debug
   cmake --build --preset build-windows-debug
   ```

4. Build the Flight Computer host demo when needed:

   ```powershell
   cmake --build build/windows-debug --target otcs_flight_computer_host_demo
   ```

5. Install Python developer tools for Pico serial testing:

   ```powershell
   python -m pip install -r requirements-dev.txt
   ```

6. Monitor Pico USB serial output with Python miniterm:

   ```powershell
   python -m serial.tools.miniterm COM3 115200
   ```

   Replace `COM3` with the port assigned by Windows. If the Pico has just been
   flashed, the port may disappear and reappear while the board reboots.

7. For Pico firmware, open `firmware/pico_satellite_node/` in VS Code and use
   the Raspberry Pi Pico extension:

   ```text
   Compile Project
   Run Project (USB)
   ```

## Environment Expectations

The environment should make it easy to maintain:

* C++20 support on macOS
* C++20 support on Windows with MSVC
* portable host-side builds
* separate build outputs from source
* reproducible local configuration
* terminal-based Pico serial testing through Python `pyserial`
* Pico firmware flashing through the Raspberry Pi Pico VS Code extension
