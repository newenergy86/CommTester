#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

namespace Display {
  void begin();
  void activity();
  void update(uint8_t slaveId, uint8_t masterId,
              uint8_t functionCode, uint32_t requests,
              uint32_t responses);
}

#endif
