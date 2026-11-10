#include "indicators.h"
#include "rs232_port.h"
#include "modbus_slave.h"
#include "display.h"
#include "encoder.h"
#include "settings.h"

void setup() {
  Settings::begin();
  Indicators::begin();
  Display::begin();
  Encoder::begin();

  ModbusSlave::begin(Settings::getSlaveId(), Settings::getBaud());


}

void loop() {
  Indicators::update();
  Encoder::update();

  int8_t rotation = 0;
  Encoder::ButtonEvent buttonEvent = Encoder::BUTTON_NONE;
  const bool hasRotation = Encoder::takeRotation(rotation);
  const bool hasButtonEvent = Encoder::takeButtonEvent(buttonEvent);

  if (hasRotation || hasButtonEvent) {
    uint8_t newSlaveId = Settings::getSlaveId();
    uint8_t newBaudIndex = Settings::getBaudIndex();

    const uint8_t changes = Display::handleEncoder(
      hasRotation ? rotation : 0,
      hasButtonEvent ? buttonEvent : Encoder::BUTTON_NONE,
      Settings::getSlaveId(),
      Settings::getBaudIndex(),
      newSlaveId,
      newBaudIndex
    );

    if (changes & (Display::CHANGE_SLAVE_ID | Display::CHANGE_BAUD)) {
      Settings::save(newSlaveId, newBaudIndex);

      if (changes & Display::CHANGE_SLAVE_ID) {
        ModbusSlave::setSlaveId(newSlaveId);
      }
      if (changes & Display::CHANGE_BAUD) {
        ModbusSlave::setBaud(Settings::getBaud());
      }
    }

    if (changes & Display::RESET_TRAFFIC) {
      ModbusSlave::resetTrafficCounters();
    }
  }

  uint8_t slaveId = Settings::getSlaveId();
  uint8_t masterId = 0xFF;
  uint8_t functionCode = 0;
  uint32_t rx = 0;
  uint32_t tx = 0;
  uint32_t rateRequests = 0;
  uint32_t errors = 0;
  uint32_t missed = 0;

  ModbusSlave::getDisplayData(
    slaveId, masterId, functionCode, rx, tx, rateRequests, errors, missed
  );

  Display::update(
    slaveId, masterId, functionCode, rx, tx, rateRequests, errors, missed,
    Settings::getBaudIndex()
  );

  ModbusSlave::update();
}
