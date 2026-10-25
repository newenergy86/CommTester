#include "indicators.h"
#include "serial_console.h"
#include "modbus_slave.h"


// ============================================
// SETUP
// ============================================

void setup() {

  Serial.begin(115200);

  Indicators::begin();


  // ------------------------------------------
  // RS-232
  // ------------------------------------------

  SerialConsole::begin();

  ModbusSlave::begin();


  // ------------------------------------------
  // STARTUP MESSAGE
  // ------------------------------------------

  Serial.println();
  Serial.println("==============================");
  Serial.println(" CommTester Bench v0.4");
  Serial.println("==============================");
  Serial.println("RS-232 : 9600 8N1");
  Serial.println("Mode   : SERVICE");
  Serial.println("Hold button > 2 sec to change mode");
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


  // ------------------------------------------
  // SERVICE
  // ------------------------------------------

  if (mode == IND_SERVICE) {

    SerialConsole::update();


    // ----------------------------------------
    // COMMAND PROCESSING
    // ----------------------------------------

    if (SerialConsole::available()) {

      String command = SerialConsole::getCommand();

      Indicators::rx();


      // --------------------------------------
      // PING
      // --------------------------------------

      if (command == "PING") {

        SerialConsole::println("PONG");

        Indicators::tx();
        Indicators::beep(1);
      }


      // --------------------------------------
      // HELP
      // --------------------------------------

      else if (command == "HELP") {

        SerialConsole::println("Commands:");
        SerialConsole::println("PING");
        SerialConsole::println("INFO");
        SerialConsole::println("MODE");

        Indicators::tx();
      }


      // --------------------------------------
      // INFO
      // --------------------------------------

      else if (command == "INFO") {

        SerialConsole::println("CommTester Bench v0.4");
        SerialConsole::println("RS-232 9600 8N1");

        Indicators::tx();
      }


      // --------------------------------------
      // MODE
      // --------------------------------------

      else if (command == "MODE") {

        IndicatorMode currentMode =
          Indicators::getMode();


        if (currentMode == IND_SERVICE) {

          SerialConsole::println("MODE: SERVICE");
        }

        else if (currentMode == IND_SLAVE) {

          SerialConsole::println("MODE: SLAVE");
        }

        else if (currentMode == IND_MASTER) {

          SerialConsole::println("MODE: MASTER");
        }


        Indicators::tx();
      }


      // --------------------------------------
      // UNKNOWN COMMAND
      // --------------------------------------

      else {

        SerialConsole::println("Unknown command");

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
  // MASTER
  // ------------------------------------------

  else if (mode == IND_MASTER) {

    // Пока MASTER ещё не реализован.
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

    Serial.print("BUTTON EVENT, CURRENT MODE = ");


    if (currentMode == IND_SERVICE) {

      Serial.println("SERVICE");
    }

    else if (currentMode == IND_SLAVE) {

      Serial.println("SLAVE");
    }

    else if (currentMode == IND_MASTER) {

      Serial.println("MASTER");
    }

    else {

      Serial.println("UNKNOWN");
    }


    // ========================================
    // MODE SWITCH
    // ========================================

    switch (currentMode) {

      case IND_SERVICE:

        Indicators::setMode(IND_SLAVE);

        Serial.println("MODE -> SLAVE");

        break;


      case IND_SLAVE:

        Indicators::setMode(IND_MASTER);

        Serial.println("MODE -> MASTER");

        break;


      case IND_MASTER:

        Indicators::setMode(IND_SERVICE);

        Serial.println("MODE -> SERVICE");

        break;
    }
  }
}