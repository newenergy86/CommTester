#ifndef INDICATORS_H
#define INDICATORS_H

#include <Arduino.h>

enum IndicatorMode {
  IND_SERVICE,
  IND_SLAVE
};

namespace Indicators {

  void begin();
  void update();

  void rx();
  void tx();
  void error();

  void setMode(IndicatorMode mode);
  IndicatorMode getMode();

  bool buttonPressed();

  void beep(byte count = 1);
}

#endif