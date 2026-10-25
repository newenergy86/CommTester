#include "serial_console.h"

#include <SoftwareSerial.h>

const byte RS232_RX = 10;
const byte RS232_TX = 11;
const unsigned long RS232_BAUD = 9600;

SoftwareSerial rs232(RS232_RX, RS232_TX);

String commandBuffer = "";
bool commandReady = false;

void SerialConsole::begin() {
  rs232.begin(RS232_BAUD);
  commandBuffer = "";
  commandReady = false;
}

void SerialConsole::update() {
  while (rs232.available()) {
    char c = rs232.read();

    Serial.print("RX char: [");
    Serial.print(c);
    Serial.println("]");

    if (c == '\r' || c == '\n') {
      if (commandBuffer.length() > 0) {
        commandReady = true;
      }
      continue;
    }

    if (commandBuffer.length() < 32) {
      commandBuffer += c;
    }
  }
}

bool SerialConsole::available() {
  return commandReady;
}

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

void SerialConsole::print(const String &text) {
  rs232.print(text);
}

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