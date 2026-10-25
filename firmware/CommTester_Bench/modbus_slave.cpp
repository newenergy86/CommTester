#include "modbus_slave.h"
#include "serial_console.h"
#include "indicators.h"
#include "modbus_rtu.h"
#include "modbus_config.h"

#include <ModbusADU.h>
#include "modbus_slave_logic.h"


// ============================================
// MODBUS SLAVE
// ============================================

namespace ModbusSlave {

  const byte SLAVE_ID = 1;

  const uint16_t NUM_HOLDING_REGISTERS = 125;
  const uint16_t NUM_COILS = 125;
  const uint16_t NUM_DISCRETE_INPUTS = 125;
  const uint16_t NUM_INPUT_REGISTERS = 125;

  uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];
  uint8_t coils[(NUM_COILS + 7) / 8];
  uint8_t discreteInputs[(NUM_DISCRETE_INPUTS + 7) / 8];
  uint16_t inputRegisters[NUM_INPUT_REGISTERS];

  ModbusRTU *modbusComm = nullptr;
  ModbusSlaveLogic modbusLogic;


  void begin() {

    for (uint16_t i = 0;
         i < NUM_HOLDING_REGISTERS;
         i++) {
      holdingRegisters[i] = (i + 1) * 100;
    }

    for (uint16_t i = 0;
         i < (NUM_COILS + 7) / 8;
         i++) {
      coils[i] = 0;
    }

    for (uint16_t i = 0;
        i < (NUM_COILS + 7) / 8;
        i++) {
      coils[i] = 0xAA;
    }

    for (uint16_t i = 0;
         i < (NUM_DISCRETE_INPUTS + 7) / 8;
         i++) {
      discreteInputs[i] = 0;
    }

    for (uint16_t i = 0;
        i < (NUM_DISCRETE_INPUTS + 7) / 8;
        i++) {
      discreteInputs[i] = 0x55;
    }

    for (uint16_t i = 0;
         i < NUM_INPUT_REGISTERS;
         i++) {
      inputRegisters[i] = 1000 + i;
    }

    SoftwareSerial &rs232 =
      SerialConsole::getSerial();

    modbusComm =
      new ModbusRTU(rs232);

    modbusLogic.configureHoldingRegisters(
      holdingRegisters,
      NUM_HOLDING_REGISTERS
    );

    modbusLogic.configureCoils(
      coils,
      NUM_COILS
    );

    modbusLogic.configureDiscreteInputs(
      discreteInputs,
      NUM_DISCRETE_INPUTS
    );

    modbusLogic.configureInputRegisters(
      inputRegisters,
      NUM_INPUT_REGISTERS
    );

    modbusComm->begin(
      MODBUS_BAUD,
      SERIAL_8N1
    );
  }


  void update() {

    if (modbusComm == nullptr) {
      return;
    }

    ModbusADU adu;

    ModbusRTUError error =
      modbusComm->readAdu(adu);

    if (error == MODBUS_RTU_TIMEOUT) {
      return;
    }

    Serial.println();
    Serial.println("FRAME RECEIVED");

    Serial.print("Length: ");
    Serial.println(adu.getRtuLen());

    Serial.print("ID: ");
    Serial.println(adu.getUnitId());

    Serial.print("Function: ");
    Serial.println(adu.getFunctionCode());

    Serial.print("CRC: ");

    if (adu.crcGood()) {
      Serial.println("OK");
    }
    else {
      Serial.println("BAD");
    }

    if (adu.getRtuLen() > 0) {

      Serial.print("RAW: ");

      for (uint16_t i = 0;
           i < adu.getRtuLen();
           i++) {

        if (adu.rtu[i] < 0x10) {
          Serial.print("0");
        }

        Serial.print(
          adu.rtu[i],
          HEX
        );

        Serial.print(" ");
      }

      Serial.println();
    }

    if (error != MODBUS_RTU_SUCCESS) {

      Serial.print("ERROR CODE: ");
      Serial.println(error);

      Serial.print("CRC CHECK: ");
      Serial.println(adu.crcGood() ? "OK" : "BAD");

      Indicators::error();
      return;
    }

    if (!adu.crcGood()) {

      Serial.println("CRC ERROR");

      Indicators::error();
      return;
    }

    Indicators::rx();

    if (adu.getUnitId() != SLAVE_ID) {

      Serial.println("FOREIGN SLAVE ID");
      return;
    }

    modbusLogic.processPdu(adu);

    Serial.println();

    adu.updateCrc();

    Serial.println("RESPONSE PREPARED");

    Serial.print("Length: ");
    Serial.println(adu.getRtuLen());

    Serial.print("ID: ");
    Serial.println(adu.getUnitId());

    Serial.print("Function: ");
    Serial.println(adu.getFunctionCode());

    Serial.print("CRC: ");

    if (adu.crcGood()) {
      Serial.println("OK");
    }
    else {
      Serial.println("BAD");
    }

    if (adu.getRtuLen() > 0) {

      Serial.print("RAW: ");

      for (uint16_t i = 0;
           i < adu.getRtuLen();
           i++) {

        if (adu.rtu[i] < 0x10) {
          Serial.print("0");
        }

        Serial.print(
          adu.rtu[i],
          HEX
        );

        Serial.print(" ");
      }

      Serial.println();
    }

    Serial.println("TRANSMITTING");

    modbusComm->writeAdu(adu);

    Serial.println("RESPONSE SENT");

    Indicators::tx();
  }

}