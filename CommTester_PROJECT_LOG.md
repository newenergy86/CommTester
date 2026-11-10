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

---

## CHECKPOINT-31 — U8X8 text-mode compile experiment

### CHANGE

- Performed a temporary, local-only conversion of the display module from U8g2 graphics mode to U8X8 text-only mode.
- Reused the existing U8g2 library package; no external dependency was changed.
- The experiment used the SSD1306 128×64 hardware-I²C U8X8 driver and text rendering API.
- No GitHub firmware source was changed as part of this experiment.

### TEST

The user compiled the complete CommTester sketch for Arduino Nano and reported:

- Flash: 16,576 bytes (53% of 30,720 bytes).
- Global SRAM: 1,508 bytes (73% of 2,048 bytes).
- Compiler-reported remaining SRAM: 540 bytes.

Compared with the U8g2 graphics-mode integrated build from CHECKPOINT-30:

- Flash decreased by 4,578 bytes.
- Global SRAM decreased by 218 bytes.
- Compiler-reported remaining SRAM increased from 322 to 540 bytes.

Compilation: PASS, user-reported.

Firmware upload and runtime tests: NOT RUN.

### FACTS

- U8X8 text mode compiles in the complete Nano project.
- The compile-time memory report is materially better than the U8g2 graphics-mode build.
- The 540-byte remainder is only the compiler-reported space left after global allocations. It does not establish worst-case runtime stack headroom.
- The verified pre-display SERVICE/SLAVE firmware remains the last accepted runtime baseline.
- The U8X8 experiment remains local and is not yet an accepted repository firmware change.

### DECISION

- Keep the U8X8 approach as the current display candidate.
- Do not upload the U8X8 build yet.
- Before flashing, assess runtime SRAM/stack margin under the most demanding SERVICE and Modbus paths.
- Do not alter Modbus, SERVICE, or indicator logic for this memory experiment.

### NEXT STEP

Prepare a temporary runtime stack/high-water diagnostic and a bounded test procedure. Use it to assess remaining SRAM under representative and worst-case paths before deciding whether the U8X8 build is safe to flash.

### BLOCKERS

Worst-case runtime stack margin is not yet measured. OLED-enabled SERVICE and Modbus regression tests have not yet been run.



---

## CHECKPOINT-32 — U8X8 display source synchronized with the flashed firmware

### CHANGE

- Replaced the stale U8g2 graphics-mode implementation in `firmware/CommTester_Bench/display.cpp` with the user's uploaded U8X8 text-mode implementation.
- Kept the existing U8g2 package; no external library was changed.
- Updated `PROJECT.md` to distinguish the user's runtime observation from a fully documented regression result.

### FACTS

- The user confirmed that the U8X8 implementation is the version flashed to the Nano.
- The user reports that the device currently operates without visible failures.
- The user's reported build figures remain: Flash 16,576 bytes; global SRAM 1,508 bytes; compiler-reported remaining SRAM 540 bytes.
- The 540-byte compiler remainder is not a worst-case runtime stack measurement.
- The previous GitHub `display.cpp` was still the U8g2 graphics-mode version; this checkpoint synchronizes it to the uploaded U8X8 source.

### TEST

- GitHub source update: PASS.
- Post-update fetch: PASS; U8X8 constructor is present and the old U8g2 graphics constructor is absent from the current `display.cpp`.
- Arduino compile after synchronization: NOT RUN in this checkpoint.
- Full SERVICE/Modbus regression with OLED enabled: NOT RECORDED.

### DECISION

- Treat U8X8 text mode as the current display implementation candidate and preserve it.
- Do not revisit stack-watermark diagnostics as a prerequisite to the next practical task.
- Preserve the existing SERVICE screen and Modbus behavior while implementing the agreed Slave traffic statistics.

### NEXT STEP

Add request/response counters for valid Modbus requests addressed to this Slave and display the agreed statistics on the SLAVE screen. Test with addressed requests, foreign IDs, broadcast and invalid CRC frames.



---

## CHECKPOINT-33 — Modbus Slave traffic statistics added to OLED source

### CHANGE

- Added 32-bit request and response counters in `modbus_slave.cpp`.
- Added `ModbusSlave::getStatistics()` to expose the counters without coupling the display module to Modbus internals.
- A request is counted only after successful RTU reception, CRC validation and a match with `SLAVE_ID=1`.
- Foreign IDs, Broadcast ID 0, transport errors and invalid-CRC frames are excluded from the statistics. Existing `Indicators::rx()` behavior was not changed.
- A response is counted only when `writeAdu()` returns true. Normal and Modbus Exception responses use the same transmission path and are counted equally.
- Updated the OLED API to accept request/response totals. The SLAVE screen now shows REQ, RESP, RATE and MISS; the SERVICE screen remains unchanged.
- The display caches the last mode and counter values and redraws only when one of them changes.
- No external library, Modbus function implementation, register map, serial parameter or SERVICE command was changed.

### FACTS

- The current `ModbusRTU::writeAdu()` returns `bool`; its present implementation returns true after transmission.
- The current Slave processing path prepares exception responses in `ModbusSlaveLogic::processPdu()` and sends them through the same `writeAdu()` call.
- The statistics are held in `uint32_t` counters. The OLED counter fields are ten digits wide.
- The percentage calculation currently uses 64-bit intermediate arithmetic to avoid overflow when multiplying long-running 32-bit counters. Its Flash/runtime cost must be checked in the Nano build.
- The source changes have been fetched back from GitHub and the expected API and display fields are present.

### TEST

- GitHub source updates: PASS.
- Post-update fetch/source presence checks: PASS.
- Arduino Nano compilation: NOT RUN.
- Updated Flash/SRAM report: NOT AVAILABLE.
- OLED runtime test: NOT RUN.
- Statistics acceptance test: NOT RUN.

### DECISION

- Keep the statistics implementation as a test candidate, not yet as a verified firmware baseline.
- Do not flash it until the user compiles the current repository source and the resulting Flash/SRAM report is reviewed.
- Preserve the current U8X8 SERVICE screen and existing Modbus behavior.

### ACCEPTANCE TEST

After successful compile and memory review:

1. Reset the Nano; enter SLAVE mode. Expected initial display: REQ=0, RESP=0, RATE=--.-%, MISS=0.
2. Send 10 valid requests to Slave ID 1. Expected: REQ=10, RESP=10, RATE=100.0%, MISS=0.
3. Send valid requests addressed to another ID, one Broadcast request (ID 0), and one invalid-CRC request. Expected: all four displayed statistics remain unchanged.
4. Send a valid request to ID 1 that produces a Modbus Exception response. Expected: REQ and RESP both increment by one.

### NEXT STEP

Compile the current GitHub firmware for Arduino Nano. Report compiler errors, Flash usage and global SRAM usage before flashing or running the hardware acceptance test.

### BLOCKERS

Build size and runtime cost of the 64-bit percentage calculation are TBD. No hardware PASS is claimed for the new statistics.



---

## CHECKPOINT-34 — OLED traffic statistics firmware compile PASS

### TEST

The user compiled the current GitHub firmware for Arduino Nano and reported:

- Flash: 17,706 bytes (57% of 30,720 bytes).
- Global SRAM: 1,568 bytes (76% of 2,048 bytes).
- Compiler-reported remaining SRAM: 480 bytes.
- Compilation: PASS.

Compared with the preceding U8X8 build before traffic statistics (CHECKPOINT-32):

- Flash increased from 16,576 to 17,706 bytes (+1,130 bytes).
- Global SRAM increased from 1,508 to 1,568 bytes (+60 bytes).
- Compiler-reported remaining SRAM decreased from 540 to 480 bytes.

### FACTS

- The current statistics implementation compiles for the target Arduino Nano.
- The compiler-reported global SRAM remainder is 480 bytes. This is not a direct measurement of worst-case runtime stack margin.
- The user has not yet reported uploading or testing the statistics firmware on hardware.
- The build report is from the user's local Arduino environment.

### DECISION

- The compile and memory result is accepted as sufficient to proceed to controlled hardware verification.
- No further memory optimization is justified before testing the actual feature.
- Keep the current implementation unchanged during the first hardware test unless a reproducible defect is found.

### TEST — PENDING HARDWARE

1. Upload the compiled firmware and enter SLAVE mode. Expected initial display: REQ=0, RESP=0, RATE=--.-%, MISS=0.
2. Send 10 valid requests to Slave ID 1. Expected: REQ=10, RESP=10, RATE=100.0%, MISS=0.
3. Send valid requests addressed to another ID, one Broadcast request (ID 0), and one invalid-CRC request. Expected: all four displayed statistics remain unchanged.
4. Send a valid request to ID 1 that produces a Modbus Exception response. Expected: REQ and RESP both increment by one.

### NEXT STEP

Upload the compiled firmware and run the hardware acceptance test above. Report the actual OLED values and any unexpected behavior.

### BLOCKERS

No compile blocker. Runtime display behavior, counter semantics on hardware and regression of existing SERVICE/SLAVE functionality remain to be verified.

## CHECKPOINT-35 — Expanded Modbus status screen implemented in source (verification pending)

### CHANGE

- Reworked the SLAVE OLED layout to show:
  - local address as `Slave:01`;
  - destination address from the latest valid RTU frame as `Master:01`;
  - last function code with a short operation label;
  - RX and TX counters on one row;
  - RATE and MISS on separate rows;
  - serial settings `9600 8N1`.
- Preserved the existing SERVICE screen.
- Updated the Modbus display-data API to provide the local Slave ID, last valid frame destination ID and function code, plus the existing counters.
- The last destination ID/function are updated for any successfully received frame with valid CRC, including frames addressed to a different Slave ID. Transport errors and invalid-CRC frames do not replace the displayed last-frame information.
- The RX/TX row uses compact k/M/G notation for counts above 9999 so both values fit within the OLED's 16 text columns. MISS remains a full decimal count.
- No external libraries, Modbus function processing, register maps or serial settings were changed.

### DISPLAY SEMANTICS

- The display label `Master` means the destination Slave address contained in the Master-originated request. Modbus RTU does not carry a separate Master station ID in the request frame.
- The local Slave ID is shown separately and remains 01 in the current firmware.
- A valid request to a foreign ID updates the displayed destination/function but does not increment this device's RX/TX statistics.

### TEST

- Current GitHub source was fetched before editing: PASS.
- Source edits and API integration: committed; post-update fetch verification pending.
- Arduino Nano compile: NOT RUN.
- OLED hardware rendering: NOT RUN.
- Modbus statistics hardware acceptance: NOT RUN.

### NEXT STEP

Compile the current GitHub firmware for Arduino Nano and report Flash/SRAM figures. If compilation passes, upload and verify the screen with:
1. No received frames: `Master:--`, `FC:--`, RX=0, TX=0, RATE=--.-%, MISS=0.
2. Valid FC03 request to ID 01: `Master:01`, `FC:03 READ HR`; RX/TX increment.
3. Valid FC04 request to ID 01: function line changes to `FC:04 READ IR`.
4. Valid frame to a foreign ID: destination/function update, but RX/TX statistics remain unchanged.
5. Invalid-CRC frame: last valid destination/function and statistics remain unchanged.

### BLOCKERS

The new layout/API has not yet been compiled or tested on hardware. No hardware PASS is claimed.

## CHECKPOINT-36 — Slave and Master addresses combined on one OLED row

### CHANGE

- Shortened the address labels to `SLV` and `MST`.
- Combined both addresses on one row: `SLV:01 MST:01`.
- Kept the initial unknown destination as `MST:--`.
- Address formatting supports both two-digit and three-digit values within the 16-column OLED width.
- No Modbus processing, counters, serial settings, external libraries or SERVICE screen behavior changed.

### TEST

- GitHub source fetched before editing: PASS.
- Display source update: committed.
- Project documentation update: committed.
- Post-update fetch verification: pending.
- Arduino Nano compilation and OLED hardware test: NOT RUN.

### NEXT STEP

Compile the updated firmware for Arduino Nano and report Flash/SRAM usage. If compilation passes, verify that both IDs appear on one line and that three-digit destination IDs do not overwrite adjacent text.

### BLOCKERS

Compilation and hardware rendering remain unverified.


---

## CHECKPOINT-37 — SERVICE removed from active Nano firmware (verification pending)

### CHANGE

- Removed SERVICE command processing (PING/PONG, HELP, INFO and MODE) from the main sketch.
- Removed SERVICE/SLAVE mode switching; the Nano now starts directly in Modbus Slave mode.
- Removed the SERVICE screen from the OLED module. The existing Slave status layout and its fields are retained.
- Removed mode-state, mode-LED, old button polling and standalone mode-beep logic from the Indicators module. RX/TX/ERROR event indication remains.
- Extracted the RS-232 SoftwareSerial instance and initialization into the project-owned `rs232_port.h/.cpp` module.
- Updated Modbus Slave to obtain its SoftwareSerial stream from `RS232Port::getSerial()`.
- Removed the obsolete `serial_console.h/.cpp` files and their command parser/buffer.
- USB Serial diagnostics in the Modbus Slave implementation were intentionally retained.
- No Modbus function-code processing, register maps, CRC logic, traffic-counter semantics, RTU transport implementation, or external library was intentionally changed.

### FACTS

- The active main sketch no longer references SERVICE, `IndicatorMode`, mode switching, or the console parser.
- The Modbus Slave still receives a `SoftwareSerial` stream on D10/D11, initialized at 9600 baud.
- The display API now receives Slave status data directly, without an operating-mode argument.
- D7 is no longer polled by the Indicators module; it is reserved for the planned encoder button.
- The former mode LED output is no longer used by firmware.

### DECISIONS

- The Nano prototype operates only as Modbus Slave.
- SERVICE remains a possible feature of the future, more capable MCU revision; it is removed only from the active Nano firmware.
- Keep the RS-232 transport as a separate project-owned module. Do not reintroduce command parsing into it.

### TEST

- GitHub source edits: committed.
- Post-edit GitHub fetch/source review: PASS; active firmware files contain no stale SERVICE, `IndicatorMode`, or `SerialConsole` references; the deleted console files return not found.
- Arduino Nano compilation: NOT RUN.
- Flash/SRAM measurement: NOT AVAILABLE.
- OLED runtime test: NOT RUN.
- Modbus hardware regression (FC01/02/03/04/05/06/15/16 and CRC rejection/recovery): NOT RUN after this change.

### ACCEPTANCE CRITERIA

1. Arduino Nano build compiles without errors.
2. Compiler Flash and global SRAM figures are recorded and compared with the previous 17,706-byte Flash / 1,568-byte SRAM report (same local toolchain and library versions required for a valid comparison).
3. On reset, the device starts directly in Slave mode and the OLED displays the Slave status screen.
4. All eight existing Modbus function-code tests pass.
5. Invalid CRC is rejected without a response, and a subsequent valid request is processed normally.
6. RX/TX/ERROR indication remains operational.

### NEXT STEP

Fetch and inspect all changed GitHub files for stale SERVICE/console references and API consistency. Then compile the current repository revision for Arduino Nano. Do not proceed to encoder implementation until compile and baseline hardware regression are accepted.

### BLOCKERS

No user-reported compile or hardware result is available for this checkpoint. The new source is a candidate revision, not yet a verified firmware baseline.

## CHECKPOINT-38 — SERVICE-free Nano firmware compilation PASS

### FACT

- The user compiled the current SERVICE-free firmware for Arduino Nano / ATmega328P successfully; no compiler errors were reported.
- Flash usage: **16,962 bytes (55% of 30,720 bytes)**.
- Global SRAM usage: **1,592 bytes (77% of 2,048 bytes)**.
- Remaining SRAM: **456 bytes** for stack and local variables.
- Compared with the previous recorded build (17,706 bytes Flash / 1,568 bytes global SRAM): Flash decreased by 744 bytes; global SRAM increased by 24 bytes.
- The comparison is directly meaningful only if the Arduino core, compiler, board settings and library versions were unchanged.

### TEST

- SERVICE-free source compilation: **PASS** (user-reported).
- Flash/SRAM measurement: **RECORDED**.
- Upload to Nano: NOT YET REPORTED.
- Direct Slave startup and OLED runtime check: NOT YET REPORTED.
- Modbus regression after SERVICE removal (FC01/02/03/04/05/06/15/16, invalid CRC rejection and valid-frame recovery): NOT YET REPORTED.
- RX/TX/ERROR indicator check: NOT YET REPORTED.

### NEXT STEP

Upload this exact GitHub revision to the Nano. Verify direct Slave startup and the OLED status screen, then run the existing Modbus regression and indicator checks. Do not begin encoder implementation until these hardware acceptance checks pass.

### BLOCKERS

Compilation is no longer a blocker. Hardware acceptance of the SERVICE-free baseline remains pending. SRAM headroom is 456 bytes; monitor stack usage during runtime testing before adding features.

## CHECKPOINT-39 — SERVICE-free Modbus function regression PASS

### FACT

The user ran `modbus_test_all.py` against the Nano on COM7 with 9600 baud, 8N1, Slave ID 1. All reported tests passed:

- FC01 — Read Coils.
- FC02 — Read Discrete Inputs.
- FC03 — Read Holding Registers at addresses 0 and 120.
- FC04 — Read Input Registers at addresses 0 and 120.
- FC05 — Write Single Coil, including read-back verification.
- FC06 — Write Single Holding Register, including read-back verification.
- FC15 — Write Multiple Coils, including read-back verification.
- FC16 — Write Multiple Holding Registers, including read-back verification.

The script summary reports OVERALL RESULT: PASS.

This report does not show a deliberate invalid-CRC test, direct recovery-after-invalid-CRC test, or RX/TX/ERROR indicator verification. Those checks remain pending unless recorded separately.

### DISPLAY OBSERVATION

The user reports that the OLED visibly flickers on each poll because the complete screen is redrawn. Current `display.cpp` confirms that any changed display datum calls `drawSlaveScreen()`, which begins with `u8x8.clearDisplay()` and redraws all rows.

### PROPOSED CHANGE

Use incremental row updates: keep the full screen draw for initialization only, then redraw only rows whose displayed values changed. Preserve the U8X8 library and all Modbus/transport behavior.

### NEXT STEP

Implement the minimal display-only incremental refresh, then compile and verify that Modbus operation is unchanged and that routine counter updates no longer clear/redraw the entire OLED.

## CHECKPOINT-40 — OLED incremental row refresh implemented (compile pending)

### CHANGE

- Updated `firmware/CommTester_Bench/display.cpp` only.
- Kept `u8x8.clearDisplay()` exclusively in the initial full-screen draw called by `Display::begin()`.
- `Display::update()` now compares the incoming values with the cached values and redraws only affected rows:
  - row 1: Slave/destination addresses;
  - row 3: function code and operation;
  - row 4: RX/TX counters;
  - row 5: response rate;
  - row 6: missed responses.
- Rows are written as exactly 16 characters, padding with spaces to erase remnants of longer previous values.
- The title and serial-format rows are drawn at initialization and are not refreshed during normal polling.
- No changes to `display.h`, Modbus processing, RTU transport, indicators, pin assignments, or external libraries.

### SOURCE REVIEW

- Fresh GitHub source fetched before editing: PASS.
- Updated file fetched after commit: PASS.
- Exactly one `clearDisplay()` call remains, inside the initial full-screen draw.
- `Display::update()` contains no full-screen redraw call.
- Incremental row update and 16-character row padding confirmed by source inspection.

### TEST

- Source update: committed.
- Arduino Nano compile: NOT YET RUN.
- OLED flicker check on hardware: NOT YET RUN.
- Modbus regression after display change: NOT YET RUN.

### NEXT STEP

Compile and upload this revision. Verify that routine Modbus polling no longer clears the full OLED, that only changed rows update, and that shorter values leave no stale characters. Then rerun `modbus_test_all.py` to confirm no protocol regression.

### BLOCKERS

Hardware behavior and compilation remain unverified. Do not proceed to encoder implementation until this display change is accepted.

## CHECKPOINT-41 — OLED RATE percent and automatic power-save source change (compile pending)

### CHANGE

- Fixed the RATE row formatting so the percent sign occupies its own character after the full `100.0` value. The no-statistics placeholder was widened to match the same five-character numeric field.
- Added a 30-second inactivity timeout in `display.cpp`.
- Added `Display::activity()`. It resets the inactivity timer and wakes the display with `u8x8.setPowerSave(0)` when asleep.
- The display enters power-save mode with `u8x8.setPowerSave(1)` after 30 seconds without a receive event.
- `ModbusSlave::update()` calls `Display::activity()` for every non-timeout receive result, before checking framing/CRC status. Thus valid frames, foreign-address frames, CRC errors and other reported receive errors wake the OLED. A pure receive timeout does not.
- While asleep, `Display::update()` continues refreshing the cached status values without writing to the OLED. On wake, the screen is redrawn from the latest cached values; subsequent changes use incremental row refresh.
- U8X8 `setPowerSave()` is an existing API of the installed U8g2/U8x8 library; no external library was changed.

### DECISION

- OLED inactivity timeout: 30 seconds.
- Only the OLED enters power-save mode. The Nano and Modbus Slave continue running.
- Any non-timeout Modbus receive event wakes the display, including malformed/CRC-invalid receive events.

### SOURCE REVIEW

- Fresh GitHub source fetched before editing: PASS.
- Updated `display.cpp`, `display.h`, and `modbus_slave.cpp` fetched after edits: PASS.
- The percent sign is explicitly written at the character after the five-character RATE value.
- Sleep/wake uses U8X8 `setPowerSave(1/0)`.
- Display activity is called after the receive timeout check and before frame/error validation.
- No changes to Modbus PDU handling, CRC algorithm, RTU timing, serial settings, register maps, indicators or external libraries.

### TEST

- Source changes: committed.
- Arduino Nano compile: NOT YET RUN.
- RATE rendering at no requests and at 100.0%: NOT YET RUN on hardware.
- OLED sleep after 30 seconds: NOT YET RUN.
- Wake on valid frame, foreign-ID frame and invalid-CRC/error receive: NOT YET RUN.
- Modbus regression after this change: NOT YET RUN.

### NEXT STEP

Compile and upload this revision. Verify RATE formatting, sleep after 30 seconds without a receive event, wake on a valid request and on an invalid-CRC/error receive event, and confirm Modbus responses continue while the OLED is asleep.

### BLOCKERS

Compilation and hardware behavior remain unverified. Do not proceed to encoder implementation until this change is accepted.


---

## CHECKPOINT-42 — RX/TX/ERR statistics and malformed PDU rejection implemented (verification pending)

### CHANGE

- Replaced the previous request/response statistics with:
  - `RX`: every non-timeout receive event, including CRC errors, RTU framing errors, valid foreign-ID frames and malformed requests.
  - `TX`: responses successfully passed through the existing `writeAdu()` path.
  - `RATE`: `TX / RX × 100%`.
  - `ERR`: RTU CRC errors, RTU framing errors and malformed request lengths for supported function codes.
  - `MISS`: displayed as `---` in Slave mode; reserved for future Master response timeouts.
- A valid frame addressed to another Slave increments RX but does not increment TX or ERR.
- Valid Modbus requests that produce exception responses (e.g. unsupported function or illegal data address) are still valid protocol exchanges and do not increment ERR.
- OLED layout uses the previously unused row 2 for ERR. RX/TX remain on row 4, RATE on row 5, MISS on row 6, and serial settings on row 7.
- Added request-length validation before calling `ModbusSlaveLogic::processPdu()`:
  - FC01–FC06 require exactly 8 RTU bytes including CRC.
  - FC15/FC16 require at least 9 bytes and exact agreement between RTU length and the byte-count field.
  - Unknown function codes remain eligible for an Illegal Function exception when the RTU frame has the minimum 4 bytes.
- Updated the project-owned RTU transport to preserve received length on CRC/framing errors and reject frames shorter than 4 bytes before calling the external `ModbusADU::crcGood()`.
- No external library was modified.

### DECISIONS

- RX means received RTU frame/event, not only a valid request addressed to this Slave.
- ERR is limited to CRC, RTU framing and malformed supported-function request length. Modbus exception responses are not transport/frame errors.
- MISS cannot be measured in Slave mode and is displayed as `---`.
- RATE is an observed response ratio over all received events; it can be below 100% because foreign IDs and invalid frames are included in RX.

### SOURCE REVIEW

- Fresh GitHub source fetched before edits: PASS.
- Updated source fetched back from GitHub: PASS.
- Display API, main sketch and Slave statistics API signatures were updated together.
- Short RTU frames are rejected before CRC validation; frame length is preserved for diagnostics.
- External `ModbusADU` and U8g2 libraries were not changed.

### TEST

- GitHub source updates and post-write fetch verification: PASS.
- Arduino Nano compilation: NOT RUN.
- Flash/SRAM usage: NOT AVAILABLE.
- OLED rendering of ERR and `MISS:---`: NOT RUN.
- Hardware RX/TX/ERR/RATE accounting: NOT RUN.
- FC01/02/03/04/05/06/15/16 regression after these changes: NOT RUN.

### ACCEPTANCE TEST

1. 10 valid requests to Slave ID 1: RX=10, TX=10, ERR=0, RATE=100.0%.
2. 5 valid requests plus 5 bad-CRC frames: RX=10, TX=5, ERR=5, RATE=50.0%; no response to bad CRC.
3. 5 valid requests plus 5 valid frames for another Slave ID: RX=10, TX=5, ERR=0, RATE=50.0%.
4. One short FC03 request with valid CRC: RX increments by 1, ERR increments by 1, TX does not change; no response.
5. Unknown function code and valid request with illegal register address: exception response is sent; RX and TX increment; ERR does not increment.
6. A valid FC03 request immediately after each error case receives a normal response.
7. OLED continues sleeping/waking as before; ERR updates incrementally without screen flicker.
8. Existing FC01/02/03/04/05/06/15/16 regression passes.

### NEXT STEP

Compile the latest GitHub revision for Arduino Nano / ATmega328P and report compiler output, Flash and SRAM. If compilation passes, upload and run the acceptance tests above.

### BLOCKERS

All changes are source-level only. No compile or hardware PASS is claimed. The previous compile figures (16,962 bytes Flash / 1,592 bytes global SRAM) predate this revision.


---

## CHECKPOINT-43 — RATE and ERR/MISS display corrections (verification pending)

### USER TEST

The user reported: RX=7, TX=4, ERR=2. Normal request, bad CRC rejection, foreign Slave ID handling, unknown function exception, illegal register address exception, and recovery passed. The malformed FC03 test reported FAIL despite the device returning no response.

### CHANGE

- Corrected RATE digit formatting. The previous display duplicated the tens digit for percentages from 10% through 99%; 57.1% now formats as 57.1%.
- Combined ERR and MISS counters on OLED row 2 as `ERR:n MISS:n`.
- MISS starts at zero and increments for a valid, structurally complete request addressed to another Slave ID. A broadcast request is also intentionally ignored and counted as MISS.
- Request structure validation now occurs before the foreign-ID check, so malformed requests count as ERR rather than MISS.
- Display update, wake redraw and cached statistics now include MISS.
- No external library was modified.

### DECISION

For the Nano Slave implementation, MISS means a valid request that this Slave intentionally ignores because the destination address is not its own. CRC/framing errors and malformed request lengths count as ERR.

Expected result for the user's test sequence: RX=7, TX=4, ERR=2, MISS=1, RATE=57.1%.

### TEST / ANALYSIS

- The supplied test-05 frame `01 03 00 00 00 19 84` has a valid CRC over the five-byte prefix `01 03 00 00 00`, but its FC03 PDU is incomplete. It should be rejected without a response under the current firmware policy.
- The test script is not stored in the repository; its test-05 assertion cannot yet be inspected. The FAIL result likely reflects an expectation mismatch, but this remains TBD until the script is reviewed.
- GitHub source fetch and post-write verification: PASS.
- Arduino Nano compilation: NOT RUN.
- OLED hardware verification: NOT RUN.
- Full Modbus regression after these changes: NOT RUN.

### NEXT STEP

Compile and upload the latest revision. Verify initial ERR=0/MISS=0, RATE=57.1% and ERR=2/MISS=1 after the reported seven-frame sequence. Provide `modbus_error_test.py` if test 05 needs its assertion corrected.

### BLOCKERS

No compile or hardware PASS is claimed for this checkpoint. The test-05 assertion remains unverified because the script is not in GitHub.

### PROTOCOL NOTE

The Modbus Application Protocol specification defines exception 03 (Illegal Data Value) for faults in request-data structure, including an incorrect implied length. The current Nano firmware deliberately discards a short fixed-length request without a response after detecting the malformed length. This is a project behavior choice, not a claim that silence is mandated by the specification. Whether to send exception 03 instead is TBD and should be decided after reviewing the test script and confirming the desired CommTester diagnostic behavior.


---

## CHECKPOINT-44 — OLED wake redraw uses current statistics (verification pending)

### USER-REPORTED ISSUE

After the OLED entered power-save mode following 30 seconds of inactivity, the next received Modbus frame woke the display, but the displayed counters remained at zero / did not reflect that frame.

### FACT — SOURCE-LEVEL CAUSE

- `ModbusSlave::update()` increments `rxCount` and then calls `Display::activity()` for a non-timeout receive event.
- In the previous implementation, `Display::activity()` woke the OLED and immediately called `drawSlaveScreen()` using the display module's cached values.
- The new values from the completed frame were passed to `Display::update()` on the following main-loop iteration.
- Therefore, the wake path redrew the stale snapshot before the regular update supplied the latest statistics.

### CHANGE

- Added the `displayNeedsRedraw` flag in `display.cpp`.
- `Display::activity()` now only wakes the OLED and marks it for a full redraw; it no longer draws from the stale cache.
- The next `Display::update()` performs the full-screen redraw using the current arguments (the latest statistics snapshot collected after frame processing), refreshes all cached values, and clears the flag.
- Normal incremental row updates and the 30-second sleep behavior are retained.
- No Modbus processing, RTU transport, statistics semantics, pin assignments, or external libraries were changed.

### SOURCE REVIEW

- Fresh GitHub source fetched before editing: PASS.
- `display.cpp` committed and fetched back; the wake path no longer redraws cached data: PASS.
- `PROJECT.md` updated and fetched back: PASS.
- No external library was modified.

### TEST

- GitHub source update and post-write verification: PASS.
- Arduino Nano compilation: NOT RUN.
- OLED sleep/wake hardware test: NOT RUN.
- Modbus regression: NOT RUN after this display-only correction.

### ACCEPTANCE TEST

1. Power on and confirm initial counters are zero.
2. Leave the device without Modbus receive events for more than 30 seconds; confirm the OLED enters power-save.
3. Send one valid request to Slave ID 1. The OLED must wake and show RX=1 and TX=1 (ERR=0, MISS=0).
4. Repeat after another sleep with one bad-CRC frame. The OLED must wake and show RX incremented by one and ERR incremented by one; TX must not increment.
5. Confirm the next valid request is processed normally and the counters continue from their previous values rather than resetting.

### NEXT STEP

Compile and upload this revision. Run the wake acceptance test above before proceeding with any other display or UI work.

### BLOCKERS

This is a source-level correction only. Compilation and hardware behavior are not yet verified; no hardware PASS is claimed.


---

## CHECKPOINT-45 — OLED row layout revised; sleep timeout increased to 120 seconds (verification pending)

### USER REQUEST

The user specified the following eight-line OLED layout:

1. `CommTester v0.4` — unchanged.
2. Blank spacer line.
3. `SLV` and `MST` addresses.
4. Function code and description (`FC`).
5. RX and TX counters.
6. RATE.
7. ERR and MISS counters on one line.
8. Serial port settings.

The inactivity timeout is increased from 30 seconds to 2 minutes.

### CHANGE

- Changed the OLED row mapping:
  - Row 0: header, unchanged.
  - Row 1: blank.
  - Row 2: SLV/MST.
  - Row 3: FC.
  - Row 4: RX/TX.
  - Row 5: RATE.
  - Row 6: ERR/MISS.
  - Row 7: serial settings.
- Increased `DISPLAY_SLEEP_TIMEOUT` from `30000UL` to `120000UL`.
- Kept the existing incremental row-refresh mechanism, current-statistics redraw after wake, and wake-on-non-timeout-receive behavior.
- No Modbus logic, statistics semantics, pin assignments, or external libraries were changed.

### SOURCE REVIEW

- Fresh current GitHub `display.cpp` fetched before editing: PASS.
- Updated `display.cpp` fetched back and content matched: PASS.
- Confirmed all eight row positions and 120000 ms timeout in the committed source: PASS.
- `PROJECT.md` updated and fetched back: PASS.
- No external library was modified.

### TEST

- GitHub source update and post-write verification: PASS.
- Arduino Nano compilation: NOT RUN.
- OLED layout on hardware: NOT RUN.
- 120-second sleep and wake test: NOT RUN.
- Modbus regression: NOT RUN (display-only source change).

### ACCEPTANCE TEST

1. Compile for Arduino Nano / ATmega328P; report Flash and SRAM.
2. Confirm line 1 remains unchanged and line 2 is blank.
3. Confirm SLV/MST on line 3, FC on line 4, RX/TX on line 5, RATE on line 6, ERR/MISS on line 7, and `9600 8N1` on line 8.
4. Leave the device without receive activity for 119 seconds: display must remain on.
5. At or after 120 seconds without receive activity: display must enter power-save.
6. Send one valid Modbus request after sleep: display must wake and show the latest counters without flicker or stale values.
7. Confirm normal Modbus responses continue and the previous RX/TX/ERR/MISS values are retained.

### NEXT STEP

Compile and upload the latest GitHub revision, then perform the OLED layout and 120-second sleep/wake checks.

### BLOCKERS

Source-level implementation only. No compile or hardware PASS is claimed.


---

## CHECKPOINT-46 — OLED layout and power-save hardware acceptance PASS

### USER-REPORTED TEST

The user confirmed that all five CHECKPOINT-45 acceptance steps passed:

1. Latest firmware compiled successfully for Arduino Nano / ATmega328P.
2. The revised eight-row OLED layout was visually verified.
3. The display remained active at 119 seconds and entered power-save at/after the 120-second inactivity threshold.
4. A received Modbus request woke the OLED and the display showed current counters immediately.
5. Modbus responses continued normally and the OLED did not flicker.

### BUILD RESULT — FACT

- Flash: **18,576 bytes (60% of 30,720 bytes)**.
- Global SRAM: **1,613 bytes (78% of 2,048 bytes)**.
- Remaining SRAM: **435 bytes** for stack and local variables.
- Compilation: **PASS**, user-reported.

Compared with CHECKPOINT-38 (16,962 bytes Flash / 1,592 bytes global SRAM / 456 bytes remaining), this build uses 1,614 additional Flash bytes and 21 additional global SRAM bytes, leaving 21 fewer bytes of reported SRAM headroom. Treat this as a direct comparison only if the board package, compiler, build settings and library versions are unchanged.

### ACCEPTANCE

- Eight-row layout: **PASS**.
- 120-second OLED sleep timing: **PASS**.
- Wake on Modbus receive with current statistics: **PASS**.
- Normal Modbus response operation during/after OLED power-save: **PASS**.
- No visible OLED flicker: **PASS**.
- Nano firmware compilation: **PASS**.

### DECISION

- Close CHECKPOINT-45.
- Mark M5 — Display as **COMPLETED**.
- Preserve the current display implementation as the accepted baseline.
- Do not make further display changes while beginning encoder work unless a regression is found.

### RESOURCE NOTE

The compiler reports 435 bytes of remaining SRAM. This is a limited margin; encoder integration must avoid dynamic allocation and unnecessary stack-heavy local objects. Validate runtime behavior and preserve the existing Modbus/OLED baseline.

### NEXT STEP

CHECKPOINT-47 — inspect current GitHub pin usage and implement the minimum encoder input test. D7 is reserved for the encoder button; D8/D9 are proposed for quadrature A/B pending a fresh source-level pin-conflict check. First acceptance criteria: correct direction, exactly one logical step per detent, reliable button press/release with debounce, and unchanged Modbus/OLED operation.

### BLOCKERS

No blocker for closing the display milestone. Encoder pin compatibility and input handling remain to be verified from the current repository source before implementation.


## CHECKPOINT-47 — Startup indication and encoder plan (decision accepted; implementation pending)

### DECISIONS

- Reassign the previously unused D5 indicator LED as a HEARTBEAT indicator.
- On startup, D5 will produce three short flashes synchronized with a short three-note ascending startup melody.
- Proposed melody: 523 Hz for 100 ms, 659 Hz for 100 ms, and 784 Hz for 180 ms, with approximately 40 ms gaps.
- In normal operation, D5 will flash briefly (approximately 50–80 ms) every 2 seconds.
- The startup melody plays once per power-up only.
- All indication timing must be non-blocking; no `delay()` calls may be introduced.
- Reuse the existing `Indicators` event/sound mechanism and `tone()` handling. Do not add an independent sound generator that could conflict with RX/TX/ERROR sounds.
- The encoder module has external pull-ups to +5 V. Planned wiring remains CLK/A=D8, DT/B=D9, SW=D7, +5 V and GND. Configure the signal pins as `INPUT` (not `INPUT_PULLUP`) and treat a closed contact as active LOW; implement software debounce.

### STATUS / SCOPE

- This is an accepted design decision, not an implementation or hardware PASS.
- Before coding, re-fetch the current indicator and main-sketch sources and inspect all pin references for D5/D7/D8/D9 conflicts.
- Implement and test the D5 heartbeat/startup indication first, independently of encoder decoding.
- Then implement the minimal encoder input test: direction, one logical step per detent, button press/release debounce, and no regression in Modbus/OLED operation.

### TEST CRITERIA

- Startup: exactly three short D5 flashes synchronized with the three-note melody; melody occurs only once.
- Normal operation: one short D5 flash at approximately 2-second intervals.
- Non-blocking behavior: Modbus request/response processing continues during startup indication and normal heartbeat.
- Sound arbitration: RX/TX/ERROR events remain correctly serialized and are not truncated or overlapped by the startup melody.
- No regression in OLED refresh, sleep/wake, or existing Modbus behavior.

### BLOCKERS

- Pin conflict review and implementation are pending.
- The melody and heartbeat have not yet been compiled or tested on hardware.


---

## CHECKPOINT-47A — D5 startup melody and HEARTBEAT implemented (verification pending)

### CHANGE

- Assigned D5 to `LED_HEARTBEAT`.
- Added a startup event to the existing `Indicators` event queue. It plays three ascending notes and flashes D5 in sync:
  - 523 Hz for 100 ms;
  - 659 Hz for 100 ms;
  - 784 Hz for 180 ms;
  - approximately 40 ms between notes.
- Added a periodic D5 heartbeat: 60 ms ON every 2 seconds, starting 2 seconds after the startup melody completes.
- The startup melody and RX/TX/ERROR sounds use the same event queue and existing `tone()` / `noTone()` mechanism.
- All timing is based on `millis()`; no blocking delays were added.
- No external libraries, Modbus logic, RS-232 transport, OLED code, or encoder code were changed.

### PIN REVIEW

- D2 — RX LED.
- D3 — TX LED.
- D4 — ERROR LED.
- D5 — HEARTBEAT LED (new use).
- D6 — buzzer.
- D7 — reserved encoder button.
- D8/D9 — proposed encoder A/B.
- D10/D11 — RS-232 SoftwareSerial.
- OLED uses hardware I²C A4/A5.

The reviewed current source files show no existing use of D5 in the other active modules. D7/D8/D9 remain reserved/proposed for the encoder and were not configured or changed in this update.

### SOURCE REVIEW

- Fresh GitHub files fetched before editing: PASS.
- `indicators.cpp` updated and fetched back: PASS.
- Startup event is inserted first during `Indicators::begin()`: PASS.
- Startup melody is handled as one event in the shared sound queue: PASS.
- Heartbeat is independent of RX/TX/ERROR event LEDs: PASS.
- No executable `delay()` calls were added: PASS.
- No external library or unrelated firmware module was changed.

### TEST

- GitHub source update and post-write fetch verification: PASS.
- Arduino Nano / ATmega328P compilation: **PASS**, user-reported.
- Flash: **19,548 bytes (63% of 30,720 bytes)**.
- Global SRAM: **1,624 bytes (79% of 2,048 bytes)**.
- Remaining SRAM: **424 bytes**.
- Compared with CHECKPOINT-46 build: +972 bytes Flash, +11 bytes global SRAM, and 11 fewer bytes remaining.
- Startup flash/melody hardware check: NOT YET RUN.
- 2-second heartbeat hardware check: NOT YET RUN.
- RX/TX/ERROR sound arbitration during/after startup: NOT YET RUN.
- Modbus/OLED regression: NOT YET RUN after this change.

### ACCEPTANCE TEST

1. Power-cycle the Nano. Confirm exactly three D5 flashes synchronized with the three ascending notes; the melody must play once only.
2. After the startup melody, confirm one approximately 60 ms D5 flash every 2 seconds.
3. Send Modbus requests during the startup sequence. Confirm requests are still received and answered normally.
4. Confirm queued RX/TX/ERROR sounds play after the startup melody without overlap or truncation.
5. Confirm the OLED still updates, sleeps after 120 seconds and wakes with current counters.
6. Compile for Arduino Nano / ATmega328P and record Flash/SRAM. Compare with the accepted baseline of 18,576 bytes Flash / 1,613 bytes global SRAM / 435 bytes remaining.

### NEXT STEP

Compile and upload this revision, then run the acceptance test above. Do not implement encoder handling until the D5 change passes.

### BLOCKERS

Compilation is PASS. Startup melody, heartbeat timing, sound arbitration and Modbus/OLED regression hardware checks are still pending. SRAM headroom is 424 bytes and remains limited.


---

## CHECKPOINT-47B — OLED constant strings moved to program Flash (verification pending)

### USER-REPORTED ISSUE

After the CHECKPOINT-47A firmware was uploaded, the user observed that the HEARTBEAT LED and corresponding sound operate, but the device appears to hang during a Modbus test run. The user suspects insufficient memory. This is not yet proven to be SRAM exhaustion.

### CHANGE

- Re-fetched the current GitHub display.cpp before editing.
- Moved fixed OLED labels and Modbus function descriptions into AVR program Flash using PROGMEM.
- Added drawRow_P() to read fixed strings from Flash into the existing 17-byte display line buffer before calling U8X8.
- Function descriptions are now read with pgm_read_byte().
- Dynamic display rows and their formatting remain unchanged.
- No Modbus maps, RTU transport, external libraries, pin assignments, indicator behavior or OLED layout were changed.

### SOURCE REVIEW

- Fresh GitHub source fetched before editing: PASS.
- display.cpp updated and fetched back; exact content match: PASS.
- External libraries unchanged: PASS.
- Modbus logic and display layout intentionally unchanged: PASS.

### TEST

- Source update and post-write verification: PASS.
- Arduino Nano / ATmega328P compilation: PENDING.
- Flash/SRAM comparison: PENDING.
- Sustained Modbus test and hang reproduction: PENDING.
- OLED display and sleep/wake regression: PENDING.
- Startup melody, heartbeat and RX/TX/ERROR sound arbitration regression: PENDING.

### ACCEPTANCE TEST

1. Compile for Arduino Nano / ATmega328P and report Flash and global SRAM.
2. Compare with the pre-optimization build: 19,548 bytes Flash / 1,624 bytes global SRAM / 424 bytes remaining.
3. Run the existing FC01/02/03/04/05/06/15/16 Modbus regression.
4. Run sustained/repeated Modbus traffic long enough to determine whether the previous hang recurs.
5. Confirm OLED text, row layout, 120-second sleep and wake behavior are unchanged.
6. Confirm startup melody, heartbeat and RX/TX/ERROR indications remain functional.

### NEXT STEP

Compile and upload CHECKPOINT-47B. First establish whether SRAM usage decreases and whether the hang is reproducible. If the device still hangs, investigate stack/heap collision and blocking serial diagnostics as separate hypotheses; do not make unrelated optimizations.

### BLOCKERS

No compile, memory reduction or hardware PASS is claimed yet. The root cause of the reported hang remains TBD.


---

## CHECKPOINT-47C — PROGMEM build and Modbus stability regression PASS

### BUILD RESULT — FACT

The user compiled the CHECKPOINT-47B source for Arduino Nano / ATmega328P and reported:

- Flash: **19,686 bytes (64% of 30,720 bytes)**.
- Global SRAM: **1,500 bytes (73% of 2,048 bytes)**.
- Remaining SRAM: **548 bytes**.
- Compilation: **PASS**.

Compared with CHECKPOINT-47A (19,548 bytes Flash / 1,624 bytes global SRAM / 424 bytes remaining):

- Flash increased by 138 bytes.
- Global SRAM decreased by 124 bytes.
- Compiler-reported free SRAM increased by 124 bytes.

### TEST — USER-REPORTED

- Complete Modbus test suite: **PASS**.
- Repeated Modbus traffic test: **PASS**.
- Device hang observed on the earlier CHECKPOINT-47A build did not recur; no hangs were observed during the reported tests.

### ANALYSIS / LIMITATION

The successful PROGMEM build and disappearance of the hang are correlated observations. This test does not independently prove that SRAM exhaustion was the original root cause, nor does the compiler's 548-byte free-SRAM figure establish worst-case runtime stack margin.

### DECISION

- Accept the PROGMEM optimization as the current Nano firmware baseline.
- Preserve the current Modbus implementation; no further memory or protocol changes are justified by this test result.
- Do not continue optimizing the already working Modbus path without a measured defect or new requirement.

### NEXT STEP

Complete the remaining explicit hardware acceptance checks for D5 startup melody/HEARTBEAT timing, sound arbitration and OLED behavior on this build. Then proceed to the minimal encoder input test.

### STATUS

- Compile and memory comparison: **PASS**.
- Complete Modbus regression: **PASS**.
- Repeated traffic / no-hang test: **PASS**.
- Original hang root cause: **TBD**.
- Full D5 melody/heartbeat and sound-arbitration acceptance on this build: **PENDING**.


---

## CHECKPOINT-47D — Startup melody, HEARTBEAT and sound arbitration hardware PASS

### USER-REPORTED HARDWARE TEST

The user confirms that the following behaviors are present and working on the current PROGMEM-optimized Nano build:

- Startup melody: **PASS**.
- Three synchronized startup flashes on D5: **PASS**.
- Periodic D5 HEARTBEAT: **PASS**.
- No conflicts between startup melody and RX/TX/ERROR sound indications: **PASS**.

### ACCEPTANCE

The previously pending startup/heartbeat and sound-arbitration checks are now accepted based on the user's hardware confirmation. Together with CHECKPOINT-47C, the current build has passed compilation, memory measurement, complete Modbus regression, repeated traffic stability, and the reported indicator checks.

### DECISION

- Accept D5 startup melody and HEARTBEAT implementation as the current hardware-verified baseline.
- Do not modify the indicator state machine without a demonstrated defect.
- Proceed to the minimal encoder input test, after a fresh review of current GitHub pin usage and source.

### NEXT STEP

Review the current main sketch and all active modules for D7/D8/D9 pin conflicts. Then implement only the minimum encoder input test: direction, one logical step per detent, and debounced button press/release. Preserve the verified Modbus, OLED and indicator behavior.

### STATUS

- Startup melody and three D5 flashes: **PASS**.
- Periodic D5 HEARTBEAT: **PASS**.
- Sound arbitration: **PASS**.
- Modbus regression and repeated traffic: **PASS** (CHECKPOINT-47C).
- Encoder input implementation: **NOT STARTED**.


---

## CHECKPOINT-48 — KY-040 encoder input test implemented (verification pending)

### CHANGE

- Added project-owned `encoder.h/.cpp`.
- Integrated `Encoder::begin()` and `Encoder::update()` into the main sketch.
- Connected encoder signals in source as CLK/A=D8, DT/B=D9, SW=D7.
- Rotation decoding uses the external KY040 library by codingABI, version 1.0.3, in polling mode. The external library itself is not modified.
- Encoder signal pins are configured as `INPUT`, relying on the encoder module's onboard pull-up resistors.
- Added project-owned 30 ms software debounce for the SW button.
- Direction and stable button transitions are reported over USB Serial at 115200 baud:
  - `ENC: CW`
  - `ENC: CCW`
  - `ENC: BUTTON PRESSED`
  - `ENC: BUTTON RELEASED`
- No menu, OLED layout, Modbus logic, RS-232 transport, indicators, or external library source was changed.

### DESIGN NOTE

- D8/D9 belong to the ATmega328P pin-change interrupt group also used by SoftwareSerial RX on D10. To avoid sharing/replacing the SoftwareSerial interrupt vector, the encoder is initially polled using KY040's polling API.
- In the current Modbus transport, `_readTimeout` is initialized to zero and no nonzero timeout is configured by the Slave setup. Thus the no-data path returns immediately; frame reception itself temporarily occupies the loop while the RTU frame boundary is collected. The effect of this interval on encoder polling must be checked on hardware.

### SOURCE REVIEW

- Fresh current GitHub source fetched before implementation: PASS.
- D7/D8/D9 pin usage checked against the current sketch, indicator module and RS-232 transport: no existing assignments found.
- Main sketch updated and fetched back: PASS.
- New encoder module files created: PASS.
- `PROJECT.md` updated: PASS.
- External KY040 library source not copied or modified: PASS.

### TEST

- GitHub source updates and post-write fetch verification: PASS.
- Arduino Nano / ATmega328P compilation: **PASS**, user-reported.
- Flash: **20,492 bytes (66% of 30,720 bytes)**.
- Global SRAM: **1,502 bytes (73% of 2,048 bytes)**.
- Compiler-reported remaining SRAM: **546 bytes**.
- Compared with CHECKPOINT-47C (19,686 bytes Flash / 1,500 bytes global SRAM / 548 bytes remaining): Flash +806 bytes, global SRAM +2 bytes, reported free SRAM −2 bytes.
- Encoder direction / one-step-per-detent test: NOT RUN.
- Button debounce test: NOT RUN.
- Modbus regression under encoder activity: NOT RUN.
- OLED and indicator regression: NOT RUN.

### ACCEPTANCE TEST

1. Install KY040 library version 1.0.3 by codingABI in the Arduino IDE/library environment.
2. Compile for Arduino Nano / ATmega328P; record Flash and SRAM. Treat the previous verified build (19,686 bytes Flash / 1,500 bytes global SRAM / 548 bytes remaining) as the comparison baseline, not as a prediction.
3. Rotate slowly clockwise through 20 detents. Expect exactly 20 `ENC: CW` messages and no CCW messages.
4. Rotate slowly counter-clockwise through 20 detents. Expect exactly 20 `ENC: CCW` messages and no CW messages.
5. Repeat rotations at a moderate speed and while Modbus traffic is active; check for missed or duplicated detents.
6. Press and release SW repeatedly, including short presses and contact bounce. Expect one message per stable press and one per stable release.
7. Run the full FC01/02/03/04/05/06/15/16 Modbus regression and repeated traffic test.
8. Confirm OLED refresh/sleep/wake and RX/TX/ERROR/HEARTBEAT indications remain unchanged.

### NEXT STEP

Compile and upload this revision, then perform the encoder acceptance test before adding any menu or UI behavior.

### STATUS

- Encoder source implementation: COMPLETE.
- Arduino Nano compilation: **PASS**.
- Encoder and full hardware regression verification: PENDING.
- No hardware PASS is claimed in this checkpoint.


## CHECKPOINT-49 — Encoder sound feedback added to shared indicator queue (verification pending)

### USER OBSERVATION

After uploading CHECKPOINT-48, the user reported that rotating and pressing the encoder did not interfere with Modbus packet reception. However, without feedback it was not clear whether the encoder events were being detected. This is a user-reported observation; encoder event counts and sound behavior were not yet verified.

### CHANGE

- Added `Indicators::encoderStep()` and `Indicators::encoderPress()` to the project-owned indicator interface.
- Added two event types to the existing shared sound queue:
  - Encoder rotation step: 2800 Hz for 35 ms.
  - Encoder button press: 1200 Hz for 110 ms.
- Both clockwise and counter-clockwise rotation events enqueue the same short step sound.
- A debounced button press enqueues the longer, lower-pitched sound. Button release remains silent.
- The encoder continues to print direction and button transitions to USB Serial.
- All encoder sounds use the existing `Indicators::update()` / `tone()` event mechanism. No direct tone generation, delay, ISR, pin reassignment, Modbus change, or external library modification was introduced.

### DESIGN NOTE

- Encoder sounds are serialized with startup, RX, TX and ERROR sounds; they cannot interrupt an active sound.
- The shared queue has a fixed capacity of eight events. As with existing event types, an event is dropped if the queue is full. Under sustained traffic, audible feedback may therefore be delayed or omitted, but the encoder callback does not wait for sound playback.
- This is intended as diagnostic feedback, not as a guaranteed one-sound-per-detent interface under a saturated sound queue.

### SOURCE REVIEW

- Fresh current GitHub sources fetched before editing: PASS.
- `encoder.cpp`, `indicators.cpp`, and `indicators.h` updated and fetched back with exact content match: PASS.
- `PROJECT.md` updated and fetched back with exact content match: PASS.
- No external library, Modbus code, RS-232 transport, OLED code, or pin assignment changed.

### TEST

- GitHub source update and post-write verification: PASS.
- Arduino Nano / ATmega328P compilation: NOT RUN.
- Flash/SRAM measurement: NOT RUN.
- Encoder rotation sound: NOT RUN on hardware.
- Encoder button-press sound: NOT RUN on hardware.
- Startup melody and RX/TX/ERROR sound arbitration regression: NOT RUN.
- Modbus regression and sustained traffic: NOT RUN after this change.

### ACCEPTANCE TEST

1. Compile for Arduino Nano / ATmega328P and record Flash/SRAM.
2. Rotate one detent at a time in both directions. Confirm each reported `ENC: CW` or `ENC: CCW` event produces one short, high-pitched sound.
3. Press and release the encoder button. Confirm one lower-pitched sound on each stable press and no sound on release.
4. Rotate repeatedly at moderate speed. Check for missed or duplicated serial events and audible feedback.
5. Repeat while Modbus traffic is active. Confirm packet reception and responses remain normal; confirm RX/TX sounds and encoder sounds are serialized without overlap.
6. Power-cycle and confirm the startup melody still plays correctly.
7. Run the full FC01/02/03/04/05/06/15/16 Modbus regression and confirm OLED and LED behavior are unchanged.

### NEXT STEP

Compile and upload this revision, then perform the encoder sound-feedback acceptance test. Do not add menu behavior until the input and sound checks pass.

### STATUS

- Source implementation: COMPLETE.
- GitHub write/read-back verification: PASS.
- Compilation: PENDING.
- Encoder and sound hardware verification: PENDING.
- Modbus regression after this change: PENDING.


## CHECKPOINT-49A — Encoder sound feedback compile and hardware check PASS

### BUILD RESULT — USER-REPORTED FACT

The user compiled the CHECKPOINT-49 sound-feedback revision for Arduino Nano / ATmega328P and reported:

- Flash: **20,532 bytes (66% of 30,720 bytes)**.
- Global SRAM: **1,506 bytes (73% of 2,048 bytes)**.
- Compiler-reported remaining SRAM: **542 bytes**.
- Compilation: **PASS**.

Compared with CHECKPOINT-48 (20,492 bytes Flash / 1,502 bytes global SRAM / 546 bytes remaining):

- Flash: +40 bytes.
- Global SRAM: +4 bytes.
- Compiler-reported remaining SRAM: −4 bytes.

### HARDWARE TEST — USER-REPORTED

The user confirmed that the encoder sound check passed. The audible feedback for encoder interaction is therefore accepted as **PASS**.

The user had also reported that encoder rotation and button interaction did not interfere with Modbus packet reception on the preceding encoder-input build. This observation is retained, but a complete Modbus regression specifically after the sound-feedback change has not been reported.

### ACCEPTANCE STATUS

- Arduino Nano compilation: **PASS**.
- Flash/SRAM measurement: **PASS**.
- Encoder sound feedback: **PASS**.
- Exact one-event-per-detent CW/CCW count: **NOT YET CONFIRMED**.
- Button debounce / one transition per press and release: **NOT YET CONFIRMED**.
- Full FC01/02/03/04/05/06/15/16 regression after this change: **PENDING**.
- OLED and all indicator regression after this change: **PENDING**.

### DECISION

- Accept the shared-queue sound implementation as the current baseline.
- Do not adjust sound frequencies or durations without a concrete usability issue.
- Do not add menu behavior yet; first close the remaining encoder input and regression checks.

### NEXT STEP

Verify exact CW/CCW event counts and stable button transitions, then run the complete Modbus regression and confirm OLED/indicator behavior.

### RESOURCE NOTE

The compiler reports 542 bytes of remaining SRAM. This is compiler-reported free SRAM, not a measurement of worst-case runtime stack margin. Avoid unnecessary global buffers and large local variables.


## CHECKPOINT-49B — Encoder detent count and active-traffic checks PASS

### USER-REPORTED HARDWARE TEST — FACT

The user performed the encoder acceptance checks and reported:

- 20 clockwise detents produced exactly 20 `ENC: CW` messages.
- 20 counter-clockwise detents produced exactly 20 `ENC: CCW` messages.
- No opposite-direction events were observed during these counted tests.
- Encoder rotation continued to be detected correctly while Modbus traffic was active.
- Button press and release transitions were visible as `ENC: BUTTON PRESSED` and `ENC: BUTTON RELEASED`.
- Encoder sound feedback remains operational (previously accepted in CHECKPOINT-49A).

### ACCEPTANCE STATUS

- Exact one-event-per-detent count in both directions, for the tested 20-detent runs: **PASS**.
- Encoder polling during active Modbus traffic, for the reported test: **PASS**.
- Button press/release event reporting: **PASS**.
- Sound feedback: **PASS** (CHECKPOINT-49A).
- Exhaustive button debounce/bounce stress test: **NOT YET CONFIRMED**.
- Full FC01/02/03/04/05/06/15/16 regression after the encoder sound-feedback change: **PENDING**.
- OLED refresh/sleep/wake and complete RX/TX/ERROR/HEARTBEAT regression after this change: **PENDING**.

### DECISION

- Accept the tested KY-040 polling implementation and its current sound feedback as the encoder input baseline.
- Do not modify the encoder decoder or sound queue without a reproducible defect.
- Do not add menu behavior until the remaining Modbus, OLED and indicator regression is complete.

### NEXT STEP

Run the full Modbus function regression (FC01/02/03/04/05/06/15/16), followed by OLED and indicator checks. Preserve the current firmware baseline.

### STATUS

- Encoder direction and tested detent counts: **PASS**.
- Encoder operation during active Modbus traffic: **PASS**.
- Button press/release reporting: **PASS**.
- Full post-change regression: **PENDING**.


## CHECKPOINT-50 — Full regression after encoder integration PASS

### USER-REPORTED TEST — FACT

The user reports successful completion of the full regression requested after the encoder sound-feedback change:

- Modbus Slave function regression FC01/02/03/04/05/06/15/16: **PASS**.
- OLED behavior checks: **PASS**.
- RX/TX/ERROR and HEARTBEAT indicator checks: **PASS**.
- No regression was observed in the tested behavior.

This checkpoint records the user's overall PASS report. Individual test logs, exact traffic duration and detailed OLED/indicator observations were not supplied and are not independently asserted here.

### PREVIOUS ENCODER ACCEPTANCE

- Exactly 20 CW events for 20 clockwise detents and 20 CCW events for 20 counter-clockwise detents: **PASS** (CHECKPOINT-49B).
- Encoder continued to register rotation during active Modbus traffic: **PASS** (CHECKPOINT-49B).
- Button press/release reporting and encoder sound feedback: **PASS** (CHECKPOINT-49A/49B).

### DECISION

- Accept the current Nano firmware, including encoder input and sound feedback, as the working baseline.
- Do not change the Modbus, OLED refresh, sleep/wake or indicator implementations while adding menu navigation unless a reproducible regression requires it.
- Begin the menu milestone as a separate UI/navigation layer.

### NEXT STEP

Define the minimal menu interaction model for the existing 128×64 U8X8 OLED and KY-040 encoder. Then implement and test navigation without changing Modbus behavior.

### STATUS

- Encoder acceptance: **PASS**.
- Full requested post-encoder regression: **PASS**, user-reported.
- Menu architecture and interaction specification: **NEXT**.
- Menu implementation: **NOT STARTED**.


## CHECKPOINT-51 — Inline main-screen settings editor implemented (hardware verification pending)

### CHANGE

The Nano Slave tester now has an encoder-driven settings editor integrated into the existing OLED status screen. No separate STATUS / SETTINGS / ABOUT menu was added.

Editable parameters:
- Slave ID: 1–247.
- RS-232 / Modbus baud rate: 1200, 2400, 4800, 9600 or 19200.
- Serial format remains fixed at 8N1.

Encoder interaction:
1. Rotate while not editing to select Slave ID or baud rate.
2. Press the encoder button to enter editing.
3. Rotate to change the staged value.
4. Press again to commit.
5. Committed values are stored in EEPROM and loaded at startup.

The second OLED row displays SELECT / EDIT status. The remaining diagnostic rows are retained.

### IMPLEMENTATION

- Added project-owned `settings.cpp/.h` using the AVR EEPROM API, with magic, version and checksum validation.
- Added encoder event retrieval while preserving the existing serial diagnostics and sound feedback.
- Added the inline editor state to the display module.
- Added runtime Slave ID update to the Slave module.
- Added runtime SoftwareSerial baud reconfiguration.
- Corrected Modbus RTU t1.5/t3.5 calculation to use the active baud and UART character format passed to `ModbusRTU::begin()`, rather than the old compile-time 9600/8N1 constants.
- External libraries were not modified.

### DECISIONS

- Keep the compact Nano firmware Slave-only.
- Keep all controls on the existing main screen.
- Keep 8N1 fixed for this revision.
- Limit selectable baud rates to 1200–19200 pending physical verification of SoftwareSerial at each supported rate.
- Save only confirmed changes to EEPROM.

### SOURCE STATUS

The changed and added source files were committed to GitHub. Post-write fetches confirmed the current source for the main sketch, display, encoder, settings, RS-232 port, Modbus RTU and Modbus Slave modules.

### TEST

- GitHub writes: PASS.
- Source read-back: PASS for the files inspected after updates.
- Arduino Nano compilation: NOT RUN in this checkpoint.
- Flash/SRAM measurement: PENDING.
- Encoder selection/edit/commit interaction: PENDING.
- Slave ID change verified with a Modbus master: PENDING.
- Baud switching at all five rates: PENDING.
- EEPROM persistence after power cycle: PENDING.
- Full FC01/02/03/04/05/06/15/16, OLED sleep/wake and indicator regression: PENDING.

### ACCEPTANCE CRITERIA

1. Compile for Arduino Nano / ATmega328P with the existing libraries; no external library modifications.
2. Verify selection of both settings and correct clockwise/counter-clockwise editing.
3. Set Slave ID to a non-default value; confirm the tester responds only at the new ID and retains the value after power cycle.
4. Test each listed baud rate with a matching Modbus master; verify correct request/response framing and CRC.
5. Change baud rate and Slave ID repeatedly; verify no lockup and no stale display values.
6. Verify the original Modbus function regression, OLED sleep/wake, RX/TX/ERROR/HEARTBEAT indications and encoder sound feedback.
7. Record final Flash/SRAM usage and compare with CHECKPOINT-49A (20,532 bytes Flash / 1,506 bytes global SRAM / 542 bytes compiler-reported remaining).

### NEXT STEP

Compile and upload this revision, then perform the acceptance tests above. Do not add further features until this checkpoint is accepted.

### STATUS

- Inline settings editor source: IMPLEMENTED.
- EEPROM persistence source: IMPLEMENTED.
- Runtime Slave ID and baud source: IMPLEMENTED.
- Build and hardware acceptance: PENDING.
- Simplified Nano tester completion: PENDING acceptance.


## CHECKPOINT-52 — Encoder gesture model and RX/TX reset implemented (verification pending)

### USER-CONFIRMED INTERACTION MODEL

The user explicitly confirmed the following control behavior:

**Outside the editor**
- Rotation: no UI action; existing screen wake and encoder sound feedback remain.
- Short press: no UI action; existing screen wake and press sound feedback remain.
- Long press (>2 seconds): enter the settings editor.
- Double press: reset RX and TX counters. RATE is recalculated from the reset counters.

**Inside the editor**
- Rotation while browsing: select the parameter.
- Short press while browsing: enter value adjustment for the selected parameter.
- Rotation while adjusting: change the staged value.
- Next short press: confirm the selected value and save it to EEPROM.
- Long press (>2 seconds): exit the editor. An unconfirmed staged value is discarded; already confirmed values remain saved.
- Double press: no action.

### CHANGE

- Replaced the encoder's single button-press event with non-blocking SHORT / DOUBLE / LONG event classification.
- Kept the project-owned 30 ms debounce and existing encoder sound feedback.
- Long press is detected after 2000 ms without using delay().
- Double-click recognition uses a 350 ms window from the first stable release to the start of the second stable press; the second press may finish after that window.
- Updated the OLED editor state machine to distinguish parameter browsing from value adjustment.
- Rotation outside the editor no longer changes the selected setting or opens the editor.
- Added `ModbusSlave::resetTrafficCounters()`, which clears RX and TX only. ERR and MISS are deliberately retained.
- Connected double-click reset to the main loop. RATE is not stored separately; it is derived from RX/TX and therefore returns to its no-sample display after reset.
- No external library was modified. Modbus protocol processing and RS-232 transport were not changed.

### SOURCE REVIEW / GITHUB

- Fresh current GitHub source was fetched before the changes.
- Encoder, display, main sketch and Modbus Slave files were updated.
- Every changed source file was fetched back and compared with the submitted content: **PASS**.
- Project documentation was updated and fetched back: **PASS**.
- Search for the removed `takeButtonPress` API returned no indexed code matches; repository code-search indexing reported incomplete results, so this is not treated as a complete repository-wide proof.

### TEST

- GitHub writes and exact read-back verification: **PASS**.
- Arduino Nano / ATmega328P compilation: **NOT RUN**.
- Flash/SRAM measurement: **PENDING**.
- Long press entry and exit: **PENDING**.
- Outside-editor rotation/short-press no-op behavior, wake and sound: **PENDING**.
- Parameter browsing and value adjustment/confirmation: **PENDING**.
- Double-click reset of RX/TX and automatic RATE update: **PENDING**.
- ERR/MISS retention after reset: **PENDING**.
- EEPROM persistence after power cycle: **PENDING**.
- Full Modbus FC01/02/03/04/05/06/15/16, OLED sleep/wake and indicator regression: **PENDING**.

### ACCEPTANCE TEST

1. Compile for Arduino Nano / ATmega328P with the existing library set; record Flash/SRAM.
2. Outside the editor, rotate and short-press: confirm no editor or parameter-selection change; confirm wake and sound feedback remain.
3. Hold the button for more than 2 seconds: confirm editor entry. Repeat to confirm exit.
4. Inside the editor, rotate between Slave ID and baud rate. Short-press to enter value adjustment; rotate to change; short-press again to confirm.
5. Exit and re-enter the editor; confirm the selected value is retained. Power-cycle and confirm EEPROM persistence.
6. Generate nonzero RX/TX/ERR/MISS values, double-click outside the editor, and confirm RX=0, TX=0, RATE shows no sample, while ERR and MISS remain unchanged.
7. Double-click inside the editor; confirm it does not reset counters or trigger a value action.
8. Run the complete Modbus FC01/02/03/04/05/06/15/16 regression and confirm OLED, encoder sound, RX/TX/ERROR and HEARTBEAT behavior.

### DECISION

- Accept the user-confirmed gesture model as the required UI behavior.
- Keep RX/TX reset separate from ERR/MISS diagnostic history.
- Do not add further UI features until this checkpoint passes compile and hardware acceptance.

### STATUS

- Gesture classifier and editor state machine: **IMPLEMENTED**.
- RX/TX reset integration: **IMPLEMENTED**.
- GitHub source and documentation read-back: **PASS**.
- Compile, memory measurement and hardware acceptance: **PENDING**.


## CHECKPOINT-53 — RATE denominator separated from total RX; OLED identifies Slave mode

### USER-APPROVED CHANGE

The user approved two focused changes:
1. RATE must not fall when the Modbus master polls other Slave IDs or sends broadcast requests.
2. The OLED title must identify the active Nano operating mode.

### CHANGE

- Added the internal `rateRequestCount` counter.
- Increment it only after a successful RTU receive and CRC validation, and only when the destination address equals the configured local Slave ID.
- Increment occurs before request-length validation. Therefore, a CRC-valid malformed request addressed to this Slave counts as an attempt and lowers RATE if no response is transmitted.
- Foreign destination IDs and broadcast address 0 are excluded from the RATE denominator.
- RX semantics are unchanged: every completed receive event, including CRC/framing errors, still increments RX.
- TX semantics are unchanged: TX increments only when the response is successfully handed to the serial transport.
- RATE is now `TX / rateRequestCount × 100%`; a zero denominator displays the existing no-sample marker.
- Double-click reset clears RX, TX and `rateRequestCount`; ERR and MISS remain unchanged.
- The OLED header is now `CommTester SLAVE` (16 characters).
- The counter is passed through the existing display-data interface and cached for incremental OLED redraw.
- No external library, protocol handler, serial transport, pin assignment or encoder behavior was changed.

### RATE SEMANTICS

- **RX:** all completed receive events on the line.
- **TX:** successfully transmitted Slave responses.
- **RATE denominator:** CRC-valid frames addressed to the configured local Slave ID, including malformed local requests.
- **Excluded from RATE denominator:** CRC/framing failures, foreign destination IDs and broadcast address 0.
- **ERR/MISS:** existing classification is unchanged. In particular, malformed frames are ERR; valid structurally complete requests for foreign/broadcast IDs are MISS.

### SOURCE STATUS

- Fresh GitHub source was fetched before each file update.
- Updated source files:
  - `firmware/CommTester_Bench/modbus_slave.cpp`
  - `firmware/CommTester_Bench/modbus_slave.h`
  - `firmware/CommTester_Bench/display.cpp`
  - `firmware/CommTester_Bench/display.h`
  - `firmware/CommTester_Bench/CommTester_Bench.ino`
- `PROJECT.md` and this project log were updated.
- Every changed file was fetched back immediately after its write and compared byte-for-byte with the submitted content: **PASS**.
- External libraries were not modified.

### TEST

- GitHub source updates: **PASS**.
- Exact post-write read-back verification: **PASS**.
- Arduino Nano / ATmega328P compilation: **PENDING**.
- Flash/SRAM measurement: **PENDING**.
- Hardware RATE behavior for local, foreign, broadcast and malformed local requests: **PENDING**.
- Double-click reset with ERR/MISS retention: **PENDING**.
- OLED title visibility: **PENDING**.
- Full Modbus FC01/02/03/04/05/06/15/16 and OLED/encoder/indicator regression: **PENDING**.

### ACCEPTANCE TEST

1. Compile for Arduino Nano / ATmega328P with the existing library set; record Flash/SRAM.
2. Poll the configured local Slave repeatedly with valid requests. If every response is transmitted, RATE should show 100.0%.
3. Poll one or more foreign Slave IDs. RX and MISS should increase; RATE should remain unchanged.
4. Send broadcast address 0 requests. RX and MISS should increase; RATE should remain unchanged.
5. Send a CRC-valid malformed request addressed to the local Slave. ERR and the RATE denominator should increase; if no response is sent, RATE should decrease.
6. Send a bad-CRC frame. RX and ERR should increase; RATE denominator should not change.
7. Double-click outside the editor. RX, TX and RATE denominator should reset; ERR and MISS should retain their previous values; RATE should show no sample until a new local request arrives.
8. Confirm the OLED title reads exactly `CommTester SLAVE`.
9. Run the complete FC01/02/03/04/05/06/15/16 regression and confirm OLED refresh/sleep/wake, encoder gestures/sounds and RX/TX/ERROR/HEARTBEAT indications.

### NEXT STEP

Compile and upload CHECKPOINT-53, then perform the acceptance tests above. Do not make further changes until the results are known.

### STATUS

- RATE accounting source: **IMPLEMENTED**.
- OLED Slave title source: **IMPLEMENTED**.
- GitHub write/read-back verification: **PASS**.
- Compilation, memory measurement and hardware acceptance: **PENDING**.


## CHECKPOINT-54 — Startup hang localization instrumentation

### CHANGE

- Added temporary serial markers inside `ModbusSlave::begin()` around RS-232 initialization, `ModbusRTU` object construction and the RTU startup call.
- Added temporary markers inside `ModbusRTU::begin()` at function entry, immediately before the RX-buffer quiet-time drain loop, and immediately after that loop.
- No protocol logic, timing calculation, pin assignment, external library or runtime behavior was intentionally changed. The markers are diagnostic only.

### FACT

The user's boot trace ends at `BOOT 5`, after `Encoder::begin()` and immediately before/at entry to `ModbusSlave::begin()`. Current source inspection shows an unbounded quiet-time drain loop at the end of `ModbusRTU::begin()`; this is a candidate cause, not yet a confirmed cause.

### TEST

- GitHub source inspection before instrumentation: PASS.
- Diagnostic source update and read-back: PASS.
- Hardware boot trace with the new markers: PASS, user-reported.
- Serial trace reached `RTU: ready`, `MS: RTU ready` and `BOOT 6`.
- User confirmed encoder response, audible feedback and activity indication after startup: PASS, user-reported.
- Arduino Nano / ATmega328P compile result and Flash/SRAM measurement: NOT REPORTED.
- Modbus function regression and remaining UI/indicator checks: PENDING.

### RESULT / INTERPRETATION

The tested boot completed through `setup()`. The startup path through RS-232 initialization, ModbusRTU object construction and the RX quiet-time drain loop returned successfully in this run.

This does not prove that the unbounded drain loop caused the earlier lockup; the earlier failure was not reproduced during this test. No functional fix has been applied.

### NEXT STEP

Run the post-change acceptance checks for settings/editor behavior, EEPROM persistence, Modbus FC01/02/03/04/05/06/15/16, traffic counters/RATE, OLED, encoder gestures/sounds and RX/TX/ERROR/HEARTBEAT indications. Record any failure before changing code.

### STATUS

- Startup diagnostic run: PASS.
- Root cause of the intermittent/non-reproduced startup issue: NOT CONFIRMED.
- Functional regression after CHECKPOINT-53: PENDING.
- Diagnostic markers: TEMPORARY; remove only after startup stability and regression are accepted.


## CHECKPOINT-55 — OLED update-path diagnostic instrumentation

### CHANGE

- Added temporary Serial markers before and after OLED row writes through U8X8, distinguishing ordinary rows from PROGMEM rows.
- Added markers around display clear, wake and sleep power-save calls.
- Added markers around the UI status redraw triggered by encoder events, including the button-event value and editor state.
- No OLED layout, encoder state machine, Modbus handling, I2C configuration, external library or functional behavior was intentionally changed.

### WHY

The user reports that the OLED can stop updating while Serial diagnostics indicate that the main firmware and Modbus receive path continue to run. The instrumentation is intended to identify whether execution stalls inside a specific U8X8/I2C operation or whether all OLED calls return normally.

### SOURCE STATUS

- Fresh current GitHub source fetched before modification: PASS.
- OLED diagnostic instrumentation committed: PASS.
- Immediate exact read-back of `firmware/CommTester_Bench/display.cpp`: PASS.
- Commit: `d1d89137cf86f9e8416ce66fe91fec814194808e`.

### TEST

- Hardware compilation and upload: PENDING.
- Reproduce the freeze by pressing the encoder and/or double-clicking: PENDING.
- Capture the last OLED diagnostic marker before the screen stops: PENDING.
- Confirm Modbus packet processing continues during the frozen-screen condition: PENDING.

### NEXT STEP

Compile and upload this diagnostic revision. Reproduce the display freeze once, then send the Serial Monitor output beginning at the last encoder event and ending after the freeze. Do not change the OLED or Modbus logic until the blocking operation is identified.

### STATUS

- Diagnostic instrumentation: IMPLEMENTED.
- Root cause: NOT CONFIRMED.
- Functional fix: NOT APPLIED.


## CHECKPOINT-56 — Restore pre-instrumentation OLED baseline

### CHANGE
- Restored `firmware/CommTester_Bench/display.cpp` exactly to its state immediately before CHECKPOINT-55 (parent commit `91cca9cb5fe1066360b6f566578558f4d2aaf78b`).
- Removed all temporary OLED Serial markers introduced by CHECKPOINT-55.
- No display logic, encoder logic, Modbus logic or external library was changed.

### WHY
The user reported that the CHECKPOINT-55 diagnostic build introduced a new startup failure (partial OLED bars followed by a black screen, no startup melody or encoder response), while the pre-CHECKPOINT-55 version starts normally but later freezes after encoder interaction. The diagnostic build is therefore not a valid baseline for reproducing the original fault.

### SOURCE VERIFICATION
- Pre-change current file fetched from `main`: PASS.
- Baseline source fetched from parent commit: PASS.
- Restore committed: 259c0f84f4f98115306b484eb364cb1c7461cdbf.
- Immediate exact read-back comparison against the pre-CHECKPOINT-55 source: PASS.
- Restored blob SHA: 3853f31b4f51e5d07b8518e317a0bdd7b60bbf74.

### TEST
- Hardware compile/upload after restore: NOT YET REPORTED.
- Startup melody and encoder initialization: PENDING user confirmation.
- Reproduction of the original delayed OLED freeze: PENDING.

### NEXT STEP
Add only targeted markers around the encoder-triggered UI redraw and OLED wake path, without instrumenting initial screen clear or every row write. Reproduce the original freeze and capture the last marker.

### STATUS
- Pre-CHECKPOINT-55 OLED source baseline restored in GitHub: PASS.
- Root cause of the original encoder-related freeze: NOT CONFIRMED.
- Functional fix: NOT APPLIED.


## CHECKPOINT-57 — Targeted encoder-to-OLED diagnostics

### CHANGE
- Starting from the restored pre-CHECKPOINT-55 `display.cpp`, added only four diagnostic markers:
  - `ENC_WAKE_BEGIN` / `ENC_WAKE_DONE` around OLED wake via `u8x8.setPowerSave(0)`.
  - `ENC_UI_BEGIN event=N` / `ENC_UI_DONE` around the encoder-triggered `drawUiStatus()` call.
- Did not instrument startup, `clearDisplay()`, generic row drawing, Modbus processing, or the main loop.
- No UI state logic, display layout, I2C configuration, pins or external libraries changed.

### WHY
The previous diagnostic build failed during initial screen drawing and altered the observed startup behavior. This revision preserves the known-good startup source and observes only the two OLED operations reachable from encoder handling: wake (when sleeping) and UI status redraw.

### SOURCE VERIFICATION
- Fresh current `display.cpp` fetched before modification: PASS.
- Source baseline confirmed equal to the pre-CHECKPOINT-55 parent: PASS.
- Diagnostic update committed: cd4566aa1189e5d35b0a72800710232b72451f83.
- Immediate exact read-back of `display.cpp`: PASS.
- Resulting blob SHA: 797e64775ef1c7b87a9c35da0b4e4c0294ab5265.
- Project log updated after source commit and fetched back: pending in this checkpoint.

### TEST
- Arduino Nano / ATmega328P compilation and upload: PENDING.
- Normal startup, startup melody and encoder feedback: PENDING.
- Reproduce the original freeze using the same encoder action sequence: PENDING.
- Capture the final `ENC_*` marker before the freeze: PENDING.

### INTERPRETATION GUIDE
- `ENC_UI_BEGIN` without `ENC_UI_DONE`: execution stopped during `drawUiStatus()` / its U8X8 row write.
- `ENC_WAKE_BEGIN` without `ENC_WAKE_DONE`: execution stopped during OLED wake / U8X8 power-save command.
- Both BEGIN/DONE pairs complete before the freeze: the failure is elsewhere; do not infer an OLED call is responsible.

### NEXT STEP
Compile and upload this revision. Confirm normal startup first. Then reproduce the original freeze once and provide the Serial Monitor output from the last encoder event through the freeze. Do not change hardware or libraries.

### STATUS
- Targeted diagnostics committed and read-back verified: PASS.
- Compilation and hardware reproduction: PENDING.
- Root cause and functional fix: NOT CONFIRMED / NOT APPLIED.


## CHECKPOINT-58 — Remove firmware Serial diagnostics for stability A/B test

### USER-REQUESTED CHANGE
Remove Serial output to determine whether diagnostic UART traffic contributes to the unstable startup and OLED freeze.

### CHANGE
Removed all firmware `Serial.print()`, `Serial.println()` and `Serial.begin()` calls from the current `firmware/CommTester_Bench` source:
- `CommTester_Bench.ino`: removed UART initialization and startup banner.
- `display.cpp`: removed the temporary `ENC_WAKE_*` and `ENC_UI_*` markers.
- `encoder.cpp`: removed encoder event/debug messages.
- `modbus_rtu.cpp`: removed RTU startup markers.
- `modbus_slave.cpp`: removed Slave startup, frame dump, CRC/error, response and transmission diagnostics.
- No Modbus state handling, OLED calls, encoder logic, timing, pin assignment, EEPROM behavior or protocol logic was intentionally changed.
- No external library was modified.
- The repository firmware directory contains no Serial API calls after this change (18 source/header/sketch files scanned).

### WHY
The user reports repeated boot sequences, slow/unstable startup, OLED flicker and a near-immediate display freeze. The user suspects resource pressure. Removing diagnostic UART traffic is a controlled test of whether Serial output/timing contributes to the symptoms. It does not by itself prove or disprove SRAM exhaustion.

### SOURCE VERIFICATION
- All five changed files were fetched from current `main` before editing.
- Every file update was committed sequentially and immediately fetched back; exact content comparison: PASS.
- Current source scan across all 18 firmware source/header/sketch files: no `Serial.` references found.
- External libraries unchanged.

### TEST
- Arduino Nano / ATmega328P compilation: PENDING user run.
- Flash/SRAM report: PENDING.
- Hardware startup stability and OLED freeze test: PENDING.
- Modbus request/response regression after diagnostics removal: PENDING.

### IMPORTANT TEST CONTROL
The user's local `BOOT 1...` markers were added locally and are not present in the repository sketch. For this A/B test, synchronize the complete updated sketch and all five changed files from `main`; do not retain local `Serial.println("BOOT ...")` instrumentation. The current repository sketch no longer initializes the USB Serial interface because the current firmware tree has no Serial console consumer.

### NEXT STEP
Compile and flash the synchronized repository version. Observe startup melody, OLED stability, encoder response and Modbus operation without Serial diagnostics. If instability remains, collect the compiler's Flash/SRAM usage and investigate reset cause/power integrity as separate hypotheses.

### STATUS
- Serial diagnostics removed from repository firmware: PASS.
- Source read-back verification: PASS.
- Build, memory measurement and hardware A/B test: PENDING.
- Root cause: NOT CONFIRMED.


## CHECKPOINT-59 — Hardware result: stable operation after Serial diagnostics removal

### USER-REPORTED TEST RESULT
After flashing the CHECKPOINT-58 source with all local BOOT markers removed, the user reports:
- Device operation is stable so far.
- The previously observed unstable startup / repeated boot behavior and near-immediate OLED freeze have not reappeared during the reported test.

### RESULT
- Serial diagnostics removal as a stability experiment: **PASS (initial hardware observation)**.
- Stability over extended runtime and full Modbus regression: **NOT YET VERIFIED**.
- Exact root cause: **NOT CONFIRMED**. The result does not distinguish SRAM pressure from UART timing/load or another timing-sensitive interaction.

### DECISION
- Keep the no-debug-Serial firmware as the current baseline.
- Do not add diagnostic Serial output back or make further code changes while the device is stable.
- Do not claim SRAM exhaustion as the cause without a measured build report or controlled experiment.

### NEXT STEP
Continue normal use and observe startup, OLED refresh, encoder gestures, Modbus RX/TX and indicators. If the fault returns, capture the exact reproduction sequence and use a non-intrusive diagnostic method or compiler memory report before changing functional code.

### STATUS
- CHECKPOINT-58 source and read-back verification: **PASS**.
- Initial user-reported hardware stability after CHECKPOINT-58: **PASS**.
- Extended soak test / full regression: **PENDING**.


## CHECKPOINT-60 — Invalid CRC rejection and valid-frame recovery PASS

### USER-REPORTED HARDWARE TEST

The user ran `modbus_test_crc.py` against the Nano Slave on COM7 with 9600 baud, 8N1, Slave ID 1.

Test sequence and observed results:

1. A valid FC03 request for three holding registers was sent before the invalid frame. The Slave returned a valid response containing register values [100, 200, 300]: **PASS**.
2. The same request with one CRC byte deliberately changed (good frame ending `05 CB`, bad frame ending `05 CC`) was sent. No response was received: **PASS**.
3. The original valid FC03 request was sent again after the CRC error. The Slave returned the same valid register values [100, 200, 300]: **PASS**.

The test summary reported OVERALL RESULT: PASS.

### FACT

- The tested Slave rejected the deliberately corrupted CRC frame without transmitting a response.
- A subsequent valid FC03 request was accepted and answered correctly.
- The three tested holding-register values were unchanged across the invalid-CRC event.
- This confirms functional recovery for the tested sequence; it does not establish recovery for every possible framing, timing or buffer-overrun condition.

### TEST SCOPE

- Interface: COM7, 9600 baud, 8N1.
- Slave ID: 1.
- Function code: FC03.
- Invalidity introduced: CRC low byte changed from `CB` to `CC`.
- Other Modbus function codes and extended soak behavior were not part of this test.

### STATUS

- Valid request before invalid CRC: **PASS**.
- Invalid CRC rejection (no response): **PASS**.
- Valid request recovery after invalid CRC: **PASS**.
- Register data unchanged before/after invalid CRC: **PASS**.



## CHECKPOINT-61 — Extended hardware stability test PASS

### USER-REPORTED TEST RESULT

The user reports that the extended runtime / long-duration test of the current Nano firmware baseline completed successfully.

### FACT

- The extended test completed without a reported failure: **PASS (user-reported)**.
- Together with CHECKPOINT-59, the current no-debug-Serial baseline has now passed both the initial stability observation and the subsequent long-duration test.
- Together with CHECKPOINT-60, invalid-CRC rejection and recovery after the invalid frame have also passed.

### SCOPE / LIMITATIONS

- Exact test duration, detailed sequence of exercised UI actions, and traffic volume were not provided; they are therefore not specified here.
- This result confirms the reported long-duration test only. It does not replace a complete post-baseline regression of every supported Modbus function code.
- The original cause of the earlier freezes/restarts remains **NOT CONFIRMED**.

### DECISION

- Keep the current no-debug-Serial firmware as the stable Nano baseline.
- Do not modify firmware or external libraries on the strength of this successful test.
- Treat extended runtime stability as closed for the tested conditions; continue with the remaining Nano acceptance-test items.

### STATUS

- Initial stability observation after Serial diagnostics removal (CHECKPOINT-59): **PASS**.
- Invalid CRC rejection and subsequent valid-frame recovery (CHECKPOINT-60): **PASS**.
- Extended runtime / long-duration test: **PASS (user-reported)**.
- Complete post-baseline Modbus function-code regression: **PENDING**, unless separately confirmed by test records.
- Root cause of the earlier instability: **NOT CONFIRMED**.


## CHECKPOINT-62 — Complete Modbus RTU function regression PASS

### USER-REPORTED HARDWARE TEST

The user ran `modbus_test_all.py` against the current Nano Slave baseline.

Test configuration:
- Interface: COM7.
- Baud rate: 9600.
- Serial format: 8N1.
- Slave ID: 1.
- Overall result reported by the test script: **PASS**.

### VERIFIED TEST CASES

| Function | Test | Result |
|---|---|---|
| FC01 | Read 16 coils, address 0 | **PASS** |
| FC02 | Read 16 discrete inputs, address 0 | **PASS** |
| FC03 | Read 5 holding registers, address 0; expected [100, 200, 300, 400, 500] | **PASS** |
| FC03 | Read 5 holding registers, address 120; expected [12100, 12200, 12300, 12400, 12500] | **PASS** |
| FC04 | Read 5 input registers, address 0; expected [1000, 1001, 1002, 1003, 1004] | **PASS** |
| FC04 | Read 5 input registers, address 120; expected [1120, 1121, 1122, 1123, 1124] | **PASS** |
| FC05 | Write single coil and read back ON state | **PASS** |
| FC06 | Write single holding register (12345) and read back | **PASS** |
| FC15 | Write 16 coils and read back bit pattern | **PASS** |
| FC16 | Write 5 holding registers [1111, 2222, 3333, 4444, 5555] and read back | **PASS** |

The script reported every listed test as PASS and ended with `OVERALL RESULT: PASS`.

### FACT

- All eight supported Modbus function codes (FC01, FC02, FC03, FC04, FC05, FC06, FC15, FC16) passed the reported hardware tests.
- FC03 and FC04 passed at both address 0 and address 120 for the tested five-register ranges.
- FC05, FC06, FC15 and FC16 write responses were followed by read-back checks; returned values/states matched the expected values.
- No firmware or external-library changes were made for this test.

### SCOPE / LIMITATIONS

- This checkpoint records the exact test cases and configuration shown in the user's console output.
- It does not establish behavior for untested boundary conditions, maximum request sizes, malformed payloads other than the CRC case recorded in CHECKPOINT-60, or other baud rates.
- The test does not independently verify electrical signal quality or interoperability with every third-party Modbus device.

### DECISION

- Mark the supported Modbus function-code regression as **PASS** for the tested baseline and conditions.
- Together with CHECKPOINT-61 (extended runtime stability) and CHECKPOINT-60 (CRC rejection/recovery), the current Nano Slave baseline has passed the principal functional and stability checks recorded so far.
- Keep the firmware and external libraries unchanged; preserve this revision as the current stable baseline.

### STATUS

- FC01 / FC02 / FC03 / FC04 / FC05 / FC06 / FC15 / FC16 regression: **PASS (user-reported hardware test)**.
- Extended runtime stability (CHECKPOINT-61): **PASS**.
- Invalid CRC rejection and valid-frame recovery (CHECKPOINT-60): **PASS**.
- Nano Slave baseline: **FUNCTIONAL REGRESSION PASS for tested conditions**.


## CHECKPOINT-63 — Slave ID editor and EEPROM persistence PASS

### USER-REPORTED HARDWARE TEST

The user completed all four acceptance conditions for changing the Modbus Slave ID through the encoder editor:

1. The newly selected Slave ID was displayed after confirmation: **PASS**.
2. Requests addressed to the previous Slave ID received no response: **PASS**.
3. Requests addressed to the new Slave ID received a response: **PASS**.
4. The new Slave ID remained active after a power cycle, confirming EEPROM persistence for this test: **PASS**.

### RESULT

- Encoder-based Slave ID selection, confirmation and runtime application: **PASS (user-reported hardware test)**.
- Rejection of requests addressed to the previous ID: **PASS**.
- Response under the newly configured ID: **PASS**.
- Persistence of the configured ID across power cycling: **PASS**.

### SCOPE / LIMITATIONS

This checkpoint records the four acceptance conditions reported by the user. The specific old/new ID values, Modbus master settings and number of repetitions were not provided and are intentionally not inferred. Baud-rate switching and its persistence remain separate acceptance tests.

### DECISION

- Accept the Slave ID editor and EEPROM persistence test for the reported conditions.
- No firmware change was required for this test.
- Preserve the current firmware baseline.

### STATUS

- Slave ID editor and runtime update: **PASS**.
- Old-ID rejection / new-ID response: **PASS**.
- EEPROM persistence after power cycle: **PASS**.
- Baud-rate editor and runtime switching: **PENDING**.


## CHECKPOINT-64 — Baud-rate switching and Modbus communication PASS

### USER-REPORTED HARDWARE TEST

The user confirmed that QModMaster successfully exchanged Modbus packets with CommTester at each of the five selectable baud rates:

- 1200 baud: **PASS**
- 2400 baud: **PASS**
- 4800 baud: **PASS**
- 9600 baud: **PASS**
- 19200 baud: **PASS**

### RESULT

- Encoder-based baud-rate selection and confirmation: **PASS** (all five values selectable).
- Runtime UART baud-rate change: **PASS** for all five tested rates.
- Modbus request/response communication with QModMaster: **PASS** at every tested rate.

### SCOPE / LIMITATIONS

This checkpoint records successful communication reported by the user. The serial format, Slave ID, number of transactions per rate and persistence of the selected baud rate after a power cycle were not separately specified; they are not inferred here.

### DECISION

- Accept baud-rate switching and Modbus communication at all five supported rates for the reported hardware test.
- No firmware or external-library changes were required.
- Preserve the current firmware baseline.

### STATUS

- Slave ID editor and EEPROM persistence (CHECKPOINT-63): **PASS**.
- Baud-rate selection and runtime communication at 1200 / 2400 / 4800 / 9600 / 19200 baud: **PASS**.
- Baud-rate EEPROM persistence after power cycle: **NOT YET CONFIRMED**.


## CHECKPOINT-65 — EEPROM settings persistence confirmed

### USER-REPORTED HARDWARE RESULT

The user confirmed that settings data are saved in EEPROM.

In the context of CHECKPOINT-63 and CHECKPOINT-64, this closes the remaining persistence check for the configurable Slave ID and baud-rate settings.

### RESULT

- Slave ID persistence in EEPROM: **PASS** (CHECKPOINT-63; user-reported power-cycle test).
- Baud-rate setting persistence in EEPROM: **PASS** (user confirmation).
- Runtime Modbus communication at all five supported baud rates: **PASS** (CHECKPOINT-64).

### SCOPE / LIMITATIONS

The user did not provide a separate power-cycle sequence or the exact stored values for this confirmation. No additional details are inferred.

### DECISION

- Accept EEPROM persistence of the configurable settings for the reported hardware tests.
- No firmware or external-library changes were required.
- Preserve the current firmware baseline.

### STATUS

- Slave ID editor and persistence: **PASS**.
- Baud-rate selection and Modbus communication at 1200 / 2400 / 4800 / 9600 / 19200 baud: **PASS**.
- EEPROM settings persistence: **PASS (user-reported)**.


## CHECKPOINT-66 — Traffic counter reset gesture PASS

### USER-REPORTED HARDWARE TEST

The user performed a double-click on the encoder button while outside the settings editor.

Observed OLED results:

- RX counter reset to zero: **PASS**.
- TX counter reset to zero: **PASS**.
- RATE returned to its no-sample state: **PASS**.
- ERR counter retained its previous value: **PASS**.
- MISS counter retained its previous value: **PASS**.

### RESULT

The double-click traffic-reset behavior matches the intended implementation. No firmware changes were required.

### SCOPE / LIMITATIONS

This checkpoint records the reported hardware observation. It does not independently verify counter behavior under a simultaneous incoming Modbus transaction or double-click timing at debounce boundaries.

### DECISION

- Accept the double-click traffic counter reset for the tested conditions.
- Preserve the current firmware baseline.

### STATUS

- RX/TX reset: **PASS**.
- RATE denominator reset / no-sample display: **PASS**.
- ERR/MISS retained: **PASS**.


## CHECKPOINT-67 — Encoder gestures outside editor PASS

### USER-REPORTED HARDWARE TEST

The user completed the four outside-editor encoder gesture checks:

1. Clockwise and counter-clockwise rotation did not change displayed parameters: **PASS**.
2. Short button press did not open the editor: **PASS**.
3. Long press (>2 seconds) opened the editor and displayed the expected entry state: **PASS**.
4. A subsequent long press exited the editor and returned to the normal Slave screen: **PASS**.

The user also confirmed that the device continued to respond to Modbus requests during the test: **PASS**.

### RESULT

Outside-editor encoder gesture behavior and Modbus responsiveness passed the reported hardware test. No firmware changes were required.

### SCOPE / LIMITATIONS

This checkpoint covers the four specified gestures outside the editor and Modbus responsiveness during that test. It does not yet confirm rotation/short-press behavior inside the editor, double-click behavior inside the editor, or display/indicator regression.

### DECISION

- Accept outside-editor encoder gestures for the tested conditions.
- Preserve the current firmware baseline.

### STATUS

- Outside-editor rotation: **PASS**.
- Outside-editor short press: **PASS**.
- Long-press editor entry: **PASS**.
- Long-press editor exit: **PASS**.
- Modbus responsiveness during gesture test: **PASS**.


## CHECKPOINT-68 — Encoder editor interaction PASS

### USER-REPORTED HARDWARE TEST

The user completed the editor interaction checks:

1. Long press opened the editor at `EDIT: SLAVE ID`: **PASS**.
2. Encoder rotation switched selection between `SLAVE ID` and `BAUD RATE`: **PASS**.
3. Short press entered the selected value-adjustment state (`VAL`): **PASS**.
4. Rotation changed the staged value; reverse rotation restored the original value: **PASS**.
5. Short press confirmed the restored value and exited value-adjustment state: **PASS**.
6. The same adjustment/restore/confirm sequence worked for `BAUD RATE`: **PASS**.
7. Double-click inside the editor had no action and did not reset traffic counters or alter settings: **PASS**.
8. Long press exited the editor: **PASS**.

The user confirmed that the saved Slave ID and baud rate remained unchanged and Modbus communication continued during the test: **PASS**.

### RESULT

The reported editor browsing, value adjustment, confirmation, no-op double-click and exit behavior passed. No firmware changes were required.

### SCOPE / LIMITATIONS

This test deliberately restored each staged value before confirmation. It verifies the editor interaction path without changing the saved settings. It does not independently verify display behavior during a prolonged editing session or every wrap-around boundary.

### DECISION

- Accept the encoder editor interaction for the tested conditions.
- Preserve the current firmware baseline.

### STATUS

- Editor entry / parameter selection: **PASS**.
- Slave ID value adjustment and confirmation: **PASS**.
- Baud-rate value adjustment and confirmation: **PASS**.
- Double-click inside editor: **PASS (no action)**.
- Editor exit: **PASS**.
- Saved settings unchanged: **PASS**.
- Modbus responsiveness during test: **PASS**.


## CHECKPOINT-69 — RX/TX LED and sound indication PASS

### USER-REPORTED HARDWARE TEST

The user exercised Modbus request/response traffic through QModMaster and confirmed all RX/TX indication criteria:

- RX LED (D2) activates on received requests: **PASS**.
- TX LED (D3) activates on transmitted responses: **PASS**.
- RX and TX LED indications are distinguishable and operate independently: **PASS**.
- RX and TX audible signals are different: **PASS**.
- Indications are brief and return to the inactive state: **PASS**.
- Repeated requests do not cause a lockup: **PASS**.
- Modbus communication remains operational: **PASS**.

### RESULT

RX/TX visual and audible indication passed the reported hardware test. No firmware changes were required.

### SCOPE / LIMITATIONS

This checkpoint covers RX/TX indication during the tested QModMaster exchange. ERROR indication and HEARTBEAT timing remain separate checks.

### DECISION

- Accept RX/TX LED and sound indication for the tested conditions.
- Preserve the current firmware baseline.

### STATUS

- RX LED / sound: **PASS**.
- TX LED / sound: **PASS**.
- Repeated Modbus traffic and continued responsiveness: **PASS**.
- ERROR LED / sound: **PENDING**.
- HEARTBEAT LED: **PASS** (CHECKPOINT-71).


## CHECKPOINT-70 — ERROR LED and invalid-CRC recovery indication PASS

### USER-REPORTED HARDWARE TEST

The user ran the invalid-CRC test sequence and confirmed all four expected behaviors:

1. ERROR LED (D4) activates on the deliberately corrupted Modbus RTU request: **PASS**.
2. The audible error indication is produced: **PASS**.
3. The Slave does not respond to the invalid-CRC request: **PASS**.
4. A subsequent valid Modbus request receives a response: **PASS**.

### RESULT

ERROR visual/audible indication, invalid-CRC rejection and recovery on the next valid request passed the reported hardware test. No firmware changes were required.

### SCOPE / LIMITATIONS

This checkpoint covers the tested invalid-CRC sequence. It does not establish behavior for every malformed frame or all RTU framing/timing errors.

### DECISION

- Accept ERROR LED/sound indication and recovery for the tested invalid-CRC sequence.
- Preserve the current firmware baseline.

### STATUS

- ERROR LED: **PASS**.
- Audible error indication: **PASS**.
- Invalid-CRC request rejected without response: **PASS**.
- Subsequent valid request answered: **PASS**.
- HEARTBEAT LED: **PENDING**.

## CHECKPOINT-71 — HEARTBEAT hardware acceptance PASS

### USER-REPORTED HARDWARE TEST

The user confirmed that all five HEARTBEAT acceptance conditions passed:

1. Power-on startup indication: three synchronized LED flashes with the startup melody: **PASS**.
2. After the startup melody, D5 periodically flashes: **PASS**.
3. HEARTBEAT interval is approximately 2 seconds: **PASS**.
4. HEARTBEAT pulse duration is approximately 60 ms: **PASS**.
5. HEARTBEAT continues independently during Modbus traffic: **PASS**.

The user additionally confirmed that the device does not hang during the test.

### RESULT

HEARTBEAT startup and periodic hardware indication passed all five reported acceptance conditions. Device responsiveness was maintained. No firmware changes were required.

### SCOPE / LIMITATIONS

Timing values are user-observed as approximately 2 seconds and 60 ms; no instrumented timing measurement was reported. This checkpoint does not establish behavior under every possible traffic or power condition.

### DECISION

- Accept HEARTBEAT indication for the tested hardware conditions.
- Preserve the current Nano firmware baseline.

### STATUS

- Startup triple-flash and melody: **PASS**.
- Periodic D5 heartbeat: **PASS**.
- Approximate 2-second interval: **PASS**.
- Approximate 60 ms pulse: **PASS**.
- Independence during Modbus traffic: **PASS**.
- No device hang during test: **PASS**.

### NEXT STEP

Synchronize PROJECT.md with the verified Nano prototype state and review remaining milestone scope.
