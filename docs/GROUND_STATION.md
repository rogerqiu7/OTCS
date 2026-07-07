# Ground Station

## Purpose

The Ground Station is the desktop operator app. It does four jobs:

* open the Pico USB serial port
* parse telemetry and acknowledgements
* show the latest spacecraft state in a terminal dashboard
* send operator commands back to the Pico

Main entry point:

```text
ground_station/main.cpp
```

Run it on Windows with:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

## High-Level Flow

```text
Pico -> TM telemetry -> Ground Station
Ground Station -> CMD command -> Pico
Pico -> ACK acknowledgement -> Ground Station
```

The Ground Station also writes:

```text
logs/events_###.log
logs/telemetry_###.csv
```

## Important Split

The Ground Station does not run the spacecraft brain.

`otcs::FlightComputer` is created in the Pico firmware, not here:

```text
firmware/pico_satellite_node/main.cpp
```

The Ground Station is only the operator-side transport, display, and logging
application.

## Main Pieces

### `ground_station/main.cpp`

This is the control loop for the desktop app.

High-level responsibilities:

* validate the serial-port argument
* open `SerialPort`
* create `MissionLogger`
* keep the current dashboard state in `DisplayState`
* read incoming serial lines
* decide whether a line is an `ACK`, telemetry, or garbage
* update link-health state
* poll keyboard input
* parse operator commands and send canonical `CMD ...` lines
* redraw the dashboard when state changes

Important helpers in this file:

* `print_usage(...)`: prints the expected command line
* `telemetry_age_text(...)`: converts last-telemetry time into text like `0s ago`
* `acknowledgement_text(...)`: formats the last parsed `ACK` into a readable string
* `render_dashboard(...)`: clears the terminal and redraws the full UI
* `parse_user_command(...)`: uppercases typed input and adds `CMD` if needed
* `poll_console_command(...)`: non-blocking keyboard input on Windows

### `DisplayState`

`DisplayState` is the current UI model. It stores:

* latest parsed telemetry
* latest parsed acknowledgement
* last raw RX line
* last raw TX line
* last event message
* whether telemetry has ever been received
* whether the link is stale
* the timestamp of the last valid telemetry packet

This lets the app redraw a stable dashboard instead of printing an endless text
stream.

### `ground_station/serial_port.[hpp|cpp]`

`SerialPort` is the transport wrapper around the Windows COM-port API.

Method summary:

* `SerialPort(...)`: opens the port, applies `115200 8N1`, sets timeouts, and clears buffers
* `port_name()`: returns the configured COM-port name
* `read_line()`: blocking convenience loop that waits until a full line exists
* `try_read_line(...)`: non-blocking line reader built on top of byte polling and an internal `pending_line_`
* `write_line(...)`: writes a full line with a trailing newline
* `close()`: releases the Windows handle

Why it exists:

* keeps Win32 serial details out of `main.cpp`
* makes the application logic easier to read
* isolates the current platform-specific boundary

### `ground_station/mission_logger.[hpp|cpp]`

`MissionLogger` owns the mission log files.

Method summary:

* `MissionLogger(...)`: creates the log directory, picks the next `events_###` and `telemetry_###` filenames, and opens both files
* `events_path()`: returns the event-log path
* `telemetry_path()`: returns the telemetry CSV path
* `log_connected(...)`: records initial port connection
* `log_rx(...)`: records raw incoming lines
* `log_tx(...)`: records raw outgoing lines
* `log_note(...)`: records operator and link-health events
* `log_telemetry(...)`: appends parsed telemetry fields to CSV
* `next_log_path(...)`: finds the next unused numbered log filename
* `timestamp()`: formats the host clock for log entries

Why it exists:

* the Ground Station needs durable logs
* dashboard state is for the operator right now
* logs are for later review

### `ground_station/status_banner.[hpp|cpp]`

This is intentionally tiny.

Method summary:

* `print_status_banner()`: prints the application name at startup

It is separated only to keep startup UI text distinct from the main control
loop.

## Link Health

The link-health rule is simple:

* valid telemetry updates `last_telemetry_time`
* if more than 3 seconds pass without valid telemetry, the link becomes stale
* when valid telemetry resumes, the link becomes healthy again

Only telemetry resets the timer. Acknowledgements do not count as proof that
the telemetry stream is healthy.

## Big Picture

This is the important architectural view:

```text
Ground Station main loop
    |
    +-- SerialPort opens COM port and moves raw text lines
    |
    +-- text_protocol parses TM and ACK messages
    |
    +-- DisplayState holds the latest mission picture
    |
    +-- MissionLogger records RX, TX, telemetry, and link events
    |
    +-- operator input becomes CMD lines sent back to the Pico
```

So the Ground Station is not spacecraft logic. It is the desktop shell around
transport, parsing, display, logging, and operator control.
