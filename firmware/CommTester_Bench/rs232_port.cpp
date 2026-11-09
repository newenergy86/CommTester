#include "rs232_port.h"

// Arduino D10 <- MAX238 R1OUT
// Arduino D11 -> MAX238 T1IN

namespace RS232Port {
  const byte RS232_RX = 10;
  const byte RS232_TX = 11;
  SoftwareSerial rs232(RS232_RX, RS232_TX);

  void begin(unsigned long baud) {
    rs232.begin(baud);
  }

  SoftwareSerial& getSerial() {
    return rs232;
  }
}
