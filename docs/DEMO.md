# OTCS Demo Script

This script demonstrates the completed first version of OTCS: a Raspberry Pi
Pico 2 W running flight software, a native C++ Ground Station, two-way USB
serial command and telemetry, fault injection, recovery, logging, link health,
and a live terminal dashboard.

## 1. Flash The Pico

Open this folder in VS Code:

```text
firmware/pico_satellite_node/
```

Use the Raspberry Pi Pico extension:

```text
Compile Project
Run Project (USB)
```

The Pico reboots into the OTCS satellite node firmware.

## 2. Build The Ground Station

From a Visual Studio Developer PowerShell:

```powershell
cd C:\Users\roger\OneDrive\Documents\projects\OTCS
cmake --build --preset build-windows-debug
```

## 3. Start The Ground Station

Close Python miniterm or any other serial monitor first. Windows allows only one
program to own the COM port.

Run:

```powershell
.\build\windows-debug\ground_station\otcs_ground_station.exe COM3
```

Replace `COM3` if Windows assigned a different port.

The dashboard shows:

```text
Connection: ONLINE
Spacecraft mode
Battery
Temperature
Fault flags
Last telemetry age
Last command
Last acknowledgement
Event and telemetry log paths
```

## 4. Confirm The Link

Type:

```text
PING
```

Expected dashboard result:

```text
Last TX:  CMD PING
Last ACK: PING OK
```

## 5. Reset The Simulated Spacecraft

Type:

```text
RESET
```

Expected result:

```text
ACK RESET OK
Mode returns through BOOT/NORMAL
Battery returns to 100%
```

## 6. Manual Mode Change

Type:

```text
SET_MODE SAFE
```

Expected result:

```text
ACK SET_MODE OK
Mode: SAFE
```

Then type:

```text
SET_MODE NORMAL
```

Expected result:

```text
ACK SET_MODE OK
Mode: NORMAL
```

## 7. Fault Injection And Rejection

Type:

```text
INJECT_FAULT LOW_BATTERY
```

Expected result:

```text
ACK INJECT_FAULT OK
Mode: SAFE
Faults: 1
```

Now try to force normal operation:

```text
SET_MODE NORMAL
```

Expected result:

```text
ACK SET_MODE REJECTED
Mode remains SAFE
Faults remains 1
```

This shows that the Flight Computer rejects unsafe recovery while a fault is
active.

## 8. Fault Recovery

Type:

```text
CLEAR_FAULT LOW_BATTERY
```

Expected result:

```text
ACK CLEAR_FAULT OK
Mode: NORMAL
Faults: 0
```

## 9. Review Mission Logs

Exit the Ground Station:

```text
EXIT
```

Then inspect the logs shown by the dashboard:

```powershell
Get-Content logs\events_001.log
Import-Csv logs\telemetry_001.csv | Select-Object -First 5
```

The actual suffix may be higher than `001` if previous runs already created log
files.

The event log shows connection, RX, TX, ACK, link-health, and operator events.
The telemetry CSV contains parsed telemetry rows.

## 10. Link Health Demo

The Ground Station marks the link stale if valid telemetry stops for more than
3 seconds after the first valid telemetry packet.

Expected dashboard/event behavior:

```text
Connection: STALE
Event: No telemetry received for 3 seconds
```

Event log:

```text
LINK_STALE NO_TELEMETRY_3S
LINK_RECOVERED
```

If the Pico is physically unplugged, Windows may close the COM port and report a
serial error. The stale-link path is for a serial link that remains open but
stops receiving valid telemetry.

## Demo Summary

The demo proves:

```text
Embedded firmware runs independently on the Pico
Telemetry streams from Pico to Ground Station
Ground Station sends commands back over USB serial
Pico parses, validates, and executes commands
Pico sends acknowledgements
Fault injection drives SAFE mode
Unsafe recovery is rejected
Fault clearing returns the system to NORMAL
Ground Station logs the mission
Ground Station detects stale telemetry
```
