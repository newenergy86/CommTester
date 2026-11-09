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
- Encoder input: KY-040 on CLK=D8, DT=D9, SW=D7. Rotation is decoded in polling mode by the external KY040 library (codingABI, v1.0.3); the button uses a project-owned 30 ms debounce. Direction and button transitions are reported to USB Serial at 115200 baud. Rotation and button-press sounds are routed through the shared Indicators event queue. The user has verified exactly 20 CW and 20 CCW events for 20 detents in each direction, with correct operation during active Modbus traffic (CHECKPOINT-49B). Button press/release transitions and sound feedback are hardware-verified. The user has subsequently reported successful completion of the full requested Modbus function, OLED and indicator regression after the encoder sound change (CHECKPOINT-50). Encoder events now feed an inline editor on the main OLED screen; this UI revision is committed but its build and hardware acceptance are pending.
- Indicators: RX/TX/ERROR LEDs, D5 HEARTBEAT LED and buzzer. D5 startup triple-flash with a three-note melody and a 2-second heartbeat are implemented in source and compile successfully for Nano; hardware verification is pending. SERVICE mode indication and old button polling have been removed; D7 is reserved for the upcoming encoder button.
- Display: SSD1306 OLED 128×64, U8X8 text-only mode from the existing U8g2 package over hardware I²C (A4/SDA, A5/SCL). Only the Modbus Slave status screen remains; its second row is used for inline encoder selection/edit feedback without opening a separate menu. Slave ID and baud rate are editable from the main screen. The display module redraws only rows whose values changed during normal updates; full-screen clearing is reserved for initialization and wake from OLED power-save. After wake, the full redraw is deferred to the next `Display::update()` call so it uses the latest post-frame statistics rather than stale cached values. The eight-row layout and 120-second inactivity power-save/wake behavior have passed user-reported compile and hardware verification (CHECKPOINT-46).

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
- The standalone test sketch compiled and the user confirmed that the physical display starts and renders without visible problems. The integrated U8X8 text-mode firmware has been flashed and the revised layout, sleep/wake behavior, current-statistics redraw, normal Modbus responses and flicker-free updates passed user-reported hardware verification (CHECKPOINT-46).
- The Slave screen shows the local Slave ID, destination ID from the latest valid RTU frame (`MST`), last function code, RX/TX counters, response rate, and combined ERR/MISS counters. RX counts every completed receive event, including bad-CRC and malformed RTU/PDU frames; TX counts responses successfully handed to the serial transport; RATE is TX/RX. ERR counts RTU CRC/framing errors and malformed request lengths. MISS counts valid, structurally complete requests that this Slave intentionally ignores because their destination ID is not its own (including broadcast ID 0). Malformed foreign-address frames count as ERR, not MISS. Unsupported function codes and valid requests producing Modbus exception responses are not counted as ERR. Both ERR and MISS start at zero and are displayed together on OLED row 7 (the seventh text row after the header and blank spacer). RX/TX/ERR/MISS values use compact k/M/G notation when needed. The previous compile reports predate the current statistics changes and must not be treated as measurements of this revision.

The LCD 1602A remains a fallback only if the OLED cannot meet the Nano's memory/runtime requirements.

## Milestones

- `M0` — Hardware base — completed baseline
- `M1` — Modbus RTU Slave on Nano — completed and tested
- `M2` — Modbus RTU Master — deferred to future MCU revision
- `M3` — Bidirectional line testing — future full-featured revision
- `M4` — Diagnostics and statistics — IN PROGRESS (traffic statistics compile PASS; updated screen build pending)
- `M5` — Display — COMPLETED (CHECKPOINT-46); inline settings editor implementation pending hardware acceptance
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

The Nano firmware is Slave-only. The current source revision tracks RX/TX/ERR/MISS, computes RATE as TX/RX, rejects malformed request lengths before PDU processing, and preserves received frame length on RTU CRC/framing errors for diagnostics. MISS counts valid requests ignored due to a foreign/broadcast destination ID. The RATE digit formatting, combined ERR/MISS row, revised eight-row layout and 120-second OLED inactivity timeout are committed. OLED wake redraw uses the latest statistics snapshot. The user has confirmed successful compilation and all five CHECKPOINT-45 hardware acceptance checks (CHECKPOINT-46).

### COMPLETED

- Nano prototype architecture established.
- Modbus Slave implementation completed.
- FC01/02/03/04/05/06/15/16 Slave tests passed.
- CRC acceptance/rejection test passed.
- Inter-byte gap and t1.5 boundary tests performed.
- Master removed from the active Nano firmware scope.
- SERVICE mode and its command parser removed from the active Nano firmware; direct Slave startup implemented in source.
- SERVICE-free Nano firmware compilation passed; Flash/SRAM usage recorded in CHECKPOINT-38.
- Complete reported Modbus function regression passed after SERVICE removal; see CHECKPOINT-39.
- Incremental OLED row refresh implemented in source; user confirmed that the screen no longer flickers.
- RATE percent-sign formatting, eight-row OLED layout and 120-second OLED power-save/wake behavior compiled and passed the user-reported hardware acceptance checks in CHECKPOINT-46.
- RX/TX/ERR statistics semantics and malformed-PDU length rejection implemented in source; verification pending in CHECKPOINT-42.

### DECISIONS

- Keep the Nano prototype and its verified Slave implementation.
- Keep the Nano prototype Slave-only; SERVICE remains in the long-term feature scope for a future MCU revision.
- Defer Modbus Master on Nano; restore it for the future upgraded-MCU revision.
- Do not modify the external `ModbusADU` library.
- Count every completed receive event in RX; count only transmitted responses in TX; calculate RATE as TX/RX.
- Count CRC, RTU framing and malformed supported-function request lengths in ERR. Count valid structurally complete requests ignored because of a foreign/broadcast destination ID in MISS.
- Use SSD1306 OLED 128×64 with U8X8 text-only mode from the existing U8g2 package over hardware I²C; retain LCD 1602A only as a fallback.
- Do not choose the future MCU until requirements and resource budget are clearer.

### OPEN QUESTIONS

- Verify the inline main-screen settings editor: Slave ID (1–247), baud (1200/2400/4800/9600/19200), persistent EEPROM storage, and dynamic RTU timing. Source is committed; compile, memory measurement, save/reboot and communication tests are pending.
- Confirm the KY040 1.0.3 library is installed in the Arduino build environment.

- Confirm whether the previous and current compile reports used identical toolchain and library versions before treating the memory delta as a strict apples-to-apples comparison.
- Review `modbus_error_test.py` test 05 and decide whether a valid-CRC malformed FC03 request should be discarded or answered with exception 03.
- Run `modbus_test_all.py` after the latest statistics and PDU-length validation changes.
- Verify bad CRC and malformed request accounting, foreign-ID MISS accounting, and recovery after errors.
- Confirm RX/TX/ERROR indicator behavior.
- Encoder input test and queued sound feedback are implemented in source. The sound-feedback revision compiled at 20,532 bytes Flash (66%), 1,506 bytes global SRAM (73%), 542 bytes compiler-reported remaining. The user confirmed that the sound feedback works. Exact detent counts, button debounce acceptance, and full Modbus/OLED/indicator regression after this change remain to be verified.
- D5 startup melody, three synchronized flashes, periodic HEARTBEAT and sound arbitration have passed user-reported hardware verification (CHECKPOINT-47D). Current PROGMEM-optimized build: 19,686 bytes Flash (64%), 1,500 bytes global SRAM (73%), 548 bytes remaining. Complete Modbus regression and repeated traffic passed without hangs (CHECKPOINT-47C). The original cause of the earlier hang remains unproven.
- Future MCU selection.

### NEXT STEP

Compile the inline settings revision for Arduino Nano / ATmega328P, record Flash/SRAM, then verify encoder editing, EEPROM persistence, dynamic baud switching and the full Modbus/OLED/indicator regression. Preserve the existing main-screen diagnostics.

### BLOCKERS

SERVICE-free compilation and the reported FC01/02/03/04/05/06/15/16 regression passed. The user confirmed that incremental OLED row refresh removed the observed flicker. The revised OLED layout, 120-second sleep/wake and normal Modbus operation have passed user-reported compile and hardware verification. D5 startup melody, three flashes, periodic HEARTBEAT and sound arbitration are hardware-verified (CHECKPOINT-47D). CHECKPOINT-47B PROGMEM optimization compiles at 19,686 bytes Flash / 1,500 bytes global SRAM / 548 bytes remaining; complete Modbus regression and repeated traffic passed without hangs (CHECKPOINT-47C). Encoder sound feedback and exact tested detent counts are hardware-verified. The user reports that the full post-encoder Modbus/OLED/indicator regression passed (CHECKPOINT-50). The inline settings editor source has now been committed; it has not yet been compiled or hardware-tested. Invalid-CRC recovery and RX/TX/ERROR indication after SERVICE removal remain unconfirmed. The previous accepted build reported 542 bytes of compiler-reported free SRAM; remeasure after the UI changes.
