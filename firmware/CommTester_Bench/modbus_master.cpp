#include "modbus_master.h"

#include "serial_console.h"
#include "indicators.h"
#include "modbus_rtu.h"
#include "modbus_config.h"

namespace ModbusMaster {

  ModbusRTU *modbusComm = nullptr;

  const unsigned long RESPONSE_TIMEOUT = 500;

  uint32_t requests = 0;
  uint32_t responses = 0;
  uint32_t errors = 0;
  uint32_t crcErrors = 0;
  uint32_t timeouts = 0;
  uint32_t frameErrors = 0;
  uint32_t exceptionResponses = 0;
  uint32_t lastResponseTime = 0;

  void printFrame(const char *prefix, ModbusADU &adu) {
    Serial.print(prefix);
    Serial.print(": ");

    for (uint16_t i = 0; i < adu.getRtuLen(); i++) {
      if (adu.rtu[i] < 0x10) Serial.print("0");
      Serial.print(adu.rtu[i], HEX);
      if (i < adu.getRtuLen() - 1) Serial.print(" ");
    }
    Serial.println();
  }

  void buildRequest(ModbusADU &adu, uint8_t slaveId, uint8_t function,
                    uint16_t startAddress, uint16_t quantity) {
    adu.rtu[0] = slaveId;
    adu.rtu[1] = function;
    adu.rtu[2] = highByte(startAddress);
    adu.rtu[3] = lowByte(startAddress);
    adu.rtu[4] = highByte(quantity);
    adu.rtu[5] = lowByte(quantity);
    adu.setRtuLen(6);
    adu.updateCrc();
  }

  void buildWriteSingleCoil(ModbusADU &adu, uint8_t slaveId,
                            uint16_t address, bool value) {
    adu.rtu[0] = slaveId;
    adu.rtu[1] = 5;
    adu.rtu[2] = highByte(address);
    adu.rtu[3] = lowByte(address);
    uint16_t coilValue = value ? 0xFF00 : 0x0000;
    adu.rtu[4] = highByte(coilValue);
    adu.rtu[5] = lowByte(coilValue);
    adu.setRtuLen(6);
    adu.updateCrc();
  }

  void buildWriteSingleRegister(ModbusADU &adu, uint8_t slaveId,
                                uint16_t address, uint16_t value) {
    adu.rtu[0] = slaveId;
    adu.rtu[1] = 6;
    adu.rtu[2] = highByte(address);
    adu.rtu[3] = lowByte(address);
    adu.rtu[4] = highByte(value);
    adu.rtu[5] = lowByte(value);
    adu.setRtuLen(6);
    adu.updateCrc();
  }

  bool buildWriteMultipleCoils(ModbusADU &adu, uint8_t slaveId,
                               uint16_t startAddress, const uint8_t *data,
                               uint16_t quantity) {
    if (data == nullptr) return false;
    if (quantity == 0 || quantity > 1968) return false;

    uint16_t byteCount = (quantity + 7) / 8;
    uint16_t length = 7 + byteCount;

    adu.rtu[0] = slaveId;
    adu.rtu[1] = 15;
    adu.rtu[2] = highByte(startAddress);
    adu.rtu[3] = lowByte(startAddress);
    adu.rtu[4] = highByte(quantity);
    adu.rtu[5] = lowByte(quantity);
    adu.rtu[6] = byteCount;

    for (uint16_t i = 0; i < byteCount; i++) {
      adu.rtu[7 + i] = data[i];
    }

    adu.setRtuLen(length);
    adu.updateCrc();
    return true;
  }

  bool buildWriteMultipleRegisters(ModbusADU &adu, uint8_t slaveId,
                                   uint16_t startAddress,
                                   const uint16_t *values,
                                   uint16_t quantity) {
    if (values == nullptr) return false;
    if (quantity == 0 || quantity > 123) return false;

    uint16_t byteCount = quantity * 2;

    adu.rtu[0] = slaveId;
    adu.rtu[1] = 16;
    adu.rtu[2] = highByte(startAddress);
    adu.rtu[3] = lowByte(startAddress);
    adu.rtu[4] = highByte(quantity);
    adu.rtu[5] = lowByte(quantity);
    adu.rtu[6] = byteCount;

    for (uint16_t i = 0; i < quantity; i++) {
      adu.rtu[7 + i * 2] = highByte(values[i]);
      adu.rtu[8 + i * 2] = lowByte(values[i]);
    }

    adu.setRtuLen(7 + byteCount);
    adu.updateCrc();
    return true;
  }

  Result exchange(ModbusADU &request, uint8_t expectedSlave,
                  uint8_t expectedFunction) {
    if (modbusComm == nullptr) return FRAME_ERROR;

    requests++;

    printFrame("MASTER TX", request);

    bool writeResult = modbusComm->writeAdu(request);
    (void)writeResult;

    unsigned long startTime = millis();
    ModbusADU response;
    ModbusRTUError error = modbusComm->readAdu(response);
    lastResponseTime = millis() - startTime;

    if (error == MODBUS_RTU_TIMEOUT) {
      timeouts++;
      errors++;
      Serial.println("MASTER RX: TIMEOUT");
      Indicators::error();
      return TIMEOUT;
    }

    if (error == MODBUS_RTU_FRAME_ERROR) {
      frameErrors++;
      errors++;
      Serial.println("MASTER RX: FRAME ERROR");
      Indicators::error();
      return FRAME_ERROR;
    }

    if (error == MODBUS_RTU_CRC_ERROR) {
      crcErrors++;
      errors++;
      Serial.println("MASTER RX: CRC ERROR");
      Indicators::error();
      return CRC_ERROR;
    }

    if (error != MODBUS_RTU_SUCCESS) {
      errors++;
      Serial.print("MASTER RX ERROR: ");
      Serial.println(error);
      Indicators::error();
      return FRAME_ERROR;
    }

    responses++;
    Indicators::rx();
    printFrame("MASTER RX", response);

    if (response.getUnitId() != expectedSlave) {
      errors++;
      Serial.println("MASTER ERROR: WRONG SLAVE ID");
      Indicators::error();
      return WRONG_SLAVE_ID;
    }

    if (response.getFunctionCode() == (expectedFunction | 0x80)) {
      exceptionResponses++;
      errors++;
      Serial.println("MASTER ERROR: MODBUS EXCEPTION");
      Indicators::error();
      return EXCEPTION_RESPONSE;
    }

    if (response.getFunctionCode() != expectedFunction) {
      errors++;
      Serial.println("MASTER ERROR: WRONG FUNCTION");
      Indicators::error();
      return WRONG_FUNCTION;
    }

    Serial.println("MASTER RESULT: OK");
    Indicators::tx();
    return OK;
  }

  void begin() {
    SoftwareSerial &rs232 = SerialConsole::getSerial();
    modbusComm = new ModbusRTU(rs232);
    modbusComm->begin(MODBUS_BAUD, SERIAL_8N1);
    modbusComm->setTimeout(RESPONSE_TIMEOUT);
    resetStatistics();

    Serial.println();
    Serial.println("MODBUS MASTER READY");
    Serial.print("Baud: ");
    Serial.println(MODBUS_BAUD);
    Serial.print("Response timeout: ");
    Serial.print(RESPONSE_TIMEOUT);
    Serial.println(" ms");
  }

  Result readCoils(uint8_t slaveId, uint16_t startAddress,
                   uint16_t quantity) {
    if (quantity == 0 || quantity > 125) return INVALID_ARGUMENT;
    ModbusADU adu;
    buildRequest(adu, slaveId, 1, startAddress, quantity);
    return exchange(adu, slaveId, 1);
  }

  Result readDiscreteInputs(uint8_t slaveId, uint16_t startAddress,
                            uint16_t quantity) {
    if (quantity == 0 || quantity > 125) return INVALID_ARGUMENT;
    ModbusADU adu;
    buildRequest(adu, slaveId, 2, startAddress, quantity);
    return exchange(adu, slaveId, 2);
  }

  Result readHoldingRegisters(uint8_t slaveId, uint16_t startAddress,
                              uint16_t quantity) {
    if (quantity == 0 || quantity > 125) return INVALID_ARGUMENT;
    ModbusADU adu;
    buildRequest(adu, slaveId, 3, startAddress, quantity);
    return exchange(adu, slaveId, 3);
  }

  Result readInputRegisters(uint8_t slaveId, uint16_t startAddress,
                            uint16_t quantity) {
    if (quantity == 0 || quantity > 125) return INVALID_ARGUMENT;
    ModbusADU adu;
    buildRequest(adu, slaveId, 4, startAddress, quantity);
    return exchange(adu, slaveId, 4);
  }

  Result writeSingleCoil(uint8_t slaveId, uint16_t address, bool value) {
    ModbusADU adu;
    buildWriteSingleCoil(adu, slaveId, address, value);
    return exchange(adu, slaveId, 5);
  }

  Result writeSingleRegister(uint8_t slaveId, uint16_t address,
                             uint16_t value) {
    ModbusADU adu;
    buildWriteSingleRegister(adu, slaveId, address, value);
    return exchange(adu, slaveId, 6);
  }

  Result writeMultipleCoils(uint8_t slaveId, uint16_t startAddress,
                            const uint8_t *data, uint16_t quantity) {
    ModbusADU adu;
    if (!buildWriteMultipleCoils(adu, slaveId, startAddress, data, quantity))
      return INVALID_ARGUMENT;
    return exchange(adu, slaveId, 15);
  }

  Result writeMultipleRegisters(uint8_t slaveId, uint16_t startAddress,
                                const uint16_t *values, uint16_t quantity) {
    ModbusADU adu;
    if (!buildWriteMultipleRegisters(adu, slaveId, startAddress, values, quantity))
      return INVALID_ARGUMENT;
    return exchange(adu, slaveId, 16);
  }

  uint32_t getRequests() { return requests; }
  uint32_t getResponses() { return responses; }
  uint32_t getErrors() { return errors; }
  uint32_t getCrcErrors() { return crcErrors; }
  uint32_t getTimeouts() { return timeouts; }
  uint32_t getFrameErrors() { return frameErrors; }
  uint32_t getExceptionResponses() { return exceptionResponses; }
  uint32_t getLastResponseTime() { return lastResponseTime; }

  void resetStatistics() {
    requests = 0;
    responses = 0;
    errors = 0;
    crcErrors = 0;
    timeouts = 0;
    frameErrors = 0;
    exceptionResponses = 0;
    lastResponseTime = 0;
  }

}