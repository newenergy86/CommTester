#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <Arduino.h>
#include "ModbusADU.h"


enum ModbusRTUError : uint8_t {
  MODBUS_RTU_SUCCESS = 0,
  MODBUS_RTU_TIMEOUT = 1,
  MODBUS_RTU_FRAME_ERROR = 2,
  MODBUS_RTU_CRC_ERROR = 3
};


class ModbusRTU {

  public:

    ModbusRTU(Stream& serial,
              int dePin = -1,
              int rePin = -1);

    void begin(unsigned long baud,
               uint32_t config = SERIAL_8N1);

    void setTimeout(unsigned long timeout);

    ModbusRTUError readAdu(ModbusADU& adu);

    bool writeAdu(ModbusADU& adu);

  private:

    Stream& _serial;

    int _dePin;
    int _rePin;

    unsigned long _t15;
    unsigned long _t35;
    unsigned long _bytePeriod;
    unsigned long _postDelay;
    unsigned long _readTimeout;

    void setTransmitMode();
    void setReceiveMode();
    void calculateTiming();
};


#endif