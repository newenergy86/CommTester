#include "settings.h"
#include <EEPROM.h>

namespace {
  const uint8_t EEPROM_MAGIC = 0xC7;
  const uint8_t EEPROM_VERSION = 1;
  const uint8_t DEFAULT_SLAVE_ID = 1;
  const uint8_t DEFAULT_BAUD_INDEX = 3;

  const unsigned long BAUD_RATES[] = {
    1200UL, 2400UL, 4800UL, 9600UL, 19200UL
  };

  uint8_t slaveId = DEFAULT_SLAVE_ID;
  uint8_t baudIndex = DEFAULT_BAUD_INDEX;

  uint8_t calculateChecksum(uint8_t id, uint8_t index) {
    return (uint8_t)(EEPROM_MAGIC ^ EEPROM_VERSION ^ id ^ index);
  }
}

void Settings::begin() {
  const uint8_t storedMagic = EEPROM.read(0);
  const uint8_t storedVersion = EEPROM.read(1);
  const uint8_t storedId = EEPROM.read(2);
  const uint8_t storedBaudIndex = EEPROM.read(3);
  const uint8_t storedChecksum = EEPROM.read(4);

  if (storedMagic == EEPROM_MAGIC &&
      storedVersion == EEPROM_VERSION &&
      storedId >= 1 && storedId <= 247 &&
      storedBaudIndex < (sizeof(BAUD_RATES) / sizeof(BAUD_RATES[0])) &&
      storedChecksum == calculateChecksum(storedId, storedBaudIndex)) {
    slaveId = storedId;
    baudIndex = storedBaudIndex;
  } else {
    slaveId = DEFAULT_SLAVE_ID;
    baudIndex = DEFAULT_BAUD_INDEX;
  }
}

uint8_t Settings::getSlaveId() {
  return slaveId;
}

uint8_t Settings::getBaudIndex() {
  return baudIndex;
}

unsigned long Settings::getBaud() {
  return BAUD_RATES[baudIndex];
}

void Settings::save(uint8_t newSlaveId, uint8_t newBaudIndex) {
  if (newSlaveId < 1 || newSlaveId > 247 ||
      newBaudIndex >= (sizeof(BAUD_RATES) / sizeof(BAUD_RATES[0]))) {
    return;
  }

  slaveId = newSlaveId;
  baudIndex = newBaudIndex;

  EEPROM.update(0, EEPROM_MAGIC);
  EEPROM.update(1, EEPROM_VERSION);
  EEPROM.update(2, slaveId);
  EEPROM.update(3, baudIndex);
  EEPROM.update(4, calculateChecksum(slaveId, baudIndex));
}
