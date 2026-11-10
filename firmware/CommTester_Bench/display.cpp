#include "display.h"
#include "settings.h"

#include <Arduino.h>
#include <U8x8lib.h>
#include <avr/pgmspace.h>

// SSD1306 128x64, hardware I2C, text-only mode
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

namespace {

  const unsigned long DISPLAY_SLEEP_TIMEOUT = 120000UL;

  bool displayReady = false;
  bool displaySleeping = false;
  bool displayNeedsRedraw = false;
  unsigned long lastActivityTime = 0;
  uint8_t displayedSlaveId = 1;
  uint8_t displayedMasterId = 0xFF;
  uint8_t displayedFunctionCode = 0;
  uint32_t displayedRx = 0;
  uint32_t displayedTx = 0;
  uint32_t displayedRateRequests = 0;
  uint32_t displayedErrors = 0;
  uint32_t missedCache = 0;
  uint8_t displayedBaudIndex = 3;

  // Inline editor state; no separate menu screen is used.
  uint8_t selectedSetting = 0; // 0 = Slave ID, 1 = baud rate
  bool editingSetting = false;
  bool changingValue = false;
  bool uiTouched = false;
  uint8_t editSlaveId = 1;
  uint8_t editBaudIndex = 3;

  // Immutable UI strings are kept in program Flash, not copied to SRAM.
  const char TEXT_HEADER[] PROGMEM = "CommTester SLAVE";
  const char TEXT_EMPTY[] PROGMEM = "";
  const char TEXT_NO_FUNCTION[] PROGMEM = "FC:--";
  const char DESC_UNKNOWN[] PROGMEM = "UNKNOWN";
  const char DESC_READ_COIL[] PROGMEM = "READ COIL";
  const char DESC_READ_DI[] PROGMEM = "READ DI";
  const char DESC_READ_HR[] PROGMEM = "READ HR";
  const char DESC_READ_IR[] PROGMEM = "READ IR";
  const char DESC_WRITE_COIL[] PROGMEM = "WRITE COIL";
  const char DESC_WRITE_HR[] PROGMEM = "WRITE HR";

  // Always write a complete 16-character row. This also removes stale
  // characters when a newly formatted value is shorter than the old one.
  void drawRow(uint8_t row, const char *text) {
    char line[17];
    uint8_t i = 0;

    while (i < 16 && text[i] != '\0') {
      line[i] = text[i];
      i++;
    }

    while (i < 16) {
      line[i++] = ' ';
    }

    line[16] = '\0';
    u8x8.drawString(0, row, line);
  }

  // Draw a fixed string directly from program Flash using a small stack buffer.
  void drawRow_P(uint8_t row, PGM_P text) {
    char line[17];
    uint8_t i = 0;
    char value;

    while (i < 16 && (value = (char)pgm_read_byte(text++)) != '\0') {
      line[i++] = value;
    }

    while (i < 16) {
      line[i++] = ' ';
    }

    line[16] = '\0';
    u8x8.drawString(0, row, line);
  }

  void appendUnsigned(char *text, uint8_t &position, uint32_t value) {
    char digits[10];
    uint8_t count = 0;

    do {
      digits[count++] = '0' + (value % 10);
      value /= 10;
    } while (value > 0 && count < sizeof(digits));

    while (count > 0) {
      text[position++] = digits[--count];
    }

    text[position] = '\0';
  }

  // Keep both counters on one 16-character OLED row.
  // Values above 9999 use compact k/M/G notation.
  void formatCompactCounter(char *out, uint32_t value) {
    uint32_t scaled;
    uint8_t pos = 0;

    if (value < 10000UL) {
      appendUnsigned(out, pos, value);
      return;
    }

    if (value < 1000000UL) {
      scaled = value / 1000UL;
      appendUnsigned(out, pos, scaled);
      out[pos++] = 'k';
    }
    else if (value < 10000000UL) {
      scaled = value / 100000UL;
      out[pos++] = '0' + (scaled / 10);
      out[pos++] = '.';
      out[pos++] = '0' + (scaled % 10);
      out[pos++] = 'M';
    }
    else if (value < 1000000000UL) {
      scaled = value / 1000000UL;
      appendUnsigned(out, pos, scaled);
      out[pos++] = 'M';
    }
    else {
      scaled = value / 100000000UL;
      out[pos++] = '0' + (scaled / 10);
      out[pos++] = '.';
      out[pos++] = '0' + (scaled % 10);
      out[pos++] = 'G';
    }

    out[pos] = '\0';
  }

  void drawTraffic(uint32_t requests, uint32_t responses) {
    char rx[5];
    char tx[5];
    char line[17] = "RX:";
    uint8_t pos = 3;

    formatCompactCounter(rx, requests);
    formatCompactCounter(tx, responses);

    for (uint8_t i = 0; rx[i] != '\0' && pos < 16; i++) {
      line[pos++] = rx[i];
    }

    line[pos++] = ' ';
    line[pos++] = 'T';
    line[pos++] = 'X';
    line[pos++] = ':';

    for (uint8_t i = 0; tx[i] != '\0' && pos < 16; i++) {
      line[pos++] = tx[i];
    }

    line[pos] = '\0';
    drawRow(4, line);
  }

  void drawFunction(uint8_t functionCode) {
    char line[17] = "FC:";
    uint8_t pos = 3;

    if (functionCode == 0) {
      drawRow_P(3, TEXT_NO_FUNCTION);
      return;
    }

    if (functionCode < 10) {
      line[pos++] = '0';
    }
    appendUnsigned(line, pos, functionCode);
    line[pos++] = ' ';

    PGM_P description = DESC_UNKNOWN;

    switch (functionCode) {
      case 1:  description = DESC_READ_COIL;  break;
      case 2:  description = DESC_READ_DI;    break;
      case 3:  description = DESC_READ_HR;    break;
      case 4:  description = DESC_READ_IR;    break;
      case 5:  description = DESC_WRITE_COIL; break;
      case 6:  description = DESC_WRITE_HR;   break;
      case 15: description = DESC_WRITE_COIL; break;
      case 16: description = DESC_WRITE_HR;   break;
    }

    char descriptionChar;
    while (pos < 16 &&
           (descriptionChar = (char)pgm_read_byte(description++)) != '\0') {
      line[pos++] = descriptionChar;
    }

    line[pos] = '\0';
    drawRow(3, line);
  }

  void appendAddress(char *line, uint8_t &pos, uint8_t address) {
    if (address >= 100) {
      line[pos++] = '0' + (address / 100);
      line[pos++] = '0' + ((address / 10) % 10);
      line[pos++] = '0' + (address % 10);
    }
    else {
      line[pos++] = '0' + (address / 10);
      line[pos++] = '0' + (address % 10);
    }
  }

  void drawAddressPair(uint8_t slaveId, uint8_t masterId) {
    char line[17] = "SLV:";
    uint8_t pos = 4;

    appendAddress(line, pos, slaveId);
    line[pos++] = ' ';
    line[pos++] = 'M';
    line[pos++] = 'S';
    line[pos++] = 'T';
    line[pos++] = ':';

    if (masterId == 0xFF) {
      line[pos++] = '-';
      line[pos++] = '-';
    }
    else {
      appendAddress(line, pos, masterId);
    }

    line[pos] = '\0';
    drawRow(2, line);
  }

  void drawRate(uint32_t requests, uint32_t responses) {
    char rateText[13] = "RATE:---.-%";

    if (requests > 0) {
      uint16_t rateTenths =
        (uint16_t)(((uint64_t)responses * 1000ULL) / requests);

      if (rateTenths > 1000) {
        rateTenths = 1000;
      }

      uint16_t wholePercent = rateTenths / 10;
      if (wholePercent >= 100) {
        rateText[5] = '1';
        rateText[6] = '0';
        rateText[7] = '0';
      }
      else if (wholePercent >= 10) {
        rateText[5] = ' ';
        rateText[6] = '0' + (wholePercent / 10);
        rateText[7] = '0' + (wholePercent % 10);
      }
      else {
        rateText[5] = ' ';
        rateText[6] = ' ';
        rateText[7] = '0' + wholePercent;
      }
      rateText[8] = '.';
      rateText[9] = '0' + (rateTenths % 10);
    }

    rateText[10] = '%';
    rateText[11] = '\0';

    drawRow(5, rateText);
  }

  void drawErrorMissed(uint32_t errors, uint32_t missed) {
    char errorText[5];
    char missedText[5];
    char line[17] = "ERR:";
    uint8_t pos = 4;

    formatCompactCounter(errorText, errors);
    formatCompactCounter(missedText, missed);

    for (uint8_t i = 0; errorText[i] != 0 && pos < 16; i++) {
      line[pos++] = errorText[i];
    }
    line[pos++] = ' ';
    line[pos++] = 'M';
    line[pos++] = 'I';
    line[pos++] = 'S';
    line[pos++] = 'S';
    line[pos++] = ':';

    for (uint8_t i = 0; missedText[i] != 0 && pos < 16; i++) {
      line[pos++] = missedText[i];
    }
    line[pos] = 0;
    drawRow(6, line);
  }

  void drawUiStatus() {
    if (!editingSetting) {
      drawRow_P(1, TEXT_EMPTY);
      return;
    }

    if (changingValue) {
      drawRow(1, selectedSetting == 0 ? "VAL: SLAVE ID" : "VAL: BAUD RATE");
    } else {
      drawRow(1, selectedSetting == 0 ? "EDIT: SLAVE ID" : "EDIT: BAUD RATE");
    }
  }

  void drawBaud(uint8_t index) {
    char line[17] = "BAUD:";
    uint8_t pos = 5;

    appendUnsigned(line, pos, Settings::getBaudForIndex(index));
    line[pos++] = ' ';
    line[pos++] = '8';
    line[pos++] = 'N';
    line[pos++] = '1';
    line[pos] = '\0';
    drawRow(7, line);
  }

  void drawSlaveScreen(uint8_t slaveId, uint8_t masterId,
                       uint8_t functionCode,
                       uint32_t rx, uint32_t tx,
                       uint32_t rateRequests,
                       uint32_t errors, uint32_t missed,
                       uint8_t baudIndex) {
    u8x8.clearDisplay();
    drawRow_P(0, TEXT_HEADER);

    drawUiStatus();
    drawAddressPair(slaveId, masterId);
    drawErrorMissed(errors, missed);
    drawFunction(functionCode);
    drawTraffic(rx, tx);
    drawRate(rateRequests, tx);
    drawBaud(baudIndex);
  }

}

void Display::begin() {
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  displayReady = true;
  displaySleeping = false;
  displayNeedsRedraw = false;
  lastActivityTime = millis();
  displayedSlaveId = 1;
  displayedMasterId = 0xFF;
  displayedFunctionCode = 0;
  displayedRx = 0;
  displayedTx = 0;
  displayedRateRequests = 0;
  displayedErrors = 0;
  displayedBaudIndex = 3;
  selectedSetting = 0;
  editingSetting = false;
  changingValue = false;
  uiTouched = false;
  editSlaveId = 1;
  editBaudIndex = 3;

  // The only full-screen draw: initial display setup.
  drawSlaveScreen(displayedSlaveId, displayedMasterId,
                  displayedFunctionCode, displayedRx,
                  displayedTx, displayedRateRequests,
                  displayedErrors, missedCache, displayedBaudIndex);
}

void Display::activity() {
  if (!displayReady) {
    return;
  }

  lastActivityTime = millis();

  if (displaySleeping) {
    Serial.println(F("ENC_WAKE_BEGIN"));
    u8x8.setPowerSave(0);
    Serial.println(F("ENC_WAKE_DONE"));
    displaySleeping = false;

    // The next update() will redraw from the newest live statistics.
    displayNeedsRedraw = true;
  }
}

uint8_t Display::handleEncoder(int8_t rotation, Encoder::ButtonEvent buttonEvent,
                               uint8_t currentSlaveId, uint8_t currentBaudIndex,
                               uint8_t &newSlaveId, uint8_t &newBaudIndex) {
  uint8_t changed = 0;
  newSlaveId = currentSlaveId;
  newBaudIndex = currentBaudIndex;

  // Outside the editor, rotation and short press have no UI action.
  // A long press opens the editor; a double click resets traffic counters.
  if (!editingSetting) {
    if (buttonEvent == Encoder::BUTTON_LONG) {
      Display::activity();
      editingSetting = true;
      changingValue = false;
      uiTouched = true;
      selectedSetting = 0;
      editSlaveId = currentSlaveId;
      editBaudIndex = currentBaudIndex;
    } else if (buttonEvent == Encoder::BUTTON_DOUBLE) {
      Display::activity();
      changed |= RESET_TRAFFIC;
    }
  } else {
    // Inside the editor, rotation selects a parameter or changes its
    // staged value depending on the current editor sub-state.
    if (rotation != 0) {
      Display::activity();
      uiTouched = true;

      if (changingValue) {
        if (selectedSetting == 0) {
          if (rotation > 0) {
            editSlaveId = (editSlaveId >= 247) ? 1 : editSlaveId + 1;
          } else {
            editSlaveId = (editSlaveId <= 1) ? 247 : editSlaveId - 1;
          }
        } else {
          if (rotation > 0) {
            editBaudIndex = (editBaudIndex >= 4) ? 0 : editBaudIndex + 1;
          } else {
            editBaudIndex = (editBaudIndex == 0) ? 4 : editBaudIndex - 1;
          }
        }
      } else {
        selectedSetting = (selectedSetting == 0) ? 1 : 0;
      }
    }

    if (buttonEvent == Encoder::BUTTON_SHORT) {
      Display::activity();
      uiTouched = true;

      if (!changingValue) {
        // Enter value adjustment for the currently selected parameter.
        editSlaveId = currentSlaveId;
        editBaudIndex = currentBaudIndex;
        changingValue = true;
      } else {
        // Confirm only the selected parameter. The other staged value is
        // never committed implicitly.
        changingValue = false;
        if (selectedSetting == 0 && editSlaveId != currentSlaveId) {
          newSlaveId = editSlaveId;
          changed |= CHANGE_SLAVE_ID;
        } else if (selectedSetting == 1 && editBaudIndex != currentBaudIndex) {
          newBaudIndex = editBaudIndex;
          changed |= CHANGE_BAUD;
        }
      }
    } else if (buttonEvent == Encoder::BUTTON_LONG) {
      // Long press exits the editor. Any currently unconfirmed value is
      // discarded; previously confirmed settings remain saved.
      Display::activity();
      editingSetting = false;
      changingValue = false;
      uiTouched = false;
      editSlaveId = currentSlaveId;
      editBaudIndex = currentBaudIndex;
    }
    // BUTTON_DOUBLE intentionally has no action while editing.
  }

  if (displayReady && !displaySleeping && !displayNeedsRedraw) {
    Serial.print(F("ENC_UI_BEGIN event="));
    Serial.println((uint8_t)buttonEvent);
    drawUiStatus();
    Serial.println(F("ENC_UI_DONE"));
  }

  return changed;
}

void Display::update(uint8_t slaveId, uint8_t masterId,
                     uint8_t functionCode,
                     uint32_t rx, uint32_t tx,
                     uint32_t rateRequests,
                     uint32_t errors, uint32_t missed,
                     uint8_t baudIndex) {
  if (!displayReady) {
    return;
  }

  const uint8_t visibleSlaveId =
    (changingValue && selectedSetting == 0) ? editSlaveId : slaveId;
  const uint8_t visibleBaudIndex =
    (changingValue && selectedSetting == 1) ? editBaudIndex : baudIndex;

  const bool addressesChanged =
    (visibleSlaveId != displayedSlaveId) || (masterId != displayedMasterId);
  const bool baudChanged = (visibleBaudIndex != displayedBaudIndex);
  const bool functionChanged =
    (functionCode != displayedFunctionCode);
  const bool countersChanged =
    (rx != displayedRx) || (tx != displayedTx) ||
    (rateRequests != displayedRateRequests) ||
    (errors != displayedErrors) || (missed != missedCache);

  // Keep the cached data current while the physical display is asleep.
  if (displaySleeping) {
    displayedSlaveId = visibleSlaveId;
    displayedMasterId = masterId;
    displayedFunctionCode = functionCode;
    displayedRx = rx;
    displayedTx = tx;
    displayedRateRequests = rateRequests;
    displayedErrors = errors;
    missedCache = missed;
    displayedBaudIndex = visibleBaudIndex;
    return;
  }

  // Wake-up is requested from the receive path. Redraw only here, using
  // the complete statistics snapshot collected after frame processing.
  if (displayNeedsRedraw) {
    drawSlaveScreen(visibleSlaveId, masterId, functionCode,
                    rx, tx, rateRequests, errors, missed, visibleBaudIndex);

    displayedSlaveId = visibleSlaveId;
    displayedMasterId = masterId;
    displayedFunctionCode = functionCode;
    displayedRx = rx;
    displayedTx = tx;
    displayedRateRequests = rateRequests;
    displayedErrors = errors;
    missedCache = missed;
    displayedBaudIndex = visibleBaudIndex;
    displayNeedsRedraw = false;
    return;
  }

  if (addressesChanged) {
    drawAddressPair(visibleSlaveId, masterId);
  }

  if (functionChanged) {
    drawFunction(functionCode);
  }

  if (countersChanged) {
    drawTraffic(rx, tx);
    drawRate(rateRequests, tx);
    drawErrorMissed(errors, missed);
  }

  if (baudChanged) {
    drawBaud(visibleBaudIndex);
  }

  displayedSlaveId = visibleSlaveId;
  displayedMasterId = masterId;
  displayedFunctionCode = functionCode;
  displayedRx = rx;
  displayedTx = tx;
  displayedRateRequests = rateRequests;
  displayedErrors = errors;
  missedCache = missed;
  displayedBaudIndex = visibleBaudIndex;

  if (millis() - lastActivityTime >= DISPLAY_SLEEP_TIMEOUT) {
    u8x8.setPowerSave(1);
    displaySleeping = true;
  }
}
