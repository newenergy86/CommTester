#ifndef INDICATORS_H
#define INDICATORS_H

#include <Arduino.h>

namespace Indicators {
  void begin();
  void update();
  void rx();
  void tx();
  void error();
  void encoderStep();
  void encoderPress();
}

#endif
