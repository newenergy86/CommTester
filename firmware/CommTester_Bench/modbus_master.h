#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <Arduino.h>
#include <ModbusADU.h>

namespace ModbusMaster {

  enum Result : uint8_t {
    OK = 0,
    TIMEOUT,
    CRC_ERROR,
    FRAME_ERROR,
    WRITE_ERROR,
    WRONG_SLAVE_ID,
    WRONG_FUNCTION,
    EXCEPTION_RESPONSE,
    INVALID_ARGUMENT
  };

  void begin();

  Result readCoils(uint8_t slaveId, uint16_t startAddress, uint16_t quantity);
  Result readDiscreteInputs(uint8_t slaveId, uint16_t startAddress, uint16_t quantity);
  Result readHoldingRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity);
  Result readInputRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity);

  Result writeSingleCoil(uint8_t slaveId, uint16_t address, bool value);
  Result writeSingleRegister(uint8_t slaveId, uint16_t address, uint16_t value);
  Result writeMultipleCoils(uint8_t slaveId, uint16_t startAddress, const uint8_t *data, uint16_t quantity);
  Result writeMultipleRegisters(uint8_t slaveId, uint16_t startAddress, const uint16_t *values, uint16_t quantity);

  uint32_t getRequests();
  uint32_t getResponses();
  uint32_t getErrors();
  uint32_t getCrcErrors();
  uint32_t getTimeouts();
  uint32_t getFrameErrors();
  uint32_t getExceptionResponses();
  uint32_t getLastResponseTime();

  void resetStatistics();

}

#endif