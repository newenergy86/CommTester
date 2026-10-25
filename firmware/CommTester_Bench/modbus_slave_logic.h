#ifndef MODBUS_SLAVE_LOGIC_H
#define MODBUS_SLAVE_LOGIC_H

#include <Arduino.h>
#include <ModbusADU.h>

class ModbusSlaveLogic {
public:
  void configureCoils(uint8_t* coils, uint16_t numCoils);
  void configureDiscreteInputs(uint8_t* inputs, uint16_t numInputs);
  void configureHoldingRegisters(uint16_t* registers, uint16_t count);
  void configureInputRegisters(uint16_t* registers, uint16_t count);
  void processPdu(ModbusADU& adu);

private:
  uint8_t* _coils = nullptr;
  uint8_t* _discreteInputs = nullptr;
  uint16_t* _holdingRegisters = nullptr;
  uint16_t* _inputRegisters = nullptr;

  uint16_t _numCoils = 0;
  uint16_t _numDiscreteInputs = 0;
  uint16_t _numHoldingRegisters = 0;
  uint16_t _numInputRegisters = 0;

  void _processReadCoils(ModbusADU& adu);
  void _processReadDiscreteInputs(ModbusADU& adu);
  void _processReadHoldingRegisters(ModbusADU& adu);
  void _processReadInputRegisters(ModbusADU& adu);
  void _processWriteSingleCoil(ModbusADU& adu);
  void _processWriteSingleHoldingRegister(ModbusADU& adu);
  void _processWriteMultipleCoils(ModbusADU& adu);
  void _processWriteMultipleHoldingRegisters(ModbusADU& adu);

  void _processReadBits(ModbusADU& adu, uint8_t* buffer, uint16_t sizeBits);
  void _processReadRegisters(ModbusADU& adu, uint16_t* buffer, uint16_t size);

  static bool _getBit(const uint8_t* buffer, uint16_t bit);
  static void _setBit(uint8_t* buffer, uint16_t bit, bool value);
};

#endif
