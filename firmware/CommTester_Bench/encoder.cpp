#include "encoder.h"
#include "indicators.h"
#include "display.h"
#include <KY040.h>

// KY-040 connections:
// CLK/A -> D8, DT/B -> D9, SW -> D7.
// The encoder module has external pull-up resistors, so use INPUT.
namespace {
  const byte ENCODER_CLK = 8;
  const byte ENCODER_DT = 9;
  const byte ENCODER_SW = 7;

  const unsigned long BUTTON_DEBOUNCE_MS = 30;

  KY040 rotaryEncoder(ENCODER_CLK, ENCODER_DT);

  bool buttonStableState = HIGH;
  bool buttonLastReading = HIGH;
  unsigned long buttonLastChange = 0;

  int8_t pendingRotation = 0;
  bool pendingButtonPress = false;
}

void Encoder::begin() {
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT);

  buttonStableState = digitalRead(ENCODER_SW);
  buttonLastReading = buttonStableState;
  buttonLastChange = millis();

  pendingRotation = 0;
  pendingButtonPress = false;

  Serial.println(F("Encoder input test ready"));
}

void Encoder::update() {
  const byte rotation = rotaryEncoder.getRotation();

  if (rotation == KY040::CLOCKWISE) {
    Serial.println(F("ENC: CW"));
    Indicators::encoderStep();
    Display::activity();
    pendingRotation = 1;
  } else if (rotation == KY040::COUNTERCLOCKWISE) {
    Serial.println(F("ENC: CCW"));
    Indicators::encoderStep();
    Display::activity();
    pendingRotation = -1;
  }

  const bool reading = digitalRead(ENCODER_SW);
  const unsigned long now = millis();

  if (reading != buttonLastReading) {
    buttonLastReading = reading;
    buttonLastChange = now;
  }

  if ((now - buttonLastChange) >= BUTTON_DEBOUNCE_MS &&
      reading != buttonStableState) {
    buttonStableState = reading;
    Display::activity();

    if (buttonStableState == LOW) {
      Serial.println(F("ENC: BUTTON PRESSED"));
      Indicators::encoderPress();
      pendingButtonPress = true;
    } else {
      Serial.println(F("ENC: BUTTON RELEASED"));
    }
  }
}

bool Encoder::takeRotation(int8_t &direction) {
  if (pendingRotation == 0) {
    return false;
  }

  direction = pendingRotation;
  pendingRotation = 0;
  return true;
}

bool Encoder::takeButtonPress() {
  const bool pressed = pendingButtonPress;
  pendingButtonPress = false;
  return pressed;
}
