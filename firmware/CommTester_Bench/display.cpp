#include "display.h"

#include <U8x8lib.h>

// SSD1306 128x64, hardware I2C, text-only mode
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

namespace {

  bool displayReady = false;
  IndicatorMode displayedMode = IND_SERVICE;
  uint32_t displayedRequests = 0;
  uint32_t displayedResponses = 0;

  void drawCounter(uint8_t row, const char* prefix, uint32_t value) {
    char text[17];

    for (uint8_t i = 0; i < 6; i++) {
      text[i] = prefix[i];
    }

    text[16] = '\0';

    for (int8_t i = 15; i >= 6; i--) {
      text[i] = '0' + (value % 10);
      value /= 10;
    }

    u8x8.drawString(0, row, text);
  }

  void drawSlaveScreen(uint32_t requests, uint32_t responses) {

    u8x8.clearDisplay();

    u8x8.drawString(0, 0, "SLAVE ID:01");

    drawCounter(1, "REQ : ", requests);
    drawCounter(2, "RESP: ", responses);

    char rateText[13] = "RATE:  --.-%";

    if (requests > 0) {
      uint16_t rateTenths =
        (uint16_t)(((uint64_t)responses * 1000ULL) / requests);

      if (rateTenths > 1000) {
        rateTenths = 1000;
      }

      uint16_t wholePercent = rateTenths / 10;

      rateText[6] = (wholePercent >= 100) ? '1' :
                    (wholePercent >= 10) ? ('0' + wholePercent / 10) : ' ';
      rateText[7] = (wholePercent >= 10) ? ('0' + (wholePercent / 10) % 10) : ' ';
      rateText[8] = '0' + (wholePercent % 10);
      rateText[9] = '.';
      rateText[10] = '0' + (rateTenths % 10);
    }

    u8x8.drawString(0, 3, rateText);

    uint32_t missed = (requests >= responses) ?
                      (requests - responses) : 0;

    drawCounter(4, "MISS: ", missed);

    u8x8.drawString(0, 5, "9600 8N1");
  }

  void drawScreen(IndicatorMode mode, uint32_t requests, uint32_t responses) {

    if (mode == IND_SLAVE) {
      drawSlaveScreen(requests, responses);
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
  displayedRequests = 0;
  displayedResponses = 0;

  drawScreen(displayedMode, 0, 0);
}

void Display::update(IndicatorMode mode, uint32_t requests, uint32_t responses) {

  if (!displayReady) {
    return;
  }

  if (mode == displayedMode &&
      (mode != IND_SLAVE ||
       (requests == displayedRequests && responses == displayedResponses))) {
    return;
  }

  displayedMode = mode;
  displayedRequests = requests;
  displayedResponses = responses;

  drawScreen(displayedMode, requests, responses);
}
