#include "modbus_master.h"

#include "serial_console.h"
#include "indicators.h"
#include "modbus_config.h"

#include <ModbusRTUMaster.h>


// ============================================
// MODBUS MASTER
// ============================================

namespace ModbusMaster {


  // ============================================
  // LIBRARY INSTANCE
  // ============================================

  ModbusRTUMaster *modbus = nullptr;


  // ============================================
  // STATISTICS
  // ============================================

  uint32_t requests = 0;
  uint32_t responses = 0;
  uint32_t errors = 0;

  uint32_t crcErrors = 0;
  uint32_t timeouts = 0;
  uint32_t frameErrors = 0;
  uint32_t exceptionResponses = 0;

  uint32_t lastResponseTime = 0;


  // ============================================
  // LAST ERROR
  // ============================================

  uint8_t lastErrorCode = 0;
  uint8_t lastException = 0;


  // ============================================
  // ERROR MAPPING
  // ============================================

  Result convertError(uint8_t error) {

    lastErrorCode = error;

    switch (error) {

      case 0:
        return OK;


      case 4:

        timeouts++;

        return TIMEOUT;


      case 5:

        frameErrors++;

        return FRAME_ERROR;


      case 6:

        crcErrors++;

        return CRC_ERROR;


      case 8:

        return WRONG_SLAVE_ID;


      case 9:

        exceptionResponses++;

        lastException =
          modbus->getExceptionResponse();

        return EXCEPTION_RESPONSE;


      case 10:

        return WRONG_FUNCTION;


      case 2:
      case 3:

        return INVALID_ARGUMENT;


      default:

        return FRAME_ERROR;
    }
  }


  // ============================================
  // HANDLE RESULT
  // ============================================

  Result handleResult(uint8_t error) {

    Result result =
      convertError(error);


    if (result == OK) {

      responses++;

      Indicators::rx();
      Indicators::tx();

      Serial.println(
        "MASTER RESULT: OK"
      );

      return OK;
    }


    errors++;

    Serial.print(
      "MASTER ERROR CODE: "
    );

    Serial.println(
      error
    );


    if (result == TIMEOUT) {

      Serial.println(
        "MASTER RX: TIMEOUT"
      );
    }

    else if (result == FRAME_ERROR) {

      Serial.println(
        "MASTER RX: FRAME ERROR"
      );
    }

    else if (result == CRC_ERROR) {

      Serial.println(
        "MASTER RX: CRC ERROR"
      );
    }

    else if (result == WRONG_SLAVE_ID) {

      Serial.println(
        "MASTER ERROR: WRONG SLAVE ID"
      );
    }

    else if (result == WRONG_FUNCTION) {

      Serial.println(
        "MASTER ERROR: WRONG FUNCTION"
      );
    }

    else if (result == EXCEPTION_RESPONSE) {

      Serial.print(
        "MASTER ERROR: MODBUS EXCEPTION "
      );

      Serial.println(
        lastException
      );
    }

    else if (result == INVALID_ARGUMENT) {

      Serial.println(
        "MASTER ERROR: INVALID ARGUMENT"
      );
    }

    else {

      Serial.println(
        "MASTER ERROR: UNKNOWN"
      );
    }


    Indicators::error();

    return result;
  }


  // ============================================
  // BEGIN
  // ============================================

  void begin() {

    SoftwareSerial &rs232 =
      SerialConsole::getSerial();


    // ------------------------------------------
    // CREATE LIBRARY INSTANCE
    // ------------------------------------------

    if (modbus == nullptr) {

      modbus =
        new ModbusRTUMaster(rs232);
    }


    // ------------------------------------------
    // LIBRARY INITIALIZATION
    // ------------------------------------------

    modbus->begin(
      MODBUS_BAUD,
      SERIAL_8N1
    );


    // ------------------------------------------
    // RESPONSE TIMEOUT
    // ------------------------------------------

    modbus->setTimeout(
      MODBUS_RESPONSE_TIMEOUT_MS
    );


    resetStatistics();


    Serial.println();

    Serial.println(
      "MODBUS MASTER READY"
    );

    Serial.print(
      "Baud: "
    );

    Serial.println(
      MODBUS_BAUD
    );

    Serial.print(
      "Response timeout: "
    );

    Serial.print(
      MODBUS_RESPONSE_TIMEOUT_MS
    );

    Serial.println(
      " ms"
    );
  }


  // ============================================
  // FC01 — READ COILS
  // ============================================

  Result readCoils(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    bool *buffer
  ) {

    if (modbus == nullptr ||
        buffer == nullptr ||
        quantity == 0 ||
        quantity > 2000) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC01 READ COILS"
    );


    Indicators::tx();


    uint8_t error =
      modbus->readCoils(
        slaveId,
        startAddress,
        buffer,
        quantity
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC02 — READ DISCRETE INPUTS
  // ============================================

  Result readDiscreteInputs(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    bool *buffer
  ) {

    if (modbus == nullptr ||
        buffer == nullptr ||
        quantity == 0 ||
        quantity > 2000) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC02 READ DISCRETE INPUTS"
    );


    Indicators::tx();


    uint8_t error =
      modbus->readDiscreteInputs(
        slaveId,
        startAddress,
        buffer,
        quantity
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC03 — READ HOLDING REGISTERS
  // ============================================

  Result readHoldingRegisters(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    uint16_t *buffer
  ) {

    if (modbus == nullptr ||
        buffer == nullptr ||
        quantity == 0 ||
        quantity > 125) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC03 READ HOLDING REGISTERS"
    );


    Indicators::tx();


    uint8_t error =
      modbus->readHoldingRegisters(
        slaveId,
        startAddress,
        buffer,
        quantity
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC04 — READ INPUT REGISTERS
  // ============================================

  Result readInputRegisters(
    uint8_t slaveId,
    uint16_t startAddress,
    uint16_t quantity,
    uint16_t *buffer
  ) {

    if (modbus == nullptr ||
        buffer == nullptr ||
        quantity == 0 ||
        quantity > 125) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC04 READ INPUT REGISTERS"
    );


    Indicators::tx();


    uint8_t error =
      modbus->readInputRegisters(
        slaveId,
        startAddress,
        buffer,
        quantity
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC05 — WRITE SINGLE COIL
  // ============================================

  Result writeSingleCoil(
    uint8_t slaveId,
    uint16_t address,
    bool value
  ) {

    if (modbus == nullptr) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC05 WRITE SINGLE COIL"
    );


    Indicators::tx();


    uint8_t error =
      modbus->writeSingleCoil(
        slaveId,
        address,
        value
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC06 — WRITE SINGLE REGISTER
  // ============================================

  Result writeSingleRegister(
    uint8_t slaveId,
    uint16_t address,
    uint16_t value
  ) {

    if (modbus == nullptr) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC06 WRITE SINGLE REGISTER"
    );


    Indicators::tx();


    uint8_t error =
      modbus->writeSingleHoldingRegister(
        slaveId,
        address,
        value
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC15 — WRITE MULTIPLE COILS
  // ============================================

  Result writeMultipleCoils(
    uint8_t slaveId,
    uint16_t startAddress,
    const bool *buffer,
    uint16_t quantity
  ) {

    if (modbus == nullptr ||
        buffer == nullptr ||
        quantity == 0 ||
        quantity > 1968) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC15 WRITE MULTIPLE COILS"
    );


    Indicators::tx();


    uint8_t error =
      modbus->writeMultipleCoils(
        slaveId,
        startAddress,
        buffer,
        quantity
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // FC16 — WRITE MULTIPLE REGISTERS
  // ============================================

  Result writeMultipleRegisters(
    uint8_t slaveId,
    uint16_t startAddress,
    const uint16_t *buffer,
    uint16_t quantity
  ) {

    if (modbus == nullptr ||
        buffer == nullptr ||
        quantity == 0 ||
        quantity > 123) {

      errors++;

      Indicators::error();

      return INVALID_ARGUMENT;
    }


    requests++;

    unsigned long startTime =
      millis();


    Serial.println(
      "MASTER: FC16 WRITE MULTIPLE REGISTERS"
    );


    Indicators::tx();


    uint8_t error =
      modbus->writeMultipleHoldingRegisters(
        slaveId,
        startAddress,
        buffer,
        quantity
      );


    lastResponseTime =
      millis() - startTime;


    return handleResult(error);
  }


  // ============================================
  // STATISTICS
  // ============================================

  uint32_t getRequests() {

    return requests;
  }


  uint32_t getResponses() {

    return responses;
  }


  uint32_t getErrors() {

    return errors;
  }


  uint32_t getCrcErrors() {

    return crcErrors;
  }


  uint32_t getTimeouts() {

    return timeouts;
  }


  uint32_t getFrameErrors() {

    return frameErrors;
  }


  uint32_t getExceptionResponses() {

    return exceptionResponses;
  }


  uint32_t getLastResponseTime() {

    return lastResponseTime;
  }


  // ============================================
  // LAST ERROR
  // ============================================

  uint8_t getLastErrorCode() {

    return lastErrorCode;
  }


  uint8_t getLastException() {

    return lastException;
  }


  // ============================================
  // RESET STATISTICS
  // ============================================

  void resetStatistics() {

    requests = 0;
    responses = 0;
    errors = 0;

    crcErrors = 0;
    timeouts = 0;
    frameErrors = 0;
    exceptionResponses = 0;

    lastResponseTime = 0;

    lastErrorCode = 0;
    lastException = 0;
  }

}