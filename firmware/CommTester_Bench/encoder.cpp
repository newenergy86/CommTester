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
  const unsigned long LONG_PRESS_MS = 2000;
  const unsigned long DOUBLE_CLICK_MS = 350;

  KY040 rotaryEncoder(ENCODER_CLK, ENCODER_DT);

  bool buttonStableState = HIGH;
  bool buttonLastReading = HIGH;
  unsigned long buttonLastChange = 0;
  unsigned long buttonPressStarted = 0;
  bool longPressReported = false;

  bool clickPending = false;
  bool secondClickInProgress = false;
  unsigned long firstClickReleasedAt = 0;

  int8_t pendingRotation = 0;
  Encoder::ButtonEvent pendingButtonEvent = Encoder::BUTTON_NONE;

  void queueButtonEvent(Encoder::ButtonEvent event) {
    // The main loop consumes events every iteration. Keep the first event
    // if an unexpected backlog occurs rather than overwriting it.
    if (pendingButtonEvent == Encoder::BUTTON_NONE) {
      pendingButtonEvent = event;
    }
  }
}

void Encoder::begin() {
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT);

  buttonStableState = digitalRead(ENCODER_SW);
  buttonLastReading = buttonStableState;
  buttonLastChange = millis();
  buttonPressStarted = millis();
  longPressReported = false;
  clickPending = false;
  secondClickInProgress = false;

  pendingRotation = 0;
  pendingButtonEvent = BUTTON_NONE;

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

  const unsigned long now = millis();
  const bool reading = digitalRead(ENCODER_SW);

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
      buttonPressStarted = now;
      longPressReported = false;
      secondClickInProgress =
        clickPending && (now - firstClickReleasedAt) <= DOUBLE_CLICK_MS;
    } else {
      Serial.println(F("ENC: BUTTON RELEASED"));

      if (!longPressReported) {
        if (secondClickInProgress) {
          clickPending = false;
          secondClickInProgress = false;
          queueButtonEvent(BUTTON_DOUBLE);
          Serial.println(F("ENC: DOUBLE CLICK"));
        } else {
          // This release starts a new click candidate. A second press
          // beginning inside the interval completes a double click.
          clickPending = true;
          firstClickReleasedAt = now;
        }
      } else {
        secondClickInProgress = false;
      }
    }
  }

  // A long press is emitted once while the button remains held.
  if (buttonStableState == LOW && !longPressReported &&
      (now - buttonPressStarted) >= LONG_PRESS_MS) {
    longPressReported = true;
    clickPending = false;
    secondClickInProgress = false;
    queueButtonEvent(BUTTON_LONG);
    Serial.println(F("ENC: LONG PRESS"));
  }

  // Defer a short-click event until the double-click window has elapsed.
  if (clickPending && !secondClickInProgress &&
      (now - firstClickReleasedAt) > DOUBLE_CLICK_MS) {
    clickPending = false;
    queueButtonEvent(BUTTON_SHORT);
    Serial.println(F("ENC: SHORT CLICK"));
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

bool Encoder::takeButtonEvent(ButtonEvent &event) {
  if (pendingButtonEvent == BUTTON_NONE) {
    return false;
  }

  event = pendingButtonEvent;
  pendingButtonEvent = BUTTON_NONE;
  return true;
}
