#include "modbus_slave_logic.h"

void ModbusSlaveLogic::configureCoils(uint8_t* coils, uint16_t numCoils) {
  _coils = coils;
  _numCoils = numCoils;
}

void ModbusSlaveLogic::configureDiscreteInputs(uint8_t* inputs, uint16_t numInputs) {
  _discreteInputs = inputs;
  _numDiscreteInputs = numInputs;
}

void ModbusSlaveLogic::configureHoldingRegisters(uint16_t* registers, uint16_t count) {
  _holdingRegisters = registers;
  _numHoldingRegisters = count;
}

void ModbusSlaveLogic::configureInputRegisters(uint16_t* registers, uint16_t count) {
  _inputRegisters = registers;
  _numInputRegisters = count;
}

void ModbusSlaveLogic::processPdu(ModbusADU& adu) {
  switch (adu.getFunctionCode()) {
    case 1:  _processReadCoils(adu); break;
    case 2:  _processReadDiscreteInputs(adu); break;
    case 3:  _processReadHoldingRegisters(adu); break;
    case 4:  _processReadInputRegisters(adu); break;
    case 5:  _processWriteSingleCoil(adu); break;
    case 6:  _processWriteSingleHoldingRegister(adu); break;
    case 15: _processWriteMultipleCoils(adu); break;
    case 16: _processWriteMultipleHoldingRegisters(adu); break;
    default: adu.prepareExceptionResponse(1); break;
  }
}

void ModbusSlaveLogic::_processReadCoils(ModbusADU& adu) {
  _processReadBits(adu, _coils, _numCoils);
}

void ModbusSlaveLogic::_processReadDiscreteInputs(ModbusADU& adu) {
  _processReadBits(adu, _discreteInputs, _numDiscreteInputs);
}

void ModbusSlaveLogic::_processReadHoldingRegisters(ModbusADU& adu) {
  _processReadRegisters(adu, _holdingRegisters, _numHoldingRegisters);
}

void ModbusSlaveLogic::_processReadInputRegisters(ModbusADU& adu) {
  _processReadRegisters(adu, _inputRegisters, _numInputRegisters);
}

void ModbusSlaveLogic::_processReadBits(ModbusADU& adu, uint8_t* buffer, uint16_t sizeBits) {
  if (adu.getUnitId() == 0) return;

  uint16_t start = adu.getDataRegister(0);
  uint16_t quantity = adu.getDataRegister(2);

  if (!buffer || sizeBits == 0) {
    adu.prepareExceptionResponse(1);
    return;
  }
  if (quantity == 0 || quantity > 2000) {
    adu.prepareExceptionResponse(3);
    return;
  }
  if (quantity > sizeBits || start > sizeBits - quantity) {
    adu.prepareExceptionResponse(2);
    return;
  }

  uint8_t byteCount = (quantity + 7) / 8;
  adu.data[0] = byteCount;

  for (uint16_t i = 0; i < quantity; i++) {
    if (_getBit(buffer, start + i))
      adu.data[1 + i / 8] |= (uint8_t)(1U << (i % 8));
    else
      adu.data[1 + i / 8] &= (uint8_t)~(1U << (i % 8));
  }

  adu.setDataLen(1 + byteCount);
}

void ModbusSlaveLogic::_processReadRegisters(ModbusADU& adu, uint16_t* buffer, uint16_t size) {
  if (adu.getUnitId() == 0) return;

  uint16_t start = adu.getDataRegister(0);
  uint16_t quantity = adu.getDataRegister(2);

  if (!buffer || size == 0) {
    adu.prepareExceptionResponse(1);
    return;
  }
  if (quantity == 0 || quantity > 125) {
    adu.prepareExceptionResponse(3);
    return;
  }
  if (quantity > size || start > size - quantity) {
    adu.prepareExceptionResponse(2);
    return;
  }

  adu.data[0] = (uint8_t)(quantity * 2);

  for (uint16_t i = 0; i < quantity; i++) {
    uint16_t value = buffer[start + i];
    adu.data[1 + i * 2] = highByte(value);
    adu.data[2 + i * 2] = lowByte(value);
  }

  adu.setDataLen(1 + quantity * 2);
}

void ModbusSlaveLogic::_processWriteSingleCoil(ModbusADU& adu) {
  uint16_t address = adu.getDataRegister(0);
  uint16_t value = adu.getDataRegister(2);

  if (!_coils || _numCoils == 0) {
    adu.prepareExceptionResponse(1);
    return;
  }
  if (value != 0 && value != 0xFF00) {
    adu.prepareExceptionResponse(3);
    return;
  }
  if (address >= _numCoils) {
    adu.prepareExceptionResponse(2);
    return;
  }

  _setBit(_coils, address, value == 0xFF00);
  adu.setDataLen(4);
}

void ModbusSlaveLogic::_processWriteSingleHoldingRegister(ModbusADU& adu) {
  uint16_t address = adu.getDataRegister(0);
  uint16_t value = adu.getDataRegister(2);

  if (!_holdingRegisters || _numHoldingRegisters == 0) {
    adu.prepareExceptionResponse(1);
    return;
  }
  if (address >= _numHoldingRegisters) {
    adu.prepareExceptionResponse(2);
    return;
  }

  _holdingRegisters[address] = value;
  adu.setDataLen(4);
}

void ModbusSlaveLogic::_processWriteMultipleCoils(ModbusADU& adu) {
  uint16_t start = adu.getDataRegister(0);
  uint16_t quantity = adu.getDataRegister(2);
  uint8_t byteCount = adu.data[4];

  if (!_coils || _numCoils == 0) {
    adu.prepareExceptionResponse(1);
    return;
  }
  if (quantity == 0 || quantity > 1968 ||
      byteCount != (uint8_t)((quantity + 7) / 8)) {
    adu.prepareExceptionResponse(3);
    return;
  }
  if (quantity > _numCoils || start > _numCoils - quantity) {
    adu.prepareExceptionResponse(2);
    return;
  }

  for (uint16_t i = 0; i < quantity; i++) {
    bool value = (adu.data[5 + i / 8] & (uint8_t)(1U << (i % 8))) != 0;
    _setBit(_coils, start + i, value);
  }

  adu.setDataLen(4);
}

void ModbusSlaveLogic::_processWriteMultipleHoldingRegisters(ModbusADU& adu) {
  uint16_t start = adu.getDataRegister(0);
  uint16_t quantity = adu.getDataRegister(2);
  uint8_t byteCount = adu.data[4];

  if (!_holdingRegisters || _numHoldingRegisters == 0) {
    adu.prepareExceptionResponse(1);
    return;
  }
  if (quantity == 0 || quantity > 123 ||
      byteCount != (uint8_t)(quantity * 2)) {
    adu.prepareExceptionResponse(3);
    return;
  }
  if (quantity > _numHoldingRegisters || start > _numHoldingRegisters - quantity) {
    adu.prepareExceptionResponse(2);
    return;
  }

  for (uint16_t i = 0; i < quantity; i++)
    _holdingRegisters[start + i] = adu.getDataRegister(5 + i * 2);

  adu.setDataLen(4);
}

bool ModbusSlaveLogic::_getBit(const uint8_t* buffer, uint16_t bit) {
  return (buffer[bit / 8] & (uint8_t)(1U << (bit % 8))) != 0;
}

void ModbusSlaveLogic::_setBit(uint8_t* buffer, uint16_t bit, bool value) {
  uint8_t mask = (uint8_t)(1U << (bit % 8));
  if (value) buffer[bit / 8] |= mask;
  else buffer[bit / 8] &= (uint8_t)~mask;
}
