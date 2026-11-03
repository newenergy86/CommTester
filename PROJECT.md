# CommTester

## Project purpose

CommTester is a portable diagnostic tester for Modbus RTU and serial communication lines.

Development is staged:

- **Nano prototype:** Arduino Nano / ATmega328P. It preserves and validates the core hardware and protocol functionality within strict Flash/SRAM limits.
- **Future full-featured hardware revision:** replace the Nano with a more capable MCU. This is an evolution of the same CommTester project, not a reason to discard the Nano firmware or its verified protocol work.

The Nano prototype is intentionally a reduced implementation. Features excluded because of its resource limits remain part of the long-term CommTester scope.

## Current Nano prototype architecture

- MCU: Arduino Nano / ATmega328P.
- Physical interface: RS-232 through MAX238.
- Serial transport: project-specific `ModbusRTU` layer and external `ModbusADU` dependency.
- SERVICE: PING/PONG and diagnostic text commands.
- Modbus: Slave only.
- Indicators: RX/TX/ERROR/MODE LEDs, buzzer and mode button.
- Display: SSD1306 OLED 128×64, U8X8 text-only mode from the existing U8g2 package over hardware I²C (A4/SDA, A5/SCL). The user has flashed this build and reports that it currently operates without visible failures; a complete, measured hardware regression and worst-case SRAM assessment have not been recorded.

### Current operating modes

- `SERVICE` — serial diagnostics, including PING → PONG.
- `SLAVE` — responds to external Modbus RTU Master requests.

### Features deferred on Nano

- Modbus Master.
- Simultaneous or combined Master/Slave line testing.
- More extensive packet and diagnostic presentation.

These features are not rejected. They are deferred to a future MCU revision because the ATmega328P has limited Flash and 2 KB SRAM, and the current prototype also needs display/UI resources.

## Long-term feature scope

The future full-featured CommTester hardware revision is intended to support:

- Modbus RTU Master;
- Modbus RTU Slave;
- SERVICE and PING/PONG;
- line diagnostics and traffic statistics;
- packet information and display UI;
- further diagnostic functions as hardware permits.

The future MCU is **TBD**. Selection criteria include sufficient Flash and SRAM, suitable hardware UART resources, display support, and compatibility with the planned RS-232/RS-485 interfaces.

**Important:** keep the current Nano implementation working and preserve its verified protocol modules. Master code removed from the active Nano sketch remains available in Git history and can be recovered when the full-featured hardware revision is started.

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

## Display

### Selected hardware and driver

- OLED: SSD1306, 128×64.
- Library: U8g2 by olikraus, installed through Arduino IDE Library Manager.
- Constructor: `U8X8_SSD1306_128X64_NONAME_HW_I2C` (text-only mode).
- Interface: hardware I²C; Nano SDA=A4 and SCL=A5.
- The standalone test sketch compiled and the user confirmed that the physical display starts and renders without visible problems. The integrated U8X8 text-mode firmware has also been flashed; the user reports no visible failures so far, but a complete display-enabled regression is not recorded.
- The SERVICE screen retains its existing status information. The SLAVE screen now has source-level implementation for request count, sent-response count, response rate and missed-response count. Counters are 32-bit; the screen uses ten-digit fields. This statistics implementation has not yet been compiled or hardware-tested. The prior U8X8 build report was 1,508 bytes global SRAM and 540 bytes compiler-reported remaining; that report predates the statistics changes. The compiler-reported remainder is not a worst-case runtime stack measurement.

The LCD 1602A remains a fallback only if the OLED cannot meet the Nano's memory/runtime requirements.

## Milestones

- `M0` — Hardware base — completed baseline
- `M1` — Modbus RTU Slave on Nano — completed and tested
- `M2` — Modbus RTU Master — deferred to future MCU revision
- `M3` — Bidirectional line testing — future full-featured revision
- `M4` — Diagnostics and statistics — IN PROGRESS (Slave traffic counters implemented in source; verification pending)
- `M5` — Display — IN PROGRESS
- `M6` — Portable enclosure
- `M7` — Full-featured CommTester hardware/firmware revision

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

The Nano firmware source has been reduced to SERVICE and SLAVE modes. The tested Slave implementation is retained. Modbus Master source files have been removed from the active sketch directory.

### COMPLETED

- Nano prototype architecture established.
- Modbus Slave implementation completed.
- FC01/02/03/04/05/06/15/16 Slave tests passed.
- CRC acceptance/rejection test passed.
- Inter-byte gap and t1.5 boundary tests performed.
- Master removed from the active Nano firmware scope.
- SERVICE ↔ SLAVE mode cycle retained.

### DECISIONS

- Keep the Nano prototype and its verified Slave implementation.
- Keep SERVICE with PING/PONG.
- Defer Modbus Master on Nano; restore it for the future upgraded-MCU revision.
- Do not modify the external `ModbusADU` library.
- Use SSD1306 OLED 128×64 with U8X8 text-only mode from the existing U8g2 package over hardware I²C; retain LCD 1602A only as a fallback.
- Do not choose the future MCU until requirements and resource budget are clearer.

### OPEN QUESTIONS

- Actual Flash/SRAM usage after removing Master.
- Remaining SRAM margin for the display and runtime stack.
- Record a complete functional regression with the OLED-enabled U8X8 firmware.
- Worst-case runtime stack margin for the U8X8 firmware remains unmeasured.
- Compile and measure the updated firmware with the traffic-statistics display.
- Run the statistics acceptance test, including foreign IDs, Broadcast and invalid CRC.
- Future MCU selection.

### NEXT STEP

Compile the current GitHub source for Arduino Nano and report Flash/SRAM usage and any errors. Do not flash the statistics build until compilation and memory usage have been reviewed.

### BLOCKERS

No confirmed blocker. The statistics source changes require compile and hardware verification before acceptance.
