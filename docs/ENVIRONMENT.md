# Environment

## Goal

Keep local development reproducible on macOS before Pico hardware arrives.

The current host-side environment should support a `C++20` workflow for the Ground Station, shared code, and host demos.

## Source Of Truth

* [Brewfile](/Users/rogerqiu/Documents/code/otcs/Brewfile)
* [CMakeLists.txt](/Users/rogerqiu/Documents/code/otcs/CMakeLists.txt)
* [CMakePresets.json](/Users/rogerqiu/Documents/code/otcs/CMakePresets.json)

## Current Local Check

Verified on July 1, 2026:

* `Apple clang version 21.0.0`
* `cmake version 4.0.2`
* `git version 2.50.1`
* `Homebrew 5.1.5`
* `ninja` not installed yet

## Next Environment Steps

1. Run `brew bundle` from the repo root.
2. Verify `ninja --version`.
3. Configure the first local build with `cmake --preset macos-debug`.
4. Add editor settings or formatter configuration once source files exist.

## Environment Expectations

The environment should make it easy to maintain:

* C++20 support on macOS
* portable host-side builds
* separate build outputs from source
* reproducible local configuration
