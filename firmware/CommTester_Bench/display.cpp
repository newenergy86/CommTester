#include "display.h"

#include <Arduino.h>
#include <U8x8lib.h>

// SSD1306 128x64, hardware I2C, text-only mode
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

namespace {

  const unsigned long DISPLAY_SLEEP_TIMEOUT = 30000UL;

  bool displayReady = false;
  bool displaySleeping = false;
  unsigned long lastActivityTime = 0;
  uint8_t displayedSlaveId = 1;
  uint8_t displayedMasterId = 0xFF;
  uint8_t displayedFunctionCode = 0;
  uint32_t displayedRx = 0;
  uint32_t displayedTx = 0;
  uint32_t displayedErrors = 0;
  uint32_t missedCache = 0;

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
      drawRow(3, "FC:--");
      return;
    }

    if (functionCode < 10) {
      line[pos++] = '0';
    }
    appendUnsigned(line, pos, functionCode);
    line[pos++] = ' ';

    const char *description = "UNKNOWN";

    switch (functionCode) {
      case 1:  description = "READ COIL";  break;
      case 2:  description = "READ DI";    break;
      case 3:  description = "READ HR";    break;
      case 4:  description = "READ IR";    break;
      case 5:  description = "WRITE COIL"; break;
      case 6:  description = "WRITE HR";   break;
      case 15: description = "WRITE COIL"; break;
      case 16: description = "WRITE HR";   break;
    }

    while (*description != '\0' && pos < 16) {
      line[pos++] = *description++;
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
    drawRow(1, line);
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
    drawRow(2, line);
  }

  void drawSlaveScreen(uint8_t slaveId, uint8_t masterId,
                       uint8_t functionCode,
                       uint32_t rx, uint32_t tx,
                       uint32_t errors, uint32_t missed) {
    u8x8.clearDisplay();
    drawRow(0, "CommTester v0.4");

    drawAddressPair(slaveId, masterId);
    drawErrorMissed(errors, missed);
    drawFunction(functionCode);
    drawTraffic(rx, tx);
    drawRate(rx, tx);
    drawRow(7, "9600 8N1");
  }

}

void Display::begin() {
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  displayReady = true;
  displaySleeping = false;
  lastActivityTime = millis();
  displayedSlaveId = 1;
  displayedMasterId = 0xFF;
  displayedFunctionCode = 0;
  displayedRx = 0;
  displayedTx = 0;
  displayedErrors = 0;

  // The only full-screen draw: initial display setup.
  drawSlaveScreen(displayedSlaveId, displayedMasterId,
                  displayedFunctionCode, displayedRx,
                  displayedTx, displayedErrors, missedCache);
}

void Display::activity() {
  if (!displayReady) {
    return;
  }

  lastActivityTime = millis();

  if (displaySleeping) {
    u8x8.setPowerSave(0);
    displaySleeping = false;

    // Restore the latest cached screen after waking.
    drawSlaveScreen(displayedSlaveId, displayedMasterId,
                    displayedFunctionCode, displayedRx,
                    displayedTx, displayedErrors, missedCache);
  }
}

void Display::update(uint8_t slaveId, uint8_t masterId,
                     uint8_t functionCode,
                     uint32_t rx, uint32_t tx,
                     uint32_t errors, uint32_t missed) {
  if (!displayReady) {
    return;
  }

  const bool addressesChanged =
    (slaveId != displayedSlaveId) || (masterId != displayedMasterId);
  const bool functionChanged =
    (functionCode != displayedFunctionCode);
  const bool countersChanged =
    (rx != displayedRx) || (tx != displayedTx) ||
    (errors != displayedErrors);

  // Keep the cached data current while the physical display is asleep.
  if (displaySleeping) {
    displayedSlaveId = slaveId;
    displayedMasterId = masterId;
    displayedFunctionCode = functionCode;
    displayedRx = rx;
    displayedTx = tx;
    displayedErrors = errors;
    return;
  }

  if (addressesChanged) {
    drawAddressPair(slaveId, masterId);
  }

  if (functionChanged) {
    drawFunction(functionCode);
  }

  if (countersChanged) {
    drawTraffic(rx, tx);
    drawRate(rx, tx);
    drawErrors(errors);
  }

  displayedSlaveId = slaveId;
  displayedMasterId = masterId;
  displayedFunctionCode = functionCode;
  displayedRx = rx;
  displayedTx = tx;
  displayedErrors = errors;

  if (millis() - lastActivityTime >= DISPLAY_SLEEP_TIMEOUT) {
    u8x8.setPowerSave(1);
    displaySleeping = true;
  }
}
