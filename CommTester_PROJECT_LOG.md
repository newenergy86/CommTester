# CommTester Project Log

This file records significant verified results, accepted decisions, checkpoints, and the immediate development state of CommTester.

## CHECKPOINT-19 — Modbus RTU Slave baseline

### CURRENT STATE

The compact portable CommTester is based on Arduino Nano / ATmega328P. The Modbus RTU Slave subsystem is operational and has reached a verified baseline.

### COMPLETED

The current Slave implementation successfully passed the following functional tests:

- FC01 — Read Coils
- FC02 — Read Discrete Inputs
- FC03 — Read Holding Registers, including addresses near the upper end of the configured 125-register area
- FC04 — Read Input Registers, including addresses near the upper end of the configured 125-register area
- FC05 — Write Single Coil
- FC06 — Write Single Holding Register
- FC15 — Write Multiple Coils
- FC16 — Write Multiple Holding Registers

Write operations were verified by reading the modified values back from the Slave.

CRC test:

- correct CRC frame — PASS;
- deliberately invalid CRC frame — PASS, frame rejected with no response.

RTU timing tests:

- normal frame — PASS;
- deliberate 3 ms, 5 ms and 10 ms inter-byte gaps — no response, which is the expected behavior for the performed gap test;
- t1.5 boundary experiment was performed from 1.40 ms through 1.70 ms. No response was observed in the tested boundary cases. The experiment is recorded as an observation, not as a general claim about all RTU implementations.

### DECISIONS

- Preserve the current Slave implementation as the working baseline.
- Do not modify the external `ModbusADU` library.
- Continue development with Modbus Master as the next major subsystem.
- Keep the compact tester separate from the larger diagnostic instrument planned for later.

### FACTS

The current automated test suite reports successful operation of all eight implemented Slave function codes: FC01, FC02, FC03, FC04, FC05, FC06, FC15 and FC16.

### OPEN QUESTIONS

- Final Master implementation and API.
- Master-side error handling and diagnostics.
- Resource budget for future display and statistics.

### NEXT STEP

M2 — establish and test the Modbus RTU Master baseline.

### BLOCKERS

None.

---

## MILESTONE STATUS

- M0 — Hardware base: baseline established
- M1 — Modbus RTU Slave: COMPLETED
- M2 — Modbus RTU Master: IN PROGRESS
- M3 — Bidirectional line testing: TBD
- M4 — Diagnostics and statistics: TBD
- M5 — Display: TBD
- M6 — Portable enclosure: TBD
- M7 — Final tester: TBD


---

## CHECKPOINT-20 — Firmware source synchronization

**Status: repository synchronized; hardware/build verification not performed in this checkpoint.**

The canonical firmware source files supplied in the project sources were copied to `firmware/CommTester_Bench/` on `main`.

Synchronized modules:
- `CommTester_Bench.ino`
- `indicators.cpp/.h`
- `serial_console.cpp/.h`
- `modbus_config.h`
- `modbus_master.cpp/.h`
- `modbus_rtu.cpp/.h`
- `modbus_slave.cpp/.h`
- `modbus_slave_logic.cpp/.h`

No firmware behavior was intentionally changed during synchronization. Experimental duplicate variants were not copied over the canonical files.

**Verification:** GitHub files were fetched after the writes and their lengths/SHAs were confirmed. Compilation and hardware tests remain NOT RUN.

**Next step:** establish the Master baseline using this synchronized source tree, with a compile-only check before any runtime Modbus request.


---

## CHECKPOINT-21 — Modbus Master removed from compact tester scope

### CURRENT STATE

The compact tester remains based on Arduino Nano / ATmega328P. The tested Modbus RTU Slave baseline is retained. Modbus Master has been removed from the current firmware scope to reserve resources for a small OLED display and a simple user interface.

### CHANGE

- Removed the Modbus Master include, initialization, loop branch, diagnostics, and mode transition from the main sketch.
- Reduced the indicator mode cycle to SERVICE ↔ SLAVE.
- Removed the Modbus Master source and header files from the sketch directory.
- Updated project status and next step.

### DECISION

- The compact CommTester firmware will operate in SERVICE and SLAVE modes only.
- Modbus Master is excluded from this version; this does not alter the verified Slave implementation.

### TEST

- Source-level review: PASS for removal of Master references from the main sketch and indicator mode.
- Arduino compilation and memory report: NOT RUN in this checkpoint.
- Slave hardware regression test: NOT RUN in this checkpoint.

### NEXT STEP

Compile the updated sketch with the existing Arduino Nano board and dependencies. Record Flash and global SRAM usage, then run the existing Slave regression tests.

### BLOCKERS

The exact SRAM and Flash savings are TBD until the updated firmware is compiled with the same board package and library versions as the baseline.


---

## CHECKPOINT-22 — Roadmap clarified for Nano prototype and future MCU revision

### CURRENT STATE

The active Arduino Nano / ATmega328P firmware is reduced to SERVICE and Modbus SLAVE. PING/PONG remains part of SERVICE. The Modbus Slave baseline is retained.

### FACTS

- The latest `main` source tree contains no active `modbus_master.cpp/.h` files.
- `CommTester_Bench.ino` initializes SERVICE and SLAVE and switches only between `IND_SERVICE` and `IND_SLAVE`.
- The current indicator enum contains only SERVICE and SLAVE.
- The Master implementation was removed from the active tree in commits immediately preceding this checkpoint. Git history retains the deleted source revision.
- The current source-level Master removal was recorded in CHECKPOINT-21, but Arduino compilation, memory reporting and hardware regression were explicitly NOT RUN there.

### DECISIONS

- Treat the Nano firmware as the first resource-constrained prototype, not as the final feature limit of CommTester.
- Keep the tested Slave and SERVICE/PING-PONG functionality on Nano.
- Defer Modbus Master on Nano; plan to restore Master on a future, more capable MCU revision of CommTester.
- The future MCU is TBD and will be selected against Flash, SRAM, hardware UART, display and interface requirements.
- Evaluate OLED 128×64 first using a page-buffered driver. Keep LCD 1602A as a fallback.
- A 1602A may show compact status and packet summaries across multiple pages; it cannot display a detailed frame and all decoded fields simultaneously.

### OPEN QUESTIONS

- Actual Flash and SRAM usage of the current Nano firmware.
- SRAM margin after runtime stack and display driver are included.
- Practical OLED page layout and memory usage.
- Whether a multi-page 1602A interface is sufficiently informative for field diagnostics.
- Future MCU selection.

### NEXT STEP

Compile the current Nano firmware, record Flash and global SRAM usage, then run the existing Slave regression suite. Use measured memory and test results to choose OLED or 1602A.

### TEST

- GitHub source inspection: PASS for SERVICE/SLAVE-only active firmware.
- Documentation consistency update: recorded in this checkpoint.
- Arduino compile: NOT RUN.
- Flash/SRAM measurement: NOT RUN.
- Slave hardware regression: NOT RUN.

### BLOCKERS

No confirmed technical blocker. Resource margin and runtime regression remain unverified.
