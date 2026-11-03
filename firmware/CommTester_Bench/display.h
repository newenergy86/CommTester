#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "indicators.h"

namespace Display {

  void begin();
  void update(IndicatorMode mode, uint32_t requests, uint32_t responses);

}

#endif
