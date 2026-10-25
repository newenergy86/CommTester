#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#include <Arduino.h>
#include <SoftwareSerial.h>

namespace SerialConsole {

  void begin();
  void update();

  bool available();
  String getCommand();

  void print(const String &text);
  void println(const String &text);

  bool rs232Available();
  byte rs232Read();
  void rs232Write(const byte *buffer, byte length);

  SoftwareSerial& getSerial();

}

#endif