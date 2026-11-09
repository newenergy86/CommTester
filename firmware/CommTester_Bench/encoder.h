#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

namespace Encoder {
  enum ButtonEvent : uint8_t {
    BUTTON_NONE = 0,
    BUTTON_SHORT,
    BUTTON_DOUBLE,
    BUTTON_LONG
  };

  void begin();
  void update();
  bool takeRotation(int8_t &direction);
  bool takeButtonEvent(ButtonEvent &event);
}

#endif
