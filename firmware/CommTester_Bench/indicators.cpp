#include "indicators.h"

// ============================================
// PIN CONFIGURATION
// ============================================

const byte LED_RX    = 2;
const byte LED_TX    = 3;
const byte LED_ERROR = 4;
const byte LED_MODE  = 5;

const byte BUZZER    = 6;
const byte BUTTON    = 7;


// ============================================
// TIMING
// ============================================

const unsigned long EVENT_LED_TIME = 80;

const unsigned long MODE_ON_TIME   = 180;
const unsigned long MODE_OFF_TIME  = 180;
const unsigned long MODE_PAUSE     = 3000;

const unsigned long BUTTON_DEBOUNCE = 30;
const unsigned long BUTTON_HOLD     = 2000;


// ============================================
// SOUND
// ============================================

const unsigned int SOUND_RX_FREQ     = 2400;
const unsigned int SOUND_TX_FREQ     = 1800;
const unsigned int SOUND_ERROR_FREQ  = 700;

const unsigned long SOUND_RX_TIME    = 70;
const unsigned long SOUND_TX_TIME    = 70;
const unsigned long SOUND_ERROR_TIME = 250;

const unsigned long SOUND_PAUSE      = 30;


// ============================================
// SINGLE EVENT QUEUE
// ============================================
//
// RX/TX/ERROR are deliberately kept in ONE queue.
// LED and sound belong to the same event, so they
// cannot drift apart and a following event cannot
// interrupt the previous one.
//
// Event order:
// RX -> pause -> TX -> pause -> ERROR
//

enum EventType {
  EVENT_NONE,
  EVENT_RX,
  EVENT_TX,
  EVENT_ERROR
};

const byte EVENT_QUEUE_SIZE = 8;

EventType eventQueue[EVENT_QUEUE_SIZE];
byte eventQueueHead = 0;
byte eventQueueTail = 0;
byte eventQueueCount = 0;

EventType activeEvent = EVENT_NONE;
unsigned long eventTimer = 0;
bool eventPause = false;


// ============================================
// MODE INDICATION
// ============================================

IndicatorMode currentMode = IND_SERVICE;

byte modeStep = 0;
unsigned long modeTimer = 0;


// ============================================
// BUTTON
// ============================================

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long buttonChangeTime = 0;
unsigned long buttonPressTime = 0;

bool buttonEvent = false;
bool buttonHoldTriggered = false;


// ============================================
// BUZZER
// ============================================

bool beepActive = false;
byte beepCount = 0;
byte beepStep = 0;
unsigned long beepTimer = 0;
bool beepPause = false;


// ============================================
// EVENT QUEUE
// ============================================

void queueEvent(EventType type) {
  if (eventQueueCount >= EVENT_QUEUE_SIZE) {
    return;
  }

  eventQueue[eventQueueTail] = type;

  eventQueueTail++;
  if (eventQueueTail >= EVENT_QUEUE_SIZE) {
    eventQueueTail = 0;
  }

  eventQueueCount++;
}


// ============================================
// ALL EVENT LEDS OFF
// ============================================

void allEventLedsOff() {
  digitalWrite(LED_RX, LOW);
  digitalWrite(LED_TX, LOW);
  digitalWrite(LED_ERROR, LOW);
}


// ============================================
// START EVENT
// ============================================

void startEvent(EventType type) {
  allEventLedsOff();

  activeEvent = type;
  eventTimer = millis();
  eventPause = false;

  switch (type) {

    case EVENT_RX:
      digitalWrite(LED_RX, HIGH);
      tone(BUZZER, SOUND_RX_FREQ);
      break;

    case EVENT_TX:
      digitalWrite(LED_TX, HIGH);
      tone(BUZZER, SOUND_TX_FREQ);
      break;

    case EVENT_ERROR:
      digitalWrite(LED_ERROR, HIGH);
      tone(BUZZER, SOUND_ERROR_FREQ);
      break;

    default:
      activeEvent = EVENT_NONE;
      noTone(BUZZER);
      break;
  }
}


// ============================================
// EVENT DURATION
// ============================================

unsigned long getEventDuration(EventType type) {

  switch (type) {

    case EVENT_RX:
      return SOUND_RX_TIME;

    case EVENT_TX:
      return SOUND_TX_TIME;

    case EVENT_ERROR:
      return SOUND_ERROR_TIME;

    default:
      return 0;
  }
}


// ============================================
// EVENT UPDATE
// ============================================

void updateEvents() {

  unsigned long now = millis();

  // ------------------------------------------
  // CURRENT EVENT
  // ------------------------------------------

  if (activeEvent != EVENT_NONE) {

    if (now - eventTimer >= getEventDuration(activeEvent)) {

      noTone(BUZZER);
      allEventLedsOff();

      activeEvent = EVENT_NONE;
      eventPause = true;
      eventTimer = now;
    }

    return;
  }


  // ------------------------------------------
  // PAUSE BETWEEN EVENTS
  // ------------------------------------------

  if (eventPause) {

    if (now - eventTimer < SOUND_PAUSE) {
      return;
    }

    eventPause = false;
  }


  // ------------------------------------------
  // START NEXT EVENT
  // ------------------------------------------

  if (eventQueueCount > 0) {

    EventType type = eventQueue[eventQueueHead];

    eventQueueHead++;

    if (eventQueueHead >= EVENT_QUEUE_SIZE) {
      eventQueueHead = 0;
    }

    eventQueueCount--;

    startEvent(type);
  }
}


// ============================================
// MODE BEEP
// ============================================

void updateBeep() {

  unsigned long now = millis();

  if (beepActive) {

    if (now - beepTimer >= 100) {

      noTone(BUZZER);

      beepStep++;
      beepTimer = now;

      if (beepStep >= beepCount) {

        beepActive = false;
        beepPause = true;
      }
      else {

        tone(BUZZER, 2000);
      }
    }

    return;
  }

  if (beepPause) {

    if (now - beepTimer < SOUND_PAUSE) {
      return;
    }

    beepPause = false;
  }
}


// ============================================
// BEGIN
// ============================================

void Indicators::begin() {

  pinMode(LED_RX, OUTPUT);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_MODE, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  digitalWrite(LED_RX, LOW);
  digitalWrite(LED_TX, LOW);
  digitalWrite(LED_ERROR, LOW);
  digitalWrite(LED_MODE, LOW);

  noTone(BUZZER);

  currentMode = IND_SERVICE;

  modeStep = 0;
  modeTimer = millis();

  eventQueueHead = 0;
  eventQueueTail = 0;
  eventQueueCount = 0;
  activeEvent = EVENT_NONE;
  eventTimer = millis();
  eventPause = false;

  beepActive = false;
  beepCount = 0;
  beepStep = 0;
  beepTimer = 0;
  beepPause = false;

  allEventLedsOff();
}


// ============================================
// RX INDICATION
// ============================================

void Indicators::rx() {
  queueEvent(EVENT_RX);
}


// ============================================
// TX INDICATION
// ============================================

void Indicators::tx() {
  queueEvent(EVENT_TX);
}


// ============================================
// ERROR INDICATION
// ============================================

void Indicators::error() {
  queueEvent(EVENT_ERROR);
}


// ============================================
// SET MODE
// ============================================

void Indicators::setMode(IndicatorMode mode) {

  currentMode = mode;

  modeStep = 0;
  modeTimer = millis();

  byte count = 1;

  if (mode == IND_SLAVE) {
    count = 2;
  }


  beep(count);
}


// ============================================
// GET MODE
// ============================================

IndicatorMode Indicators::getMode() {
  return currentMode;
}


// ============================================
// BUTTON EVENT
// ============================================

bool Indicators::buttonPressed() {

  if (buttonEvent) {

    buttonEvent = false;

    return true;
  }

  return false;
}


// ============================================
// BEEP
// ============================================

void Indicators::beep(byte count) {

  // Если сейчас идёт Modbus-индикация,
  // не запускаем beep поверх неё.
  if (activeEvent != EVENT_NONE || eventPause) {
    return;
  }


  beepCount = count;
  beepStep = 0;
  beepTimer = millis();
  beepActive = true;
  beepPause = false;

  tone(BUZZER, 2000);
}


// ============================================
// UPDATE
// ============================================

void Indicators::update() {

  unsigned long now = millis();


  // ==========================================
  // RX / TX / ERROR
  // ==========================================

  updateEvents();


  // ==========================================
  // MODE LED
  // ==========================================

  byte modeCount = 1;

  if (currentMode == IND_SLAVE) {
    modeCount = 2;
  }



  if (modeStep < modeCount) {

    unsigned long elapsed = now - modeTimer;

    if (elapsed < MODE_ON_TIME) {
      digitalWrite(LED_MODE, HIGH);
    }
    else {

      digitalWrite(LED_MODE, LOW);

      if (elapsed >= MODE_ON_TIME + MODE_OFF_TIME) {
        modeStep++;
        modeTimer = now;
      }
    }
  }

  else {

    digitalWrite(LED_MODE, LOW);

    if (now - modeTimer >= MODE_PAUSE) {
      modeStep = 0;
      modeTimer = now;
    }
  }


  // ==========================================
  // BUTTON
  // ==========================================

  bool reading = digitalRead(BUTTON);

  if (reading != lastButtonReading) {
    buttonChangeTime = now;
  }

  if (now - buttonChangeTime >= BUTTON_DEBOUNCE) {

    if (reading != stableButtonState) {

      stableButtonState = reading;

      if (stableButtonState == LOW) {

        buttonPressTime = now;
        buttonEvent = false;
        buttonHoldTriggered = false;
      }

      else {

        buttonPressTime = 0;
        buttonHoldTriggered = false;
      }
    }
  }

  lastButtonReading = reading;


  // ==========================================
  // BUTTON HOLD > 2 SEC
  // ==========================================

  if (stableButtonState == LOW &&
      buttonPressTime != 0 &&
      !buttonHoldTriggered) {

    if (now - buttonPressTime >= BUTTON_HOLD) {

      buttonEvent = true;
      buttonHoldTriggered = true;
    }
  }


  // ==========================================
  // MODE BEEP
  // ==========================================

  updateBeep();
}