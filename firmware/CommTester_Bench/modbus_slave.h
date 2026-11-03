#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <Arduino.h>

namespace ModbusSlave {

  void begin();
  void update();
  void getStatistics(uint32_t &requests, uint32_t &responses);

}

#endif
