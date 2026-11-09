#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <Arduino.h>

namespace ModbusSlave {

  void begin(uint8_t initialSlaveId, unsigned long baud);
  void setSlaveId(uint8_t slaveId);
  void setBaud(unsigned long baud);
  void resetTrafficCounters();
  void update();
  void getDisplayData(uint8_t &slaveId, uint8_t &lastRequestId,
                      uint8_t &lastFunctionCode,
                      uint32_t &rx, uint32_t &tx,
                      uint32_t &errors, uint32_t &missed);

}

#endif
