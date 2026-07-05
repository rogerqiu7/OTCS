# Protocol

## Purpose

This document defines the OTCS telemetry, command, acknowledgement, and error formats.

The protocol design reflects the same engineering priorities as the codebase:

* clear ownership of responsibilities
* stable message formats
* explicit validation rules
* deterministic parsing behavior
* easy logging and diagnostics
* separation between protocol definition and transport mechanism

## Current Text Protocol

The current OTCS wire protocol is newline-delimited text over USB serial. Each
message is one line.

## Telemetry

Telemetry is sent by the Pico to the Ground Station once per second.

```text
TM SAT=1 TIME=12345 SEQ=7 MODE=NORMAL TEMP=22 BAT=95 FAULTS=0 UPTIME=12345
```

Required fields, in order:

```text
TM
SAT=<spacecraft id>
TIME=<timestamp milliseconds>
SEQ=<telemetry sequence number>
MODE=<BOOT|NORMAL|SAFE|FAULT>
TEMP=<temperature Celsius>
BAT=<battery percent, 0-100>
FAULTS=<fault bitmask>
UPTIME=<uptime milliseconds>
```

The parser is intentionally strict. Missing fields, unknown modes, unexpected
field order, and impossible battery values are rejected.

## Commands

Commands are sent by the Ground Station to the Pico. The Ground Station accepts
operator input such as `ping` or `inject_fault low_battery`, normalizes it to
uppercase, adds the `CMD` prefix, and writes the canonical command line.

```text
CMD SET_MODE SAFE
```

Supported commands:

```text
CMD PING
CMD RESET
CMD SET_MODE BOOT
CMD SET_MODE NORMAL
CMD SET_MODE SAFE
CMD SET_MODE FAULT
CMD RESET_FAULT
CMD INJECT_FAULT LOW_BATTERY
CMD INJECT_FAULT HIGH_TEMPERATURE
CMD INJECT_FAULT SENSOR_OFFLINE
CMD INJECT_FAULT COMMS_TIMEOUT
CMD INJECT_FAULT INVALID_COMMAND
CMD CLEAR_FAULT LOW_BATTERY
CMD CLEAR_FAULT HIGH_TEMPERATURE
CMD CLEAR_FAULT SENSOR_OFFLINE
CMD CLEAR_FAULT COMMS_TIMEOUT
CMD CLEAR_FAULT INVALID_COMMAND
CMD REQUEST_STATUS
```

Fault commands require a non-`NONE` fault argument. Unknown commands and extra
arguments are rejected by the parser.

## Acknowledgements

Acknowledgements are sent by the Pico after it parses and applies a command.

```text
ACK SET_MODE OK
```

Result values:

```text
OK
REJECTED
INVALID
```

Examples:

```text
ACK PING OK
ACK SET_MODE OK
ACK SET_MODE REJECTED
ACK INJECT_FAULT OK
ACK CLEAR_FAULT OK
```

`REJECTED` is a valid spacecraft decision. For example, `SET_MODE NORMAL` is
rejected while a fault is still active.

If the Pico receives malformed command text, it emits:

```text
ERR CMD INVALID
```

## Verified Two-Way Demo

This sequence has been tested against the physical Pico over USB serial:

```text
RESET
PING
SET_MODE SAFE
SET_MODE NORMAL
INJECT_FAULT LOW_BATTERY
SET_MODE NORMAL
CLEAR_FAULT LOW_BATTERY
PING
```

Expected protocol-level behavior:

```text
TX: CMD PING
RX: ACK PING OK

TX: CMD INJECT_FAULT LOW_BATTERY
RX: ACK INJECT_FAULT OK
RX: TM ... MODE=SAFE ... FAULTS=1 ...

TX: CMD SET_MODE NORMAL
RX: ACK SET_MODE REJECTED

TX: CMD CLEAR_FAULT LOW_BATTERY
RX: ACK CLEAR_FAULT OK
RX: TM ... MODE=NORMAL ... FAULTS=0 ...
```

## Extension Notes

The current protocol is text-based because it is easy to inspect in a serial
monitor, easy to log, and good for bring-up. A binary packet format with sync
bytes, payload lengths, sequence numbers, and CRC checks is a possible extension
point, but the completed OTCS demo uses the text protocol documented above.
