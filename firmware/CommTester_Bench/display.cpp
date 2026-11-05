#include "display.h"

#include <U8x8lib.h>

// SSD1306 128x64, hardware I2C, text-only mode
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

namespace {

  bool displayReady = false;
  IndicatorMode displayedMode = IND_SERVICE;
  uint8_t displayedSlaveId = 0;
  uint8_t displayedMasterId = 0xFF;
  uint8_t displayedFunctionCode = 0;
  uint32_t displayedRequests = 0;
  uint32_t displayedResponses = 0;

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

    for (uint8_t i = 0; rx[i] != '\0'; i++) {
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
    u8x8.drawString(0, 4, line);
  }

  void drawFunction(uint8_t functionCode) {
    char line[17] = "FC:";
    uint8_t pos = 3;

    if (functionCode == 0) {
      u8x8.drawString(0, 3, "FC:--");
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
    u8x8.drawString(0, 3, line);
  }

  void drawAddress(uint8_t row, const char *label, uint8_t address) {
    char line[17];
    uint8_t pos = 0;

    while (*label != '\0' && pos < 12) {
      line[pos++] = *label++;
    }

    line[pos++] = ':';

    if (address >= 100) {
      line[pos++] = '0' + (address / 100);
      line[pos++] = '0' + ((address / 10) % 10);
      line[pos++] = '0' + (address % 10);
    }
    else {
      line[pos++] = '0' + (address / 10);
      line[pos++] = '0' + (address % 10);
    }

    line[pos] = '\0';
    u8x8.drawString(0, row, line);
  }

  void drawSlaveScreen(uint8_t slaveId, uint8_t masterId,
                       uint8_t functionCode,
                       uint32_t requests, uint32_t responses) {
    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "CommTester v0.4");

    drawAddress(1, "Slave", slaveId);

    if (masterId == 0xFF) {
      u8x8.drawString(0, 2, "Master:--");
    }
    else {
      drawAddress(2, "Master", masterId);
    }

    drawFunction(functionCode);
    drawTraffic(requests, responses);

    char rateText[13] = "RATE:--.-%";

    if (requests > 0) {
      uint16_t rateTenths =
        (uint16_t)(((uint64_t)responses * 1000ULL) / requests);

      if (rateTenths > 1000) {
        rateTenths = 1000;
      }

      uint16_t wholePercent = rateTenths / 10;
      rateText[5] = (wholePercent >= 100) ? '1' :
                    (wholePercent >= 10) ? ('0' + wholePercent / 10) : ' ';
      rateText[6] = (wholePercent >= 10) ? ('0' + (wholePercent / 10) % 10) : ' ';
      rateText[7] = '0' + (wholePercent % 10);
      rateText[8] = '.';
      rateText[9] = '0' + (rateTenths % 10);
    }

    u8x8.drawString(0, 5, rateText);

    uint32_t missed = (requests >= responses) ?
                      (requests - responses) : 0;
    char missText[17] = "MISS:";
    uint8_t missPos = 5;
    appendUnsigned(missText, missPos, missed);
    u8x8.drawString(0, 6, missText);

    u8x8.drawString(0, 7, "9600 8N1");
  }

  void drawScreen(IndicatorMode mode, uint8_t slaveId, uint8_t masterId,
                  uint8_t functionCode,
                  uint32_t requests, uint32_t responses) {
    if (mode == IND_SLAVE) {
      drawSlaveScreen(slaveId, masterId, functionCode, requests, responses);
      return;
    }

    u8x8.clearDisplay();
    u8x8.drawString(0, 0, "CommTester v0.4");
    u8x8.drawString(0, 1, "MODE: SERVICE");
    u8x8.drawString(0, 2, "RS232: 9600");
    u8x8.drawString(0, 3, "FORMAT: 8N1");
    u8x8.drawString(0, 4, "SLAVE ID: 1");
    u8x8.drawString(0, 5, "HOLD BTN: 2s");
  }

}

void Display::begin() {
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  displayReady = true;
  displayedMode = IND_SERVICE;
  displayedSlaveId = 0;
  displayedMasterId = 0xFF;
  displayedFunctionCode = 0;
  displayedRequests = 0;
  displayedResponses = 0;

  drawScreen(displayedMode, 1, 0xFF, 0, 0, 0);
}

void Display::update(IndicatorMode mode, uint8_t slaveId, uint8_t masterId,
                     uint8_t functionCode,
                     uint32_t requests, uint32_t responses) {
  if (!displayReady) {
    return;
  }

  if (mode == displayedMode &&
      (mode != IND_SLAVE ||
       (slaveId == displayedSlaveId &&
        masterId == displayedMasterId &&
        functionCode == displayedFunctionCode &&
        requests == displayedRequests &&
        responses == displayedResponses))) {
    return;
  }

  displayedMode = mode;
  displayedSlaveId = slaveId;
  displayedMasterId = masterId;
  displayedFunctionCode = functionCode;
  displayedRequests = requests;
  displayedResponses = responses;

  drawScreen(mode, slaveId, masterId, functionCode, requests, responses);
}
