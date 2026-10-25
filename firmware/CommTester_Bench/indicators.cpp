#include "indicators.h"

const byte LED_RX    = 2;
const byte LED_TX    = 3;
const byte LED_ERROR = 4;
const byte LED_MODE  = 5;

const byte BUZZER    = 6;
const byte BUTTON    = 7;

const unsigned long EVENT_LED_TIME = 80;
const unsigned long MODE_ON_TIME   = 180;
const unsigned long MODE_OFF_TIME  = 180;
const unsigned long MODE_PAUSE     = 3000;
const unsigned long BUTTON_DEBOUNCE = 30;
const unsigned long BUTTON_HOLD     = 2000;

const unsigned int SOUND_RX_FREQ    = 2400;
const unsigned int SOUND_TX_FREQ    = 1800;
const unsigned int SOUND_ERROR_FREQ = 700;

const unsigned long SOUND_RX_TIME    = 70;
const unsigned long SOUND_TX_TIME    = 70;
const unsigned long SOUND_ERROR_TIME = 250;
const unsigned long SOUND_PAUSE = 30;

unsigned long rxUntil = 0;
unsigned long txUntil = 0;
unsigned long errorUntil = 0;

IndicatorMode currentMode = IND_SERVICE;
byte modeStep = 0;
unsigned long modeTimer = 0;

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long buttonChangeTime = 0;
unsigned long buttonPressTime = 0;
bool buttonEvent = false;
bool buttonHoldTriggered = false;

enum SoundType {
  SOUND_NONE,
  SOUND_RX,
  SOUND_TX,
  SOUND_ERROR,
  SOUND_BEEP
};

struct SoundItem {
  SoundType type;
  byte count;
};

const byte SOUND_QUEUE_SIZE = 8;
SoundItem soundQueue[SOUND_QUEUE_SIZE];
byte soundQueueHead = 0;
byte soundQueueTail = 0;
byte soundQueueCount = 0;
bool soundActive = false;
bool soundPause = false;
unsigned long soundTimer = 0;
byte soundBeepCount = 0;
byte soundBeepStep = 0;

void queueSound(SoundType type, byte count = 1) {
  if (soundQueueCount >= SOUND_QUEUE_SIZE) {
    return;
  }

  soundQueue[soundQueueTail].type = type;
  soundQueue[soundQueueTail].count = count;

  soundQueueTail++;
  if (soundQueueTail >= SOUND_QUEUE_SIZE) {
    soundQueueTail = 0;
  }

  soundQueueCount++;
}

void startSound(SoundType type, byte count) {
  soundActive = true;
  soundPause = false;
  soundTimer = millis();
  soundBeepCount = count;
  soundBeepStep = 0;

  switch (type) {
    case SOUND_RX:
      tone(BUZZER, SOUND_RX_FREQ);
      break;
    case SOUND_TX:
      tone(BUZZER, SOUND_TX_FREQ);
      break;
    case SOUND_ERROR:
      tone(BUZZER, SOUND_ERROR_FREQ);
      break;
    case SOUND_BEEP:
      tone(BUZZER, 2000);
      break;
    default:
      noTone(BUZZER);
      soundActive = false;
      break;
  }
}

unsigned long getSoundDuration(SoundType type) {
  switch (type) {
    case SOUND_RX: return SOUND_RX_TIME;
    case SOUND_TX: return SOUND_TX_TIME;
    case SOUND_ERROR: return SOUND_ERROR_TIME;
    case SOUND_BEEP: return 100;
    default: return 0;
  }
}

void updateSound() {
  unsigned long now = millis();

  if (soundActive) {
    SoundType type =
      soundQueue[
        (soundQueueHead + SOUND_QUEUE_SIZE - soundQueueCount)
        % SOUND_QUEUE_SIZE
      ].type;

    if (type == SOUND_RX ||
        type == SOUND_TX ||
        type == SOUND_ERROR) {

      if (now - soundTimer >= getSoundDuration(type)) {
        noTone(BUZZER);
        soundActive = false;
        soundPause = true;
        soundTimer = now;
      }

      return;
    }

    if (type == SOUND_BEEP) {
      if (!soundPause) {
        if (now - soundTimer >= 100) {
          noTone(BUZZER);
          soundBeepStep++;
          soundTimer = now;

          if (soundBeepStep >= soundBeepCount) {
            soundActive = false;
            soundPause = true;
          }
        }
      }

      return;
    }
  }

  if (soundPause) {
    if (now - soundTimer < SOUND_PAUSE) {
      return;
    }
    soundPause = false;
  }

  if (!soundActive && soundQueueCount > 0) {
    SoundType type = soundQueue[soundQueueHead].type;
    byte count = soundQueue[soundQueueHead].count;

    soundQueueHead++;
    if (soundQueueHead >= SOUND_QUEUE_SIZE) {
      soundQueueHead = 0;
    }

    soundQueueCount--;
    startSound(type, count);
  }
}

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
  soundQueueHead = 0;
  soundQueueTail = 0;
  soundQueueCount = 0;
  soundActive = false;
  soundPause = false;
}

void Indicators::rx() {
  rxUntil = millis() + EVENT_LED_TIME;
  queueSound(SOUND_RX);
}

void Indicators::tx() {
  txUntil = millis() + EVENT_LED_TIME;
  queueSound(SOUND_TX);
}

void Indicators::error() {
  errorUntil = millis() + 300;
  queueSound(SOUND_ERROR);
}

void Indicators::setMode(IndicatorMode mode) {
  currentMode = mode;
  modeStep = 0;
  modeTimer = millis();

  byte count = 1;
  if (mode == IND_SLAVE) count = 2;
  if (mode == IND_MASTER) count = 3;
  beep(count);
}

IndicatorMode Indicators::getMode() {
  return currentMode;
}

bool Indicators::buttonPressed() {
  if (buttonEvent) {
    buttonEvent = false;
    return true;
  }
  return false;
}

void Indicators::beep(byte count) {
  queueSound(SOUND_BEEP, count);
}

void Indicators::update() {
  unsigned long now = millis();

  digitalWrite(LED_RX, now < rxUntil);
  digitalWrite(LED_TX, now < txUntil);
  digitalWrite(LED_ERROR, now < errorUntil);

  byte modeCount = 1;
  if (currentMode == IND_SLAVE) modeCount = 2;
  if (currentMode == IND_MASTER) modeCount = 3;

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

  if (stableButtonState == LOW &&
      buttonPressTime != 0 &&
      !buttonHoldTriggered) {

    if (now - buttonPressTime >= BUTTON_HOLD) {
      buttonEvent = true;
      buttonHoldTriggered = true;
    }
  }

  updateSound();
}