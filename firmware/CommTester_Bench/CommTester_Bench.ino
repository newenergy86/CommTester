#include "indicators.h"
#include "rs232_port.h"
#include "modbus_slave.h"
#include "display.h"
#include "encoder.h"

void setup() {
  Serial.begin(115200);
  Indicators::begin();
  Display::begin();
  Encoder::begin();

  RS232Port::begin();
  ModbusSlave::begin();

  Serial.println();
  Serial.println(F("=============================="));
  Serial.println(F(" CommTester Slave"));
  Serial.println(F("=============================="));
  Serial.println(F("RS-232 : 9600 8N1"));
  Serial.println(F("Mode   : MODBUS SLAVE"));
  Serial.println(F("Encoder: D8/D9 + D7"));
  Serial.println();
}

void loop() {
  Indicators::update();
  Encoder::update();

  uint8_t slaveId = 1;
  uint8_t masterId = 0xFF;
  uint8_t functionCode = 0;
  uint32_t rx = 0;
  uint32_t tx = 0;
  uint32_t errors = 0;
  uint32_t missed = 0;

  ModbusSlave::getDisplayData(
    slaveId, masterId, functionCode, rx, tx, errors, missed
  );

  Display::update(
    slaveId, masterId, functionCode, rx, tx, errors, missed
  );

  ModbusSlave::update();
}
