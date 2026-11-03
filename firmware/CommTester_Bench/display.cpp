#include "display.h"

#include <U8x8lib.h>

// SSD1306 128x64, hardware I2C, text-only mode
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

namespace {

  bool displayReady = false;
  IndicatorMode displayedMode = IND_SERVICE;

  void drawScreen(IndicatorMode mode) {

    u8x8.clearDisplay();

    u8x8.drawString(0, 0, "CommTester v0.4");

    if (mode == IND_SERVICE) {
      u8x8.drawString(0, 1, "MODE: SERVICE");
    }
    else {
      u8x8.drawString(0, 1, "MODE: SLAVE");
    }

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

  drawScreen(displayedMode);
}

void Display::update(IndicatorMode mode) {

  if (!displayReady || mode == displayedMode) {
    return;
  }

  displayedMode = mode;
  drawScreen(displayedMode);
}
