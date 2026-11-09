#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "encoder.h"

namespace Display {
  const uint8_t CHANGE_SLAVE_ID = 0x01;
  const uint8_t CHANGE_BAUD = 0x02;
  const uint8_t RESET_TRAFFIC = 0x04;

  void begin();
  void activity();
  uint8_t handleEncoder(int8_t rotation, Encoder::ButtonEvent buttonEvent,
                        uint8_t currentSlaveId, uint8_t currentBaudIndex,
                        uint8_t &newSlaveId, uint8_t &newBaudIndex);
  void update(uint8_t slaveId, uint8_t masterId,
              uint8_t functionCode, uint32_t rx,
              uint32_t tx, uint32_t rateRequests,
              uint32_t errors, uint32_t missed,
              uint8_t baudIndex);
}

#endif
