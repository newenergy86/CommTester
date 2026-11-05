#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <Arduino.h>

namespace ModbusSlave {

  void begin();
  void update();
  void getDisplayData(uint8_t &slaveId, uint8_t &lastRequestId,
                      uint8_t &lastFunctionCode,
                      uint32_t &rx, uint32_t &tx,
                      uint32_t &errors);

}

#endif
