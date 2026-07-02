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
* Ground Station host executable builds and runs
* Flight Computer host demo builds and runs

## Required Tools

For local Mac development, install or verify:

* Apple Command Line Tools
* Homebrew
* CMake
* Ninja
* Git
* VS Code or another C++ editor

For local Windows development, install or verify:

* Visual Studio with the Desktop development with C++ workload
* CMake
* Ninja
* Git
* VS Code or another C++ editor

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

There is no direct C++ equivalent to `requirements.txt`.

For this repo, environment and dependency intent is tracked with:

* `Brewfile` for Mac tool installation
* `CMakeLists.txt` for build configuration
* `CMakePresets.json` for repeatable local build setup

The build is configured for `C++20` in the root [CMakeLists.txt](../CMakeLists.txt).

Later, if external C++ libraries are added, we can introduce either:

* `vcpkg.json`
* `conanfile.txt` or `conanfile.py`

For now, no third-party C++ libraries are required.
