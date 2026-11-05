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
- Modbus: Slave only; the Nano firmware starts directly in this mode.
- RS-232 transport: `SoftwareSerial` on D10/D11, isolated in the project-owned `rs232_port` module.
- Indicators: RX/TX/ERROR LEDs and buzzer. SERVICE mode indication and old button polling have been removed; D7 is reserved for the upcoming encoder button.
- Display: SSD1306 OLED 128×64, U8X8 text-only mode from the existing U8g2 package over hardware I²C (A4/SDA, A5/SCL). Only the Modbus Slave status screen remains. Compilation and hardware regression after SERVICE removal are pending.

### Current operating mode

- `SLAVE` — the only active Nano mode; the firmware starts directly and continuously in Modbus Slave.

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
- The Slave screen shows the local Slave ID, the destination ID from the latest valid RTU frame (labelled `MST`), the last function code, RX/TX counters on one row, response rate, missed responses, and current serial settings (`9600 8N1`). The two addresses share one row as `SLV:01 MST:01` (three-digit addresses are also supported); RX/TX counters share another row. Counters remain 32-bit; RX/TX values above 9999 use compact k/M/G notation to fit on one row, while MISS is displayed as a full decimal value. The previously reported statistics build compiled at Flash 17,706 bytes and global SRAM 1,568 bytes. The SERVICE-free source revision has not yet been compiled.

The LCD 1602A remains a fallback only if the OLED cannot meet the Nano's memory/runtime requirements.

## Milestones

- `M0` — Hardware base — completed baseline
- `M1` — Modbus RTU Slave on Nano — completed and tested
- `M2` — Modbus RTU Master — deferred to future MCU revision
- `M3` — Bidirectional line testing — future full-featured revision
- `M4` — Diagnostics and statistics — IN PROGRESS (traffic statistics compile PASS; updated screen build pending)
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

The Nano firmware source has been simplified to a single Modbus SLAVE operating mode. SERVICE command handling and mode switching have been removed. The Modbus Slave processing implementation is retained; its RS-232 dependency has been separated into `rs232_port`.

### COMPLETED

- Nano prototype architecture established.
- Modbus Slave implementation completed.
- FC01/02/03/04/05/06/15/16 Slave tests passed.
- CRC acceptance/rejection test passed.
- Inter-byte gap and t1.5 boundary tests performed.
- Master removed from the active Nano firmware scope.
- SERVICE mode and its command parser removed from the active Nano firmware; direct Slave startup implemented in source.

### DECISIONS

- Keep the Nano prototype and its verified Slave implementation.
- Keep the Nano prototype Slave-only; SERVICE remains in the long-term feature scope for a future MCU revision.
- Defer Modbus Master on Nano; restore it for the future upgraded-MCU revision.
- Do not modify the external `ModbusADU` library.
- Use SSD1306 OLED 128×64 with U8X8 text-only mode from the existing U8g2 package over hardware I²C; retain LCD 1602A only as a fallback.
- Do not choose the future MCU until requirements and resource budget are clearer.

### OPEN QUESTIONS

- Actual Flash/SRAM usage after SERVICE removal.
- Compile the current SERVICE-free source and compare its memory report with the prior 17,706-byte Flash / 1,568-byte SRAM report (toolchain and library versions must match).
- Verify the Slave-only OLED screen on hardware.
- Run the full Modbus Slave regression after SERVICE removal, including all eight function codes and invalid-CRC rejection/recovery.
- Encoder hardware and input handling are not implemented yet.
- Future MCU selection.

### NEXT STEP

Compile the SERVICE-free Nano firmware and record Flash/SRAM usage. If compilation passes, upload and verify direct Slave startup, OLED status display, and the existing Modbus regression suite. Only after this baseline is accepted, implement encoder input and runtime settings.

### BLOCKERS

The SERVICE-free source has not yet been compiled or tested on hardware. The next gate is compile and memory measurement, followed by OLED and Modbus regression.
