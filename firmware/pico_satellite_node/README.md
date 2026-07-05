# OTCS Pico Satellite Node

This is the real Pico 2 W firmware project for OTCS.

It was created with the Raspberry Pi Pico VS Code extension and should be built
and flashed with that extension during early development.

## Current Behavior

The firmware initializes USB serial, runs the shared OTCS flight computer logic,
and prints text telemetry once per second:

```text
TM SAT=1 TIME=1000 SEQ=1 MODE=BOOT TEMP=22 BAT=100 FAULTS=0 UPTIME=1000
TM SAT=1 TIME=2000 SEQ=2 MODE=NORMAL TEMP=22 BAT=99 FAULTS=0 UPTIME=2000
```

## Build And Flash

Open this folder in VS Code:

```text
firmware/pico_satellite_node/
```

Use the Raspberry Pi Pico extension:

```text
Compile Project
Run Project (USB)
```

`Run Project (USB)` builds the firmware, creates a `.uf2`, copies it to the
Pico in BOOTSEL mode, and lets the board reboot into the new firmware.

## Test Serial Output

From the repo root:

```powershell
python -m serial.tools.miniterm COM3 115200
```

Replace `COM3` with the Windows COM port assigned to the Pico.

## Intended Direction

This firmware should stay as the Pico hardware wrapper. Shared spacecraft logic
belongs in `flight_computer/`, shared message formatting belongs in `protocol/`,
and shared types belong in `common/`.
