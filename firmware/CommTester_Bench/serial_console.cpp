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

// 32 characters + terminating null byte.
char commandBuffer[33];
byte commandLength = 0;

bool commandReady = false;


// ============================================
// BEGIN
// ============================================

void SerialConsole::begin() {

  rs232.begin(RS232_BAUD);

  commandLength = 0;
  commandBuffer[0] = '\0';
  commandReady = false;
}


// ============================================
// UPDATE
// ============================================

void SerialConsole::update() {

  // Do not overwrite a command before the caller consumes it.
  while (rs232.available() && !commandReady) {

    char c = rs232.read();

    // Отладка через USB Serial
    Serial.print(F("RX char: ["));
    Serial.print(c);
    Serial.println(F("]"));


    // ----------------------------------------
    // CR / LF
    // ----------------------------------------

    if (c == '\r' || c == '\n') {

      if (commandLength > 0) {
        commandBuffer[commandLength] = '\0';
        commandReady = true;
      }

      continue;
    }


    // ----------------------------------------
    // NORMAL CHARACTER
    // ----------------------------------------

    if (commandLength < 32) {
      commandBuffer[commandLength++] = c;
      commandBuffer[commandLength] = '\0';
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

const char* SerialConsole::getCommand() {

  if (!commandReady) {
    return "";
  }

  // Trim leading whitespace.
  byte start = 0;
  while (commandBuffer[start] == ' ' ||
         commandBuffer[start] == '\t' ||
         commandBuffer[start] == '\r' ||
         commandBuffer[start] == '\n') {
    start++;
  }

  // Trim trailing whitespace.
  byte end = commandLength;
  while (end > start &&
         (commandBuffer[end - 1] == ' ' ||
          commandBuffer[end - 1] == '\t' ||
          commandBuffer[end - 1] == '\r' ||
          commandBuffer[end - 1] == '\n')) {
    end--;
  }

  byte length = end - start;

  if (start > 0 && length > 0) {
    memmove(commandBuffer, commandBuffer + start, length);
  }

  commandBuffer[length] = '\0';

  // Convert ASCII lowercase to uppercase.
  for (byte i = 0; i < length; i++) {
    if (commandBuffer[i] >= 'a' && commandBuffer[i] <= 'z') {
      commandBuffer[i] -= ('a' - 'A');
    }
  }

  commandReady = false;
  commandLength = 0;

  return commandBuffer;
}


// ============================================
// PRINT
// ============================================

void SerialConsole::print(const __FlashStringHelper *text) {

  rs232.print(text);
}


// ============================================
// PRINTLN
// ============================================

void SerialConsole::println(const __FlashStringHelper *text) {

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
