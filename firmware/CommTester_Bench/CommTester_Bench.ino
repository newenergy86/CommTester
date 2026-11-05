#include "indicators.h"
#include "rs232_port.h"
#include "modbus_slave.h"
#include "display.h"

void setup() {
  Serial.begin(115200);
  Indicators::begin();
  Display::begin();

  RS232Port::begin();
  ModbusSlave::begin();

  Serial.println();
  Serial.println(F("=============================="));
  Serial.println(F(" CommTester Slave"));
  Serial.println(F("=============================="));
  Serial.println(F("RS-232 : 9600 8N1"));
  Serial.println(F("Mode   : MODBUS SLAVE"));
  Serial.println();
}

void loop() {
  Indicators::update();

  uint8_t slaveId = 1;
  uint8_t masterId = 0xFF;
  uint8_t functionCode = 0;
  uint32_t requests = 0;
  uint32_t responses = 0;

  ModbusSlave::getDisplayData(
    slaveId, masterId, functionCode, requests, responses
  );

  Display::update(
    slaveId, masterId, functionCode, requests, responses
  );

  ModbusSlave::update();
}
