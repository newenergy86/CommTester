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
---

## CHECKPOINT-23 — SERVICE console SRAM optimization

### CHANGE

- Replaced the dynamic String command buffer with a fixed 33-byte character buffer (32 command characters plus null terminator).
- Changed command retrieval to return a pointer to the internal buffer; removed the temporary String copy.
- Preserved trimming of leading/trailing whitespace and case-insensitive command handling by converting ASCII lowercase characters to uppercase.
- Moved fixed diagnostic and response string literals in the main sketch to program memory using the Arduino F() macro.
- Updated the console print interface to accept flash-resident strings.

### FACTS

The user compiled the updated firmware for Arduino Nano and reported:

- Flash: 11,032 bytes (35% of 30,720 bytes).
- Global SRAM: 1,148 bytes (56% of 2,048 bytes).
- Remaining SRAM reported by the compiler: 900 bytes.

Compared with the immediately preceding build report:

- Flash decreased from 11,952 to 11,032 bytes: 920 bytes saved.
- Global SRAM decreased from 1,398 to 1,148 bytes: 250 bytes saved.
- Compiler-reported remaining SRAM increased from 650 to 900 bytes.

The user reports that SERVICE command operation appears functional after the change. PING returns PONG and RX/TX LED indications are observed. SERVICE command checks were reported informally; a complete regression record is not yet available.

### SOUND OBSERVATION

The user reports that the piezo produces the RX tone consistently, while the TX tone is intermittent during PING.

Source inspection identifies a likely software interaction, not yet confirmed by a dedicated timing experiment:

- PING queues RX and TX indication events and also calls the separate Indicators::beep(1).
- The event queue and the standalone beep state machine both control the same buzzer pin.
- updateEvents() runs before updateBeep() in Indicators::update(). The standalone beep may therefore call noTone() during an RX/TX event and truncate its tone.

This is recorded as a source-level hypothesis pending a focused test. No indicator code was changed in this checkpoint.

### DECISIONS

- Keep the fixed-buffer SERVICE console implementation as the new candidate baseline.
- Do not change the indicator sound logic until the RX/TX/beep interaction is isolated and tested.
- Record configurable RS-232 parameters and configurable Slave ID as future SERVICE-console requirements; implementation is deferred.
- Do not reduce Modbus data map sizes or alter external libraries as part of this optimization.

### TEST

- Arduino Nano compile: PASS, according to the user's build report.
- SERVICE PING/PONG and LED indication: reported working.
- SERVICE HELP, INFO, MODE and whitespace/case variations: not individually confirmed in the latest report.
- Full Modbus Slave hardware regression after this console change: NOT RUN / NOT CONFIRMED.
- Sound overlap diagnosis: source-level hypothesis; dedicated test NOT RUN.

### NEXT STEP

Treat the SRAM optimization as successful at compile level. Before further memory changes, perform the Modbus Slave regression and isolate the SERVICE buzzer overlap with a minimal test. Keep the reported 1,148-byte global SRAM usage as the new build baseline.


---

## CHECKPOINT-24 — SERVICE PING sound overlap resolved

### CHANGE

- Removed the redundant `Indicators::beep(1)` call from the SERVICE PING handler.
- PING now relies on the existing RX and TX events in the shared event queue.
- No changes were made to `indicators.cpp`, `indicators.h`, the event queue, or external libraries.

### FACTS

- The user flashed the updated firmware and confirmed that the PING sound problem is resolved.
- The change was committed as `30fa7f49ce6ebc62c45e03c34755fbc08ea0c70e`.
- The updated main sketch was fetched again from GitHub; the redundant beep call is absent from the PING handler.

### TEST

- SERVICE PING sound indication: PASS, according to the user's hardware confirmation.
- PONG response and RX/TX LED behavior: no regression was reported during the confirmation.
- Unknown-command indication and mode-switch beep: not separately re-tested in this checkpoint.
- Full Modbus Slave hardware regression after the SERVICE console SRAM optimization: NOT RUN / NOT CONFIRMED.

### DECISION

- Keep the minimal PING change.
- Do not refactor the shared indicator state machine; the focused fix resolved the observed problem.
- Preserve the 1,148-byte global SRAM build report from CHECKPOINT-23 as the current measured memory baseline.

### NEXT STEP

Run the Modbus Slave hardware regression on the current firmware, covering FC01, FC02, FC03, FC04, FC05, FC06, FC15 and FC16, including read-back verification for writes and CRC rejection. Record any deviations before making further code changes.


---

## CHECKPOINT-25 — Modbus Slave read-function regression

### TEST

The user reports successful hardware tests of all four read functions on the current firmware:

- FC01 — Read Coils: PASS
- FC02 — Read Discrete Inputs: PASS
- FC03 — Read Holding Registers: PASS
- FC04 — Read Input Registers: PASS

The tests were performed using QModMaster with the CommTester operating as Slave. The test parameters were Slave ID 1, 9600 baud, 8N1, and zero-based start address 0. The user confirmed that all four tables work.

### SCOPE

This confirms the four read paths on the current firmware after the SERVICE console SRAM optimization and PING sound fix. It does not yet confirm write functions, invalid-CRC rejection on the current firmware, or the complete regression suite.

### NEXT STEP

Test write functions FC05, FC06, FC15 and FC16, then read back the written values. After that, test invalid CRC rejection and recovery.


---

## CHECKPOINT-26 — Complete Modbus RTU function regression

### TEST

The user ran the existing `tests/modbus_test_all.py` against the current CommTester firmware.

Test configuration:
- Serial port: COM7
- Baud rate: 9600
- Format: 8N1
- Slave ID: 1

All 10 test cases reported PASS:

- FC01 — Read Coils: PASS
- FC02 — Read Discrete Inputs: PASS
- FC03 — Read Holding Registers at addresses 0 and 120: PASS
- FC04 — Read Input Registers at addresses 0 and 120: PASS
- FC05 — Write Single Coil and read-back: PASS
- FC06 — Write Single Holding Register and read-back: PASS
- FC15 — Write Multiple Coils (16 bits) and read-back: PASS
- FC16 — Write Multiple Holding Registers (5 registers) and read-back: PASS

The test output reports `OVERALL RESULT: PASS`.

Read-back values matched the expected values for all tested write operations. Reads at address 120 returned the expected final five values in both register tables. The script also validated CRCs of received responses.

### FACTS

- This is a user-reported hardware test of the current firmware, not merely a source-level review.
- All eight implemented Modbus Slave function codes are confirmed by the current run.
- The complete function regression also confirms the tested read/write paths and their read-back behavior.

### LIMITATIONS

- This script validates CRCs on received responses; it does not test rejection of a request carrying an intentionally invalid CRC.
- Recovery after an invalid-CRC request is therefore NOT CONFIRMED by this checkpoint.
- RTU inter-byte gap behavior is not covered by this complete function test.

### DECISION

- Retain the current Modbus Slave implementation as the verified functional baseline.
- Do not modify firmware after this successful regression without a separately identified defect or requirement.

### NEXT STEP

Run the dedicated invalid-CRC test and verify that the Slave rejects the malformed request without responding, then confirm that a subsequent valid request is processed normally.


---

## CHECKPOINT-27 — Invalid CRC rejection and Slave recovery

### TEST

The user ran the updated `tests/modbus_test_crc.py` against the current CommTester firmware.

Test configuration:
- Serial port: COM7
- Baud rate: 9600
- Format: 8N1
- Slave ID: 1

All three test stages passed:

1. Valid FC03 request before the malformed frame: PASS.
   - Slave returned a structurally valid FC03 response.
   - Response CRC was valid.
   - Holding register values returned were [12345, 200, 300]. The first register retained the value written by the preceding FC06 function test; the test intentionally does not assume factory-initialized register contents.
2. FC03 request with deliberately corrupted CRC: PASS.
   - Slave returned no data.
3. Valid FC03 request after the malformed frame: PASS.
   - Slave returned a structurally valid FC03 response with valid CRC.
   - Response data before and after the invalid-CRC request was unchanged.

The script reported:
- Valid request before error: PASS
- Invalid CRC rejected: PASS
- Valid request after error: PASS
- OVERALL RESULT: PASS

### FACTS

- The current hardware test confirms rejection of the tested invalid-CRC request without a response.
- A subsequent valid request is processed successfully.
- The FC03 response payload remained unchanged across the invalid-CRC request.
- No firmware changes were required for this test.

### DECISION

- Keep the current Modbus Slave firmware as the verified functional baseline.
- Do not modify firmware in response to the earlier apparent FAIL: it was caused by a stale/local test-script version with an incorrect fixed expectation for Holding Register 0.
- The corrected test compares the valid response before and after the malformed frame and validates response structure and CRC.

### LIMITATIONS

- This checkpoint covers the tested invalid-CRC FC03 request and immediate valid-request recovery. It is not an exhaustive test of every malformed-frame type or every possible RTU timing boundary.

### NEXT STEP

M1 Modbus RTU Slave functional regression is complete for the currently defined test scope. Proceed to the next planned project task without changing the verified Slave baseline.


---

## CHECKPOINT-28 — Modbus Slave diagnostic strings moved to Flash; post-change regression PASS

### CHANGE

- Replaced fixed diagnostic string literals in `firmware/CommTester_Bench/modbus_slave.cpp` with Arduino `F()` flash-string wrappers.
- No Modbus processing logic, register/coil maps, transport behavior, external libraries, or protocol parameters were changed.
- Firmware source commit: `90d47359041d7ca7f2ecb1b8bfd9c115bd6ae41f`.

### MEMORY FACTS

The user compiled the updated firmware for Arduino Nano and reported:

- Flash: 11,116 bytes (36% of 30,720 bytes).
- Global SRAM: 990 bytes (48% of 2,048 bytes).
- Compiler-reported remaining SRAM: 1,058 bytes.

Compared with the CHECKPOINT-23 build report (11,032 bytes Flash; 1,148 bytes global SRAM; 900 bytes remaining):

- Global SRAM decreased by a further 158 bytes.
- Compiler-reported remaining SRAM increased by 158 bytes.
- Flash usage increased by 84 bytes.

Compared with the pre-CHECKPOINT-23 report (1,398 bytes global SRAM), the current build uses 408 fewer bytes of global SRAM. The compiler-reported remaining SRAM is not a measurement of worst-case runtime stack margin.

### TEST

The user confirmed SERVICE operation after the update.

After resetting the Nano to restore the initial test maps, the user ran:

- `tests/modbus_test_all.py`: OVERALL RESULT PASS.
  - FC01 — Read Coils: PASS
  - FC02 — Read Discrete Inputs: PASS
  - FC03 — Read Holding Registers at addresses 0 and 120: PASS
  - FC04 — Read Input Registers at addresses 0 and 120: PASS
  - FC05 — Write Single Coil and read-back: PASS
  - FC06 — Write Single Holding Register and read-back: PASS
  - FC15 — Write Multiple Coils and read-back: PASS
  - FC16 — Write Multiple Holding Registers and read-back: PASS
- `tests/modbus_test_crc.py`: OVERALL RESULT PASS.
  - Valid FC03 request before invalid-CRC frame: PASS
  - Invalid-CRC request rejected with no response: PASS
  - Subsequent valid FC03 request processed successfully: PASS
  - Register data unchanged across invalid-CRC request: PASS

The first complete-function run before resetting the Nano reported mismatches in FC01 and FC03 because the RAM maps retained values written by earlier tests (FC15 changed the coils; FC06 changed Holding Register 0). After reset, initial values were restored and the complete suite passed. This was test-state persistence, not a firmware regression.

### FACTS

- The post-optimization build compiles successfully.
- SERVICE operation was confirmed by the user.
- All eight implemented Slave function codes passed the complete hardware regression after reset.
- The dedicated invalid-CRC rejection and recovery test passed.
- No protocol or functional code changes were needed.

### DECISION

- Retain this firmware as the current verified Nano SERVICE/SLAVE baseline.
- Do not perform further speculative optimization or alter the working Modbus implementation.
- Evaluate the OLED integration using a page-buffered driver and measure the complete build's SRAM usage and runtime margin.

### LIMITATIONS

- The reported 1,058 bytes is the compiler's remaining SRAM after global allocations; actual stack headroom under worst-case runtime conditions has not yet been measured.
- OLED integration and regression with the display enabled have not yet been performed.

### NEXT STEP

Begin the OLED integration feasibility step: identify a suitable page-buffered SSD1306 driver and determine its actual SRAM footprint before integrating it into the firmware.


---

## CHECKPOINT-29 — SSD1306 OLED status-screen integration (verification pending)

### FACTS

- The user compiled the standalone U8g2 SSD1306 128×64 test sketch for Arduino Nano:
  - Flash: 11,060 bytes (36%).
  - Global SRAM: 710 bytes (34%).
  - Compiler-reported remaining SRAM: 1,338 bytes.
- The user connected the OLED and confirmed that it starts and renders without visible problems.
- The standalone test uses the U8g2 page-buffer constructor `U8G2_SSD1306_128X64_NONAME_1_HW_I2C`.

### CHANGE

- Added `firmware/CommTester_Bench/display.h` and `display.cpp`.
- Added the display module to the main sketch.
- Initialized the display during setup and update the screen only when the operating mode changes.
- The initial status screen shows:
  - CommTester heading;
  - SERVICE or SLAVE mode;
  - RS-232 9600 8N1;
  - Slave ID 1;
  - button mode-switch hint.
- Uses U8g2 `_1_` page buffer and hardware I²C (Nano A4/SDA, A5/SCL).
- No changes were made to Modbus transport, Slave processing, SERVICE command handling, indicator logic, or external libraries.

### SOURCE COMMITS

- `display.h`: `97f94e3d362e464f3554b88991ed2bbce66c24f4`
- `display.cpp`: `9b64133c81da4a62879a87c94b391fce059d3b28`
- Main sketch integration: `fffe34ddd587888710d39cfd227aa53a8acbcd25`
- Project status update: `1f70c45f584e47bc9408c7fe0c37e762511e9943`

### TEST

- Standalone OLED compile: PASS, user-reported.
- Standalone physical OLED startup/rendering: PASS, user-reported.
- Complete CommTester firmware compile with U8g2: NOT YET RUN.
- Integrated Flash/SRAM measurement: NOT YET RUN.
- SERVICE regression with OLED enabled: NOT YET RUN.
- Modbus Slave regression with OLED enabled: NOT YET RUN.

### DECISION

- Keep the selected SSD1306 + U8g2 `_1_` implementation as the integration candidate.
- The first integration is deliberately limited to a mode/status screen. Packet decoding and detailed frame presentation are deferred until the display-enabled firmware passes compile and regression checks.
- Do not change the existing Modbus or SERVICE implementation as part of this step.

### NEXT STEP

Compile `firmware/CommTester_Bench/CommTester_Bench.ino` for Arduino Nano with U8g2 installed. Record Flash and global SRAM usage and report any compiler errors. Do not proceed to protocol/UI expansion until the integrated build is verified.


---

## CHECKPOINT-30 — Integrated U8g2 build: SRAM margin insufficient

### TEST

The user compiled the complete OLED-integrated CommTester firmware for Arduino Nano and reported:

- Flash: 21,154 bytes (68%).
- Global SRAM: 1,726 bytes (84%).
- Compiler-reported remaining SRAM: 322 bytes.

Compared with the pre-OLED baseline from CHECKPOINT-28:

- Flash increased from 11,116 to 21,154 bytes (+10,038 bytes).
- Global SRAM increased from 990 to 1,726 bytes (+736 bytes).
- Compiler-reported remaining SRAM decreased from 1,058 to 322 bytes.

The standalone OLED test previously reported 710 bytes of global SRAM and 1,338 bytes remaining. The complete firmware's additional memory usage reflects the combined firmware and display/library integration; it must not be attributed solely to the 128-byte page buffer.

### FACTS

- The integrated firmware compiles.
- The 322-byte compiler-reported remainder is not a runtime stack measurement and is too small to accept as a safe operating margin for this application.
- No display-enabled firmware flash or runtime regression has been performed.
- The existing pre-OLED SERVICE/SLAVE baseline remains the last verified firmware baseline.

### DECISION

- Do not flash or accept the current U8g2 graphics-mode integration as a working firmware baseline.
- Preserve the current source as an experimental checkpoint; do not alter the verified Modbus/SERVICE logic.
- Before considering a different external library, test the U8X8 text-only mode provided by the same U8g2 package. This avoids changing the external dependency and removes the graphics/page-buffer requirement.
- Evaluate a no-framebuffer graphics driver only if U8X8 does not provide a sufficient display and SRAM margin.

### NEXT STEP

Perform a controlled compile-only experiment using U8X8 text mode in a temporary copy of the display module. Compare Flash and global SRAM against the current 21,154-byte / 1,726-byte integrated build. Do not flash until an acceptable memory margin and runtime test plan are established.
