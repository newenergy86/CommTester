#include "indicators.h"
#include "serial_console.h"
#include "modbus_slave.h"
#include "display.h"

#include <string.h>

// ============================================
// SETUP
// ============================================

void setup() {

  Serial.begin(115200);

  Indicators::begin();
  Display::begin();


  // ------------------------------------------
  // RS-232
  // ------------------------------------------

  SerialConsole::begin();

  ModbusSlave::begin();

  // ------------------------------------------
  // STARTUP MESSAGE
  // ------------------------------------------

  Serial.println();
  Serial.println(F("=============================="));
  Serial.println(F(" CommTester Bench v0.4"));
  Serial.println(F("=============================="));
  Serial.println(F("RS-232 : 9600 8N1"));
  Serial.println(F("Mode   : SERVICE"));
  Serial.println(F("Hold button > 2 sec to change mode"));
  Serial.println();

}


// ============================================
// LOOP
// ============================================

void loop() {


  // ------------------------------------------
  // INDICATORS
  // ------------------------------------------

  Indicators::update();


  // ------------------------------------------
  // CURRENT MODE
  // ------------------------------------------

  IndicatorMode mode = Indicators::getMode();

  uint8_t slaveId = 1;
  uint8_t masterId = 0xFF;
  uint8_t functionCode = 0;
  uint32_t requests = 0;
  uint32_t responses = 0;

  ModbusSlave::getDisplayData(
    slaveId, masterId, functionCode, requests, responses
  );

  Display::update(
    mode, slaveId, masterId, functionCode, requests, responses
  );


  // ------------------------------------------
  // SERVICE
  // ------------------------------------------

  if (mode == IND_SERVICE) {

    SerialConsole::update();


    // ----------------------------------------
    // COMMAND PROCESSING
    // ----------------------------------------

    if (SerialConsole::available()) {

      const char* command = SerialConsole::getCommand();

      Indicators::rx();


      // --------------------------------------
      // PING
      // --------------------------------------

      if (strcmp(command, "PING") == 0) {

        SerialConsole::println(F("PONG"));

        Indicators::tx();
      }


      // --------------------------------------
      // HELP
      // --------------------------------------

      else if (strcmp(command, "HELP") == 0) {

        SerialConsole::println(F("Commands:"));
        SerialConsole::println(F("PING"));
        SerialConsole::println(F("INFO"));
        SerialConsole::println(F("MODE"));

        Indicators::tx();
      }


      // --------------------------------------
      // INFO
      // --------------------------------------

      else if (strcmp(command, "INFO") == 0) {

        SerialConsole::println(F("CommTester Bench v0.4"));
        SerialConsole::println(F("RS-232 9600 8N1"));

        Indicators::tx();
      }


      // --------------------------------------
      // MODE
      // --------------------------------------

      else if (strcmp(command, "MODE") == 0) {

        IndicatorMode currentMode =
          Indicators::getMode();


        if (currentMode == IND_SERVICE) {

          SerialConsole::println(F("MODE: SERVICE"));
        }

        else if (currentMode == IND_SLAVE) {

          SerialConsole::println(F("MODE: SLAVE"));
        }

        Indicators::tx();
      }


      // --------------------------------------
      // UNKNOWN COMMAND
      // --------------------------------------

      else {

        SerialConsole::println(F("Unknown command"));

        Indicators::error();
        Indicators::beep(2);
      }
    }
  }


  // ------------------------------------------
  // SLAVE
  // ------------------------------------------

  else if (mode == IND_SLAVE) {

    ModbusSlave::update();
  }


  // ------------------------------------------
  // BUTTON
  // ------------------------------------------

  if (Indicators::buttonPressed()) {

    IndicatorMode currentMode =
      Indicators::getMode();


    // ========================================
    // DIAGNOSTICS
    // ========================================

    Serial.print(F("BUTTON EVENT, CURRENT MODE = "));


    if (currentMode == IND_SERVICE) {

      Serial.println(F("SERVICE"));
    }

    else if (currentMode == IND_SLAVE) {

      Serial.println(F("SLAVE"));
    }

    else {

      Serial.println(F("UNKNOWN"));
    }


    // ========================================
    // MODE SWITCH
    // ========================================

    switch (currentMode) {

      case IND_SERVICE:

        Indicators::setMode(IND_SLAVE);

        Serial.println(F("MODE -> SLAVE"));

        break;


      case IND_SLAVE:

        Indicators::setMode(IND_SERVICE);

        Serial.println(F("MODE -> SERVICE"));

        break;
    }
  }
}