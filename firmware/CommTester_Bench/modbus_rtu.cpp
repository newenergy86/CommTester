#include "modbus_rtu.h"
#include "modbus_config.h"


ModbusRTU::ModbusRTU(Stream& serial,
                     int dePin,
                     int rePin)
  : _serial(serial)
{
  _dePin = dePin;
  _rePin = rePin;

  _t15 = 0;
  _t35 = 0;
  _bytePeriod = 0;
  _postDelay = 0;
  _readTimeout = 0;
}


void ModbusRTU::begin(unsigned long baud,
                       uint32_t config)
{
  unsigned long bitsPerChar;

  switch (config) {

    case SERIAL_8E2:
    case SERIAL_8O2:
      bitsPerChar = 12;
      break;

    case SERIAL_8N2:
    case SERIAL_8E1:
    case SERIAL_8O1:
      bitsPerChar = 11;
      break;

    case SERIAL_8N1:
    default:
      bitsPerChar = 10;
      break;
  }

  _bytePeriod =
    (bitsPerChar * 1000000UL) / baud;

  unsigned long tCharUs =
    (MODBUS_BITS_PER_CHAR * 1000000UL) / MODBUS_BAUD;

  _t15 = (tCharUs * 15UL) / 10UL;
  _t35 = (tCharUs * 35UL) / 10UL;

  _postDelay = _bytePeriod;

  if (_dePin >= 0) {
    pinMode(_dePin, OUTPUT);
    digitalWrite(_dePin, LOW);
  }

  if (_rePin >= 0) {
    pinMode(_rePin, OUTPUT);
    digitalWrite(_rePin, LOW);
  }

  unsigned long startMicros = micros();

  do {
    if (_serial.available() > 0) {
      startMicros = micros();
      _serial.read();
    }
  } while (micros() - startMicros < _t35);
}


void ModbusRTU::setTimeout(unsigned long timeout)
{
  _readTimeout = timeout;
}


void ModbusRTU::calculateTiming()
{
}


void ModbusRTU::setTransmitMode()
{
  if (_dePin >= 0) {
    digitalWrite(_dePin, HIGH);
  }

  if (_rePin >= 0) {
    digitalWrite(_rePin, HIGH);
  }
}


void ModbusRTU::setReceiveMode()
{
  if (_dePin >= 0) {
    digitalWrite(_dePin, LOW);
  }

  if (_rePin >= 0) {
    digitalWrite(_rePin, LOW);
  }
}


ModbusRTUError ModbusRTU::readAdu(ModbusADU& adu)
{
  adu.setRtuLen(0);

  unsigned long startMillis = millis();

  while (!_serial.available()) {
    if (millis() - startMillis >= _readTimeout) {
      return MODBUS_RTU_TIMEOUT;
    }
  }

  uint16_t len = 0;
  unsigned long lastByteMicros = micros();

  while (len < 256) {

    if (_serial.available()) {
      lastByteMicros = micros();
      adu.rtu[len] = _serial.read();
      len++;
      continue;
    }

    if (micros() - lastByteMicros > _t15) {
      break;
    }
  }

  adu.setRtuLen(len);

  while (micros() - lastByteMicros < _t35) {
  }

  if (_serial.available()) {
    adu.setRtuLen(0);
    return MODBUS_RTU_FRAME_ERROR;
  }

  if (!adu.crcGood()) {
    adu.setRtuLen(0);
    return MODBUS_RTU_CRC_ERROR;
  }

  return MODBUS_RTU_SUCCESS;
}


bool ModbusRTU::writeAdu(ModbusADU& adu)
{
  adu.updateCrc();

  uint16_t len = adu.getRtuLen();

  setTransmitMode();

  for (uint16_t i = 0; i < len; i++) {
    _serial.write(adu.rtu[i]);
  }

  _serial.flush();

  unsigned long startMicros = micros();

  while (micros() - startMicros < _postDelay) {
  }

  setReceiveMode();

  return true;
}