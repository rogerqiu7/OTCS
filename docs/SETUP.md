# Setup

## Status

Local macOS environment verified on this machine:

* `clang++` installed
* `cmake` installed
* `git` installed
* `brew` installed
* `ninja` missing at time of scaffold

## Required Tools

For local Mac development, install or verify:

* Apple Command Line Tools
* Homebrew
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

`brew bundle` uses the repo's [Brewfile](/Users/rogerqiu/Documents/code/otcs/Brewfile).

## Verification Commands

```bash
clang++ --version
cmake --version
ninja --version
git --version
```

## First Build

After `ninja` is installed:

```bash
cmake --preset macos-debug
cmake --build --preset build-macos-debug
```

## Notes

There is no direct C++ equivalent to `requirements.txt`.

For this repo, environment and dependency intent is tracked with:

* `Brewfile` for Mac tool installation
* `CMakeLists.txt` for build configuration
* `CMakePresets.json` for repeatable local build setup

The build is configured for `C++20` in the root [CMakeLists.txt](/Users/rogerqiu/Documents/code/otcs/CMakeLists.txt).

Later, if external C++ libraries are added, we can introduce either:

* `vcpkg.json`
* `conanfile.txt` or `conanfile.py`

For now, no third-party C++ libraries are required.
