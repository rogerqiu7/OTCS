# Flight Computer

## Purpose

The flight computer is the portable spacecraft brain.

It owns the spacecraft state machine and command behavior without depending on
Windows or Pico SDK APIs. That is why the same code can be:

* tested on the host in `tests/`
* exercised by the host demo in `flight_computer/main.cpp`
* compiled into the Pico firmware in `firmware/pico_satellite_node/`

The main class lives in:

```text
flight_computer/flight_computer.hpp
flight_computer/flight_computer.cpp
```

## What It Owns

`otcs::FlightComputer` stores:

* spacecraft mode
* active fault flags
* timestamp and uptime
* telemetry sequence count
* simulated battery percentage
* simulated temperature
* whether boot has completed

That is the actual spacecraft state that the Pico runs.

## Main Methods

### `telemetry()`

Returns a `TelemetrySnapshot` describing the current internal state.

What it does:

* copies the current fields into the shared telemetry struct
* gives the protocol layer something stable to format as `TM ...`

### `tick(std::uint32_t elapsed_ms)`

Advances the spacecraft by one time step and returns the new telemetry snapshot.

What it does:

* advances timestamp and uptime
* increments the telemetry sequence
* reports `BOOT` once before switching into `NORMAL`
* drains battery in a simple deterministic way
* forces `SAFE` if faults are active while the craft is in `NORMAL`

This is the heartbeat of the spacecraft simulation.

### `handle_command(const Command& command)`

Applies a parsed command and returns an `Acknowledgement`.

What it does by command type:

* `PING` and `REQUEST_STATUS`: always return `OK`
* `RESET`: restores boot-state defaults
* `SET_MODE`: changes mode unless `NORMAL` is requested while faults are active
* `RESET_FAULT`: clears all faults and returns to `NORMAL` from `SAFE` or `FAULT`
* `INJECT_FAULT`: sets a fault flag and moves to `SAFE` if needed
* `CLEAR_FAULT`: clears one fault and returns to `NORMAL` when no faults remain

This is where the spacecraft makes accept/reject decisions.

### `enter_safe_if_faulted()`

Private helper that moves the spacecraft from `NORMAL` to `SAFE` when any fault
flag is active.

### `drain_battery()`

Private helper that subtracts 1 percent of battery per simulated second, never
below zero.

This is intentionally simple so behavior stays deterministic in tests.

## Host Demo

`flight_computer/main.cpp` is a tiny host-side example program.

What it does:

* creates a `FlightComputer`
* runs a few `tick()` cycles
* parses a couple of sample commands
* prints telemetry and acknowledgements

It is useful for checking the portable logic without the Pico wrapper or Ground
Station.

## What The Pico Runs

The Pico firmware entry point is:

```text
firmware/pico_satellite_node/main.cpp
```

That file is a thin hardware wrapper around the shared flight computer.

Important functions there:

* `poll_usb_line(...)`: collects one newline-terminated command from USB serial
* `process_command_line(...)`: parses a command, calls `handle_command(...)`, and prints either `ACK ...` or `ERR CMD INVALID`
* `main()`: initializes USB serial, creates `otcs::FlightComputer`, emits telemetry once per second, polls for commands between telemetry cycles

The Pico firmware loop is:

```text
create FlightComputer
    |
    v
tick(1000)
    |
    v
TelemetrySnapshot
    |
    v
format_telemetry(...)
    |
    v
print TM line over USB serial
    |
    v
poll for command text
    |
    v
parse_command(...)
    |
    v
handle_command(...)
    |
    v
format_acknowledgement(...)
    |
    v
print ACK line over USB serial
```

## Big Picture

This is the important architectural view of what the Pico is actually running:

```text
Pico SDK wrapper
    |
    +-- stdio_init_all()
    +-- sleep_ms(...)
    +-- getchar_timeout_us(...)
    +-- printf(...)
    |
    v
shared FlightComputer
    |
    +-- owns spacecraft mode, faults, clocks, and battery
    +-- advances state with tick()
    +-- handles commands with handle_command()
    |
    v
shared text protocol
    |
    +-- formats TM lines
    +-- parses CMD lines
    +-- formats ACK lines
```

So the Pico is not running random ad hoc serial code. It is running a small
stack:

* Pico-specific I/O and timing wrapper
* shared flight computer logic
* shared text protocol

That is the onboard software path for OTCS.
