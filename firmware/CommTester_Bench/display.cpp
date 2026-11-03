#include "display.h"

#include <U8g2lib.h>


// ============================================
// SSD1306 128x64 OLED
// Hardware I2C, one-page buffer (128 bytes)
// Nano: SDA = A4, SCL = A5
// ============================================

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

namespace {

  bool displayReady = false;
  IndicatorMode displayedMode = IND_SERVICE;


  void drawScreen(IndicatorMode mode) {

    u8g2.firstPage();

    do {

      // Outer frame
      u8g2.drawFrame(0, 0, 128, 64);

      // Header
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(5, 10, "CommTester");
      u8g2.drawHLine(4, 13, 120);

      // Operating mode
      if (mode == IND_SERVICE) {
        u8g2.drawStr(5, 26, "MODE: SERVICE");
      }
      else {
        u8g2.drawStr(5, 26, "MODE: SLAVE");
      }

      u8g2.drawHLine(4, 30, 120);

      // Serial interface
      u8g2.drawStr(5, 42, "RS232: 9600 8N1");
      u8g2.drawStr(5, 53, "SLAVE ID: 1");
      u8g2.drawStr(5, 62, "HOLD BTN: SWITCH");

    } while (u8g2.nextPage());
  }

}


void Display::begin() {

  u8g2.begin();

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
