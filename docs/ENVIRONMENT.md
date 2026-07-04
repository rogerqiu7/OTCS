# Environment

## Goal

Keep local development reproducible across macOS and Windows before Pico
hardware-specific firmware is added.

The current host-side environment should support a `C++20` workflow for the Ground Station, shared code, and host demos.

## Source Of Truth

* [Brewfile](../Brewfile)
* [requirements-dev.txt](../requirements-dev.txt)
* [CMakeLists.txt](../CMakeLists.txt)
* [CMakePresets.json](../CMakePresets.json)

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

The Windows build requires a Visual Studio developer environment so that both
`cl.exe` and the Windows SDK libraries, such as `kernel32.lib`, are available.

## Next macOS Environment Steps

1. Run `brew bundle` from the repo root.
2. Verify `ninja --version`.
3. Configure the first local build with `cmake --preset macos-debug`.
4. Add editor settings or formatter configuration once source files exist.

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

## Environment Expectations

The environment should make it easy to maintain:

* C++20 support on macOS
* C++20 support on Windows with MSVC
* portable host-side builds
* separate build outputs from source
* reproducible local configuration
* terminal-based Pico serial testing through Python `pyserial`
