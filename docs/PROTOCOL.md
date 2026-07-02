# Protocol

## Purpose

This document will define the OTCS telemetry, command, acknowledgement, and error formats.

The protocol design should reflect the same engineering priorities as the codebase:

* clear ownership of responsibilities
* stable message formats
* explicit validation rules
* deterministic parsing behavior
* easy logging and diagnostics
* separation between protocol definition and transport mechanism

## Planned Text Protocol

Telemetry example:

```text
TM SAT=1 TIME=12345 MODE=NORMAL TEMP=72 BAT=95 FAULTS=0
```

Command example:

```text
CMD SET_MODE SAFE
```

Acknowledgement example:

```text
ACK SET_MODE OK
```

## Planned Sections

* Message types
* Required fields
* Field encoding
* Mode enumeration
* Fault enumeration
* Validation rules
* Error responses
* Binary packet migration plan
* Versioning strategy
* Host/firmware compatibility rules
