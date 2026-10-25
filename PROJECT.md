# CommTester

## Project purpose

CommTester is a compact portable Modbus/RS-232 line tester based on Arduino Nano.

The device is intended to be useful as a practical field diagnostic tool and as a development/test platform for the larger diagnostic instrument being developed separately.

The compact tester and the future main diagnostic instrument are separate projects and must not be conflated in architectural decisions.

## Current architecture

- MCU: Arduino Nano / ATmega328P
- Serial communication: current implementation uses the project's `ModbusRTU` transport layer and an external `ModbusADU` dependency.
- Modbus Slave: implemented and tested.
- Modbus Master: development in progress.
- Indicators: LED and sound indication are implemented and being tested.
- Display: planned; hardware selection and integration are TBD.

## Modbus Slave status

### FACT

The following RTU functions have been tested successfully against the current Slave implementation:

- FC01 — Read Coils
- FC02 — Read Discrete Inputs
- FC03 — Read Holding Registers
- FC04 — Read Input Registers
- FC05 — Write Single Coil
- FC06 — Write Single Holding Register
- FC15 — Write Multiple Coils
- FC16 — Write Multiple Holding Registers

CRC handling has also been tested:

- valid CRC frame is accepted and answered;
- invalid CRC frame is rejected without a response.

RTU inter-byte timing behavior has been tested with normal frames and deliberate gaps. The exact boundary behavior is recorded in the project log and must not be inferred beyond the performed tests.

## Milestones

- `M0` — Hardware base — completed baseline
- `M1` — Modbus RTU Slave — completed and tested
- `M2` — Modbus RTU Master — current milestone
- `M3` — Bidirectional line testing
- `M4` — Diagnostics and statistics
- `M5` — Display
- `M6` — Portable enclosure
- `M7` — Final tester

## Development method

The project follows `DEVELOPMENT_RULES.md`.

In particular:

1. GitHub is the authoritative source for current project state and working code.
2. Significant decisions are recorded immediately.
3. Project information is classified as `FACT`, `DECISION`, `ASSUMPTION`, `TBD`, or `REJECTED`.
4. Existing open-source libraries, protocol implementations, reference designs, and industrial solutions are considered before writing custom functionality.
5. Work proceeds by Milestone with explicit completion criteria.
6. Unknown behavior should be experimentally verified before architecture is based on it.
7. Major independent layers are changed one at a time.
8. Known-good baselines are preserved before substantial experiments.
9. Significant stages end with a checkpoint containing current state, completed work, decisions, facts, open questions, next step, and blockers.

## Current development state

### CURRENT STATE

M1 is complete. The Modbus Slave has passed the current functional, CRC, and timing test set.

M2 — Modbus RTU Master is the active development target.

### COMPLETED

- Compact Arduino Nano tester architecture established.
- Modbus Slave implementation completed.
- FC01/02/03/04/05/06/15/16 Slave tests passed.
- CRC acceptance/rejection test passed.
- Inter-byte gap and t1.5 boundary tests performed.
- Development rules established in `DEVELOPMENT_RULES.md`.

### DECISIONS

- The compact tester remains based on Arduino Nano / ATmega328P.
- The external `ModbusADU` library is not modified.
- Project-specific Modbus functionality is developed in the CommTester codebase where required.
- Modbus Master is developed as the next major subsystem.

### OPEN QUESTIONS

- Final Modbus Master API and implementation details.
- Master error handling and diagnostic reporting.
- Final display choice and whether the ATmega328P has sufficient resources for the desired UI/statistics.
- Final enclosure dimensions.

### NEXT STEP

Complete and test the Modbus RTU Master baseline before adding display functionality or other major subsystems.

### BLOCKERS

No current blocker for M2 development.
