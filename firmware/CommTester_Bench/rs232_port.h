#ifndef RS232_PORT_H
#define RS232_PORT_H

#include <Arduino.h>
#include <SoftwareSerial.h>

namespace RS232Port {
  void begin(unsigned long baud);
  SoftwareSerial& getSerial();
}

#endif
