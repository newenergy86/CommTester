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

  // ------------------------------------------
  // SLAVE ADDRESS
  // ------------------------------------------

  const byte SLAVE_ID = 1;


  // ------------------------------------------
  // TRAFFIC STATISTICS
  // ------------------------------------------

  uint32_t requestCount = 0;
  uint32_t responseCount = 0;
  uint8_t lastRequestId = 0xFF;
  uint8_t lastFunctionCode = 0;


  // ------------------------------------------
  // MODBUS DATA SIZES
  // ------------------------------------------
  //
  // Holding/Input Registers:
  //   125 registers × 2 bytes = 250 bytes each
  //
  // Coils/Discrete Inputs:
  //   stored as packed bits, not bool[125]
  //
  // Therefore:
  //
  //   125 coils            = 16 bytes
  //   125 discrete inputs  = 16 bytes
  //
  // This is important for AVR SRAM.
  // ------------------------------------------

  const uint16_t NUM_HOLDING_REGISTERS = 125;

  const uint16_t NUM_COILS = 125;

  const uint16_t NUM_DISCRETE_INPUTS = 125;

  const uint16_t NUM_INPUT_REGISTERS = 125;


  // ------------------------------------------
  // MODBUS DATA
  // ------------------------------------------

  uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];

  uint8_t coils[(NUM_COILS + 7) / 8];

  uint8_t discreteInputs[(NUM_DISCRETE_INPUTS + 7) / 8];

  uint16_t inputRegisters[NUM_INPUT_REGISTERS];


  // ------------------------------------------
  // COMMUNICATION
  // ------------------------------------------

  ModbusRTU *modbusComm = nullptr;

  ModbusSlaveLogic modbusLogic;


  // ==========================================
  // BEGIN
  // ==========================================

  void begin() {

    requestCount = 0;
    responseCount = 0;
    lastRequestId = 0xFF;
    lastFunctionCode = 0;

    // ----------------------------------------
    // TEST HOLDING REGISTERS
    // ----------------------------------------

    for (uint16_t i = 0;
         i < NUM_HOLDING_REGISTERS;
         i++) {

      holdingRegisters[i] = (i + 1) * 100;
    }


    // ----------------------------------------
    // TEST COILS
    // ----------------------------------------
    //
    // Packed representation:
    //
    // bit 0 = coil 0
    // bit 1 = coil 1
    // ... 
    // bit 7 = coil 7
    //
    // 0xAA = 10101010
    //
    // Therefore:
    //
    // coil 0 = OFF
    // coil 1 = ON
    // coil 2 = OFF
    // coil 3 = ON
    // coil 4 = OFF
    // coil 5 = ON
    // coil 6 = OFF
    // coil 7 = ON
    //
    // All remaining coils are OFF.
    // ----------------------------------------

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


    // ----------------------------------------
    // TEST DISCRETE INPUTS
    // ----------------------------------------
    //
    // 0x55 = 01010101
    //
    // Therefore:
    //
    // input 0 = ON
    // input 1 = OFF
    // input 2 = ON
    // input 3 = OFF
    // input 4 = ON
    // input 5 = OFF
    // input 6 = ON
    // input 7 = OFF
    //
    // All remaining inputs are OFF.
    // ----------------------------------------

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


    // ----------------------------------------
    // TEST INPUT REGISTERS
    // ----------------------------------------

    for (uint16_t i = 0;
         i < NUM_INPUT_REGISTERS;
         i++) {

      inputRegisters[i] = 1000 + i;
    }


    // ----------------------------------------
    // GET RS-232
    // ----------------------------------------

    SoftwareSerial &rs232 =
      SerialConsole::getSerial();


    // ----------------------------------------
    // MODBUS COMMUNICATION
    // ----------------------------------------

    modbusComm =
      new ModbusRTU(rs232);


    // ----------------------------------------
    // CONFIGURE HOLDING REGISTERS
    // ----------------------------------------

    modbusLogic.configureHoldingRegisters(
      holdingRegisters,
      NUM_HOLDING_REGISTERS
    );


    // ----------------------------------------
    // CONFIGURE COILS
    // ----------------------------------------

    modbusLogic.configureCoils(
      coils,
      NUM_COILS
    );


    // ----------------------------------------
    // CONFIGURE DISCRETE INPUTS
    // ----------------------------------------

    modbusLogic.configureDiscreteInputs(
      discreteInputs,
      NUM_DISCRETE_INPUTS
    );


    // ----------------------------------------
    // CONFIGURE INPUT REGISTERS
    // ----------------------------------------

    modbusLogic.configureInputRegisters(
      inputRegisters,
      NUM_INPUT_REGISTERS
    );


    // ----------------------------------------
    // START MODBUS
    // ----------------------------------------

    modbusComm->begin(
      MODBUS_BAUD,
      SERIAL_8N1
    );
  }


  // ==========================================
  // STATISTICS
  // ==========================================

  void getDisplayData(uint8_t &slaveId, uint8_t &requestId,
                      uint8_t &functionCode,
                      uint32_t &requests, uint32_t &responses) {
    slaveId = SLAVE_ID;
    requestId = lastRequestId;
    functionCode = lastFunctionCode;
    requests = requestCount;
    responses = responseCount;
  }


  // ==========================================
  // UPDATE
  // ==========================================

  void update() {

    if (modbusComm == nullptr) {
      return;
    }


    // ----------------------------------------
    // RECEIVE FRAME
    // ----------------------------------------

    ModbusADU adu;

    ModbusRTUError error =
      modbusComm->readAdu(adu);


    // ----------------------------------------
    // NO FRAME
    // ----------------------------------------

    if (error == MODBUS_RTU_TIMEOUT) {
      return;
    }


    // ----------------------------------------
    // FRAME RECEIVED
    // ----------------------------------------

    Serial.println();
    Serial.println(F("FRAME RECEIVED"));


    Serial.print(F("Length: "));
    Serial.println(adu.getRtuLen());


    Serial.print(F("ID: "));
    Serial.println(adu.getUnitId());


    Serial.print(F("Function: "));
    Serial.println(adu.getFunctionCode());


    Serial.print(F("CRC: "));

    if (adu.crcGood()) {
      Serial.println(F("OK"));
    }
    else {
      Serial.println(F("BAD"));
    }


    // ----------------------------------------
    // RAW FRAME
    // ----------------------------------------

    if (adu.getRtuLen() > 0) {

      Serial.print(F("RAW: "));

      for (uint16_t i = 0;
           i < adu.getRtuLen();
           i++) {

        if (adu.rtu[i] < 0x10) {
          Serial.print(F("0"));
        }

        Serial.print(
          adu.rtu[i],
          HEX
        );

        Serial.print(F(" "));
      }

      Serial.println();
    }


    // ----------------------------------------
    // COMMUNICATION ERROR
    // ----------------------------------------

    if (error != MODBUS_RTU_SUCCESS) {

      Serial.print(F("ERROR CODE: "));
      Serial.println(error);

      Serial.print(F("CRC CHECK: "));
      Serial.println(adu.crcGood() ? F("OK") : F("BAD"));

      Indicators::error();

      return;
    }


    // ----------------------------------------
    // CRC ERROR
    // ----------------------------------------

    if (!adu.crcGood()) {

      Serial.println(F("CRC ERROR"));

      Indicators::error();

      return;
    }


    // Keep the latest valid frame visible, including requests for other IDs.
    // The displayed Master ID is the destination address carried in the request.
    lastRequestId = adu.getUnitId();
    lastFunctionCode = adu.getFunctionCode();

    Indicators::rx();


    // ----------------------------------------
    // SLAVE ID
    // ----------------------------------------

    if (adu.getUnitId() != SLAVE_ID) {

      Serial.println(F("FOREIGN SLAVE ID"));

      return;
    }

    // Count only valid requests addressed to this Slave.
    // Broadcast (ID 0), foreign IDs and bad CRC frames are excluded.
    requestCount++;


    // ----------------------------------------
    // PROCESS REQUEST
    // ----------------------------------------

    modbusLogic.processPdu(adu);


    // ----------------------------------------
    // SHOW RESPONSE
    // ----------------------------------------

    Serial.println();

    adu.updateCrc();

    Serial.println(F("RESPONSE PREPARED"));


    Serial.print(F("Length: "));
    Serial.println(adu.getRtuLen());


    Serial.print(F("ID: "));
    Serial.println(adu.getUnitId());


    Serial.print(F("Function: "));
    Serial.println(adu.getFunctionCode());


    Serial.print(F("CRC: "));

    if (adu.crcGood()) {
      Serial.println(F("OK"));
    }
    else {
      Serial.println(F("BAD"));
    }


    // ----------------------------------------
    // RAW RESPONSE
    // ----------------------------------------

    if (adu.getRtuLen() > 0) {

      Serial.print(F("RAW: "));

      for (uint16_t i = 0;
           i < adu.getRtuLen();
           i++) {

        if (adu.rtu[i] < 0x10) {
          Serial.print(F("0"));
        }

        Serial.print(
          adu.rtu[i],
          HEX
        );

        Serial.print(F(" "));
      }

      Serial.println();
    }


    // ----------------------------------------
    // TRANSMIT RESPONSE
    // ----------------------------------------

    Serial.println(F("TRANSMITTING"));

    if (modbusComm->writeAdu(adu)) {

      responseCount++;

      Serial.println(F("RESPONSE SENT"));

      Indicators::tx();
    }
    else {

      Serial.println(F("RESPONSE FAILED"));

      Indicators::error();
    }
  }

}
