#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

namespace Settings {
  void begin();
  uint8_t getSlaveId();
  uint8_t getBaudIndex();
  unsigned long getBaud();
  void save(uint8_t slaveId, uint8_t baudIndex);
}

#endif
