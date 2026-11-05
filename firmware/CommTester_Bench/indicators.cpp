#include "indicators.h"

// ============================================
// PIN CONFIGURATION
// ============================================

const byte LED_RX    = 2;
const byte LED_TX    = 3;
const byte LED_ERROR = 4;

const byte BUZZER    = 6;


// ============================================
// TIMING
// ============================================

const unsigned long EVENT_LED_TIME = 80;



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
// BEGIN
// ============================================

void Indicators::begin() {
  pinMode(LED_RX, OUTPUT);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  noTone(BUZZER);

  eventQueueHead = 0;
  eventQueueTail = 0;
  eventQueueCount = 0;
  activeEvent = EVENT_NONE;
  eventTimer = millis();
  eventPause = false;

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
// UPDATE
// ============================================

void Indicators::update() {
  updateEvents();
}
