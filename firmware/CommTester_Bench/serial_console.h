#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#include <Arduino.h>
#include <SoftwareSerial.h>

namespace SerialConsole {

  void begin();
  void update();

  bool available();
  const char* getCommand();

  void print(const __FlashStringHelper *text);
  void println(const __FlashStringHelper *text);

  bool rs232Available();
  byte rs232Read();
  void rs232Write(const byte *buffer, byte length);

  SoftwareSerial& getSerial();

}

#endif
