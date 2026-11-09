#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

namespace Encoder {
  void begin();
  void update();
  bool takeRotation(int8_t &direction);
  bool takeButtonPress();
}

#endif
