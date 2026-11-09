#include "modbus_slave.h"
#include "display.h"
#include "rs232_port.h"
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

  byte configuredSlaveId = 1;


  // ------------------------------------------
  // TRAFFIC STATISTICS
  // ------------------------------------------

  uint32_t rxCount = 0;
  uint32_t txCount = 0;
  uint32_t rateRequestCount = 0;
  uint32_t errorCount = 0;
  uint32_t missedCount = 0;
  uint8_t lastRequestId = 0xFF;
  uint8_t lastFunctionCode = 0;

  // Validate the RTU length before the PDU handler accesses fields.
  // Fixed-size requests (FC01-06) are exactly 8 bytes including CRC.
  // FC15/16 carry a byte-count field at offset 6.
  bool hasValidRequestLength(ModbusADU &adu) {
    const uint16_t length = adu.getRtuLen();

    if (length < 4) {
      return false;
    }

    switch (adu.rtu[1]) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        return length == 8;

      case 15:
      case 16:
        return length >= 9 &&
               length == (uint16_t)(9 + adu.rtu[6]);

      default:
        // Unknown function codes are valid candidates for an
        // Illegal Function exception, provided the RTU frame
        // contains at least address, function and CRC.
        return true;
    }
  }


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

  void begin(uint8_t initialSlaveId, unsigned long baud) {

    configuredSlaveId = (initialSlaveId >= 1 && initialSlaveId <= 247) ? initialSlaveId : 1;

    rxCount = 0;
    txCount = 0;
    rateRequestCount = 0;
    errorCount = 0;
    missedCount = 0;
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

    RS232Port::begin(baud);
    SoftwareSerial &rs232 =
      RS232Port::getSerial();


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
      baud,
      SERIAL_8N1
    );
  }

  void setSlaveId(uint8_t newSlaveId) {
    if (newSlaveId >= 1 && newSlaveId <= 247) {
      configuredSlaveId = newSlaveId;
    }
  }

  void setBaud(unsigned long baud) {
    if (modbusComm == nullptr || baud == 0) {
      return;
    }

    RS232Port::begin(baud);
    modbusComm->begin(baud, SERIAL_8N1);
  }


  // ==========================================
  // STATISTICS
  // ==========================================

  void getDisplayData(uint8_t &slaveId, uint8_t &requestId,
                      uint8_t &functionCode,
                      uint32_t &rx, uint32_t &tx,
                      uint32_t &rateRequests,
                      uint32_t &errors, uint32_t &missed) {
    slaveId = configuredSlaveId;
    requestId = lastRequestId;
    functionCode = lastFunctionCode;
    rx = rxCount;
    tx = txCount;
    rateRequests = rateRequestCount;
    errors = errorCount;
    missed = missedCount;
  }


  // Reset traffic and response-rate counters. ERR and MISS are diagnostic
  // history and intentionally remain unchanged.
  void resetTrafficCounters() {
    rxCount = 0;
    txCount = 0;
    rateRequestCount = 0;
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

    // Count every completed receive event, including CRC and
    // RTU framing errors. A pure timeout is not an RX frame.
    rxCount++;

    // Any completed or malformed receive event wakes the OLED.
    Display::activity();


    // ----------------------------------------
    // FRAME RECEIVED
    // ----------------------------------------

    Serial.println();
    Serial.println(F("FRAME RECEIVED"));


    Serial.print(F("Length: "));
    Serial.println(adu.getRtuLen());


    Serial.print(F("ID: "));
    if (adu.getRtuLen() >= 1) {
      Serial.println(adu.getUnitId());
    }
    else {
      Serial.println(F("N/A"));
    }


    Serial.print(F("Function: "));
    if (adu.getRtuLen() >= 2) {
      Serial.println(adu.getFunctionCode());
    }
    else {
      Serial.println(F("N/A"));
    }


    Serial.print(F("CRC: "));

    if (error == MODBUS_RTU_FRAME_ERROR) {
      Serial.println(F("N/A"));
    }
    else if (adu.crcGood()) {
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
      if (error == MODBUS_RTU_CRC_ERROR) {
        Serial.println(F("BAD"));
      }
      else {
        Serial.println(F("N/A"));
      }

      errorCount++;
      Indicators::error();

      return;
    }


    // ----------------------------------------
    // CRC ERROR
    // ----------------------------------------

    if (!adu.crcGood()) {

      Serial.println(F("CRC ERROR"));

      errorCount++;
      Indicators::error();

      return;
    }


    // RATE denominator: CRC-valid frames addressed to this Slave.
    // Count before PDU-length validation so malformed local requests are
    // included as attempts that did not produce a response. Foreign IDs and
    // broadcast ID 0 are excluded.
    if (adu.getUnitId() == configuredSlaveId) {
      rateRequestCount++;
    }

    // Keep the latest valid frame visible, including requests for other IDs.
    // The displayed Master ID is the destination address carried in the request.
    lastRequestId = adu.getUnitId();
    lastFunctionCode = adu.getFunctionCode();

    Indicators::rx();


    // Validate the complete request before deciding whether this
    // Slave should answer it. Malformed foreign-address frames are ERR,
    // not MISS.
    if (!hasValidRequestLength(adu)) {
      Serial.println(F("INVALID PDU LENGTH"));
      errorCount++;
      Indicators::error();
      return;
    }


    // ----------------------------------------
    // SLAVE ID
    // ----------------------------------------

    if (adu.getUnitId() != configuredSlaveId) {

      Serial.println(F("FOREIGN SLAVE ID"));

      // Valid request intentionally ignored by this Slave.
      missedCount++;
      return;
    }


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

      txCount++;

      Serial.println(F("RESPONSE SENT"));

      Indicators::tx();
    }
    else {

      Serial.println(F("RESPONSE FAILED"));

      Indicators::error();
    }
  }

}
