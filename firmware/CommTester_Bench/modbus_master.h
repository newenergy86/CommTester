#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <Arduino.h>

namespace ModbusMaster {

  // ============================================
  // RESULT CODES
  // ============================================

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


  // ============================================
  // INITIALIZATION
  // ============================================

  void begin();


  // ============================================
  // READ FUNCTIONS
  // ============================================

  // FC01 — Read Coils
  //
  // buffer должен иметь размер не менее quantity.
  //
  Result readCoils(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    bool *buffer
  );


  // FC02 — Read Discrete Inputs
  //
  // buffer должен иметь размер не менее quantity.
  //
  Result readDiscreteInputs(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    bool *buffer
  );


  // FC03 — Read Holding Registers
  //
  // buffer должен иметь размер не менее quantity.
  //
  Result readHoldingRegisters(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    uint16_t *buffer
  );


  // FC04 — Read Input Registers
  //
  // buffer должен иметь размер не менее quantity.
  //
  Result readInputRegisters(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    uint16_t *buffer
  );


  // ============================================
  // WRITE FUNCTIONS
  // ============================================

  // FC05 — Write Single Coil
  //
  Result writeSingleCoil(
    uint8_t slaveId,
    uint16_t address,
    bool value
  );


  // FC06 — Write Single Holding Register
  //
  Result writeSingleRegister(
    uint8_t slaveId,
    uint16_t address,
    uint16_t value
  );


  // FC15 — Write Multiple Coils
  //
  // buffer содержит quantity значений bool.
  //
  Result writeMultipleCoils(
    uint8_t slaveId,
    uint16_t startAddress,
    const bool *buffer,
    uint16_t quantity
  );


  // FC16 — Write Multiple Holding Registers
  //
  // buffer содержит quantity регистров.
  //
  Result writeMultipleRegisters(
    uint8_t slaveId,
    uint16_t startAddress,
    const uint16_t *buffer,
    uint16_t quantity
  );


  // ============================================
  // STATISTICS
  // ============================================

  uint32_t getRequests();
  uint32_t getResponses();
  uint32_t getErrors();

  uint32_t getCrcErrors();
  uint32_t getTimeouts();
  uint32_t getFrameErrors();
  uint32_t getExceptionResponses();

  uint32_t getLastResponseTime();


  // ============================================
  // LAST ERROR
  // ============================================

  // Последний код ошибки библиотеки ModbusRTUMaster.
  //
  // 0  = success
  // 1  = invalid unit ID
  // 2  = invalid buffer
  // 3  = invalid quantity
  // 4  = response timeout
  // 5  = frame error
  // 6  = CRC error
  // 7  = unknown communication error
  // 8  = unexpected unit ID
  // 9  = exception response
  // 10 = unexpected function
  // 11 = unexpected response length
  // 12 = unexpected byte count
  // 13 = unexpected data address
  // 14 = unexpected data value
  // 15 = unexpected quantity

  uint8_t getLastErrorCode();

  uint8_t getLastException();


  // ============================================
  // RESET STATISTICS
  // ============================================

  void resetStatistics();

}

#endif