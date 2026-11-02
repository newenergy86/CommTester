#include "serial_console.h"

#include <SoftwareSerial.h>


// ============================================
// RS-232 PIN CONFIGURATION
// ============================================

// Arduino D10 ← MAX238 pin 6 (R1OUT)
// Arduino D11 → MAX238 pin 5 (T1IN)

const byte RS232_RX = 10;
const byte RS232_TX = 11;

const unsigned long RS232_BAUD = 9600;


// ============================================
// SOFTWARE SERIAL
// ============================================

SoftwareSerial rs232(RS232_RX, RS232_TX);


// ============================================
// COMMAND BUFFER
// ============================================

String commandBuffer = "";

bool commandReady = false;


// ============================================
// BEGIN
// ============================================

void SerialConsole::begin() {

  rs232.begin(RS232_BAUD);

  commandBuffer = "";
  commandReady = false;
}


// ============================================
// UPDATE
// ============================================

void SerialConsole::update() {

  while (rs232.available()) {

    char c = rs232.read();

    // Отладка через USB Serial
    Serial.print("RX char: [");
    Serial.print(c);
    Serial.println("]");


    // ----------------------------------------
    // CR / LF
    // ----------------------------------------

    if (c == '\r' || c == '\n') {

      if (commandBuffer.length() > 0) {

        commandReady = true;
      }

      continue;
    }


    // ----------------------------------------
    // NORMAL CHARACTER
    // ----------------------------------------

    if (commandBuffer.length() < 32) {

      commandBuffer += c;
    }
  }
}


// ============================================
// COMMAND AVAILABLE
// ============================================

bool SerialConsole::available() {

  return commandReady;
}


// ============================================
// GET COMMAND
// ============================================

String SerialConsole::getCommand() {

  if (!commandReady) {

    return "";
  }

  String result = commandBuffer;

  commandBuffer = "";
  commandReady = false;

  result.trim();
  result.toUpperCase();

  return result;
}


// ============================================
// PRINT
// ============================================

void SerialConsole::print(const String &text) {

  rs232.print(text);
}


// ============================================
// PRINTLN
// ============================================

void SerialConsole::println(const String &text) {

  rs232.println(text);
}

bool SerialConsole::rs232Available() {

  return rs232.available();
}


byte SerialConsole::rs232Read() {

  return rs232.read();
}


void SerialConsole::rs232Write(const byte *buffer, byte length) {

  rs232.write(buffer, length);
}

SoftwareSerial& SerialConsole::getSerial() {

  return rs232;
}