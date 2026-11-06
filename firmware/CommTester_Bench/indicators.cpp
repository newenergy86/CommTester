#include "indicators.h"

// ============================================
// PIN CONFIGURATION
// ============================================

const byte LED_RX        = 2;
const byte LED_TX        = 3;
const byte LED_ERROR     = 4;
const byte LED_HEARTBEAT = 5;

const byte BUZZER        = 6;


// ============================================
// TIMING
// ============================================

const unsigned long HEARTBEAT_INTERVAL = 2000;
const unsigned long HEARTBEAT_ON_TIME = 60;

const unsigned long SOUND_RX_TIME    = 70;
const unsigned long SOUND_TX_TIME    = 70;
const unsigned long SOUND_ERROR_TIME = 250;

const unsigned long SOUND_PAUSE = 30;
const unsigned long STARTUP_NOTE_GAP = 40;


// ============================================
// SOUND
// ============================================

const unsigned int SOUND_RX_FREQ    = 2400;
const unsigned int SOUND_TX_FREQ    = 1800;
const unsigned int SOUND_ERROR_FREQ = 700;


// ============================================
// SINGLE EVENT QUEUE
// ============================================
//
// RX/TX/ERROR and the startup melody share ONE queue.
// This prevents tone() calls from interrupting one another.
//
// The startup melody is one queued event consisting of
// three notes. The heartbeat LED is independent of the
// RX/TX/ERROR LEDs and does not use delay().
//

enum EventType {
  EVENT_NONE,
  EVENT_STARTUP,
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

byte startupNoteIndex = 0;
bool startupNoteGap = false;


// ============================================
// HEARTBEAT STATE
// ============================================

unsigned long heartbeatTimer = 0;
unsigned long heartbeatPulseTimer = 0;
bool heartbeatOn = false;


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
// ALL MODBUS EVENT LEDS OFF
// ============================================

void allEventLedsOff() {
  digitalWrite(LED_RX, LOW);
  digitalWrite(LED_TX, LOW);
  digitalWrite(LED_ERROR, LOW);
}


// ============================================
// STARTUP MELODY
// ============================================

unsigned int getStartupNoteFrequency(byte index) {
  switch (index) {
    case 0: return 523;
    case 1: return 659;
    default: return 784;
  }
}


unsigned long getStartupNoteDuration(byte index) {
  if (index == 2) {
    return 180;
  }

  return 100;
}


void startStartupNote(byte index) {
  digitalWrite(LED_HEARTBEAT, HIGH);
  tone(BUZZER, getStartupNoteFrequency(index));
  eventTimer = millis();
}


void updateStartupEvent(unsigned long now) {
  if (startupNoteGap) {
    if (now - eventTimer < STARTUP_NOTE_GAP) {
      return;
    }

    startupNoteGap = false;
    startStartupNote(startupNoteIndex);
    return;
  }

  if (now - eventTimer < getStartupNoteDuration(startupNoteIndex)) {
    return;
  }

  noTone(BUZZER);
  digitalWrite(LED_HEARTBEAT, LOW);

  startupNoteIndex++;

  if (startupNoteIndex >= 3) {
    activeEvent = EVENT_NONE;
    eventPause = true;
    eventTimer = now;
    heartbeatTimer = now;
    return;
  }

  eventTimer = now;
  startupNoteGap = true;
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

    case EVENT_STARTUP:
      startupNoteIndex = 0;
      startupNoteGap = false;
      startStartupNote(startupNoteIndex);
      break;

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

  if (activeEvent == EVENT_STARTUP) {
    updateStartupEvent(now);
    return;
  }

  // ------------------------------------------
  // CURRENT RX/TX/ERROR EVENT
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
// HEARTBEAT UPDATE
// ============================================

void updateHeartbeat() {
  unsigned long now = millis();

  // The startup melody owns D5 while it is playing.
  if (activeEvent == EVENT_STARTUP) {
    return;
  }

  if (heartbeatOn) {
    if (now - heartbeatPulseTimer >= HEARTBEAT_ON_TIME) {
      digitalWrite(LED_HEARTBEAT, LOW);
      heartbeatOn = false;
    }

    return;
  }

  if (now - heartbeatTimer >= HEARTBEAT_INTERVAL) {
    digitalWrite(LED_HEARTBEAT, HIGH);
    heartbeatOn = true;
    heartbeatPulseTimer = now;
    heartbeatTimer = now;
  }
}


// ============================================
// BEGIN
// ============================================

void Indicators::begin() {
  pinMode(LED_RX, OUTPUT);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_HEARTBEAT, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  noTone(BUZZER);

  eventQueueHead = 0;
  eventQueueTail = 0;
  eventQueueCount = 0;
  activeEvent = EVENT_NONE;
  eventTimer = millis();
  eventPause = false;

  startupNoteIndex = 0;
  startupNoteGap = false;

  heartbeatTimer = millis();
  heartbeatPulseTimer = 0;
  heartbeatOn = false;

  allEventLedsOff();
  digitalWrite(LED_HEARTBEAT, LOW);

  // Startup melody is first in the shared sound queue.
  queueEvent(EVENT_STARTUP);
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
  updateHeartbeat();
}
