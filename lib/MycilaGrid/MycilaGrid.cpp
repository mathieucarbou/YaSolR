// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaGrid.h>

#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaZeroCrossDetection.h>

#define TAG "POWER"

#define YASOLR_MQTT_GRID_EXPIRATION 60000

extern Mycila::Logger logger;

void Mycila::GridClass::setMQTTGridPowerTopic(const String& gridPowerMQTTTopic) {
  if (!gridPowerMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Grid Power from MQTT topic: %s", gridPowerMQTTTopic.c_str());
    _mqtt->subscribe(gridPowerMQTTTopic.c_str(), [this](const String& topic, const String& payload) {
      _mqttGridPower = payload.toFloat();
      _mqttGridPowerLastTime = millis();
      logger.debug(TAG, "MQTT Grid Power: %f", _mqttGridPower);
    });
    _readGridPowerFromMQTT = true;
  }
}

void Mycila::GridClass::setMQTTGridVoltageTopic(const String& gridVoltageMQTTTopic) {
  if (!gridVoltageMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Grid Voltage from MQTT topic: %s", gridVoltageMQTTTopic.c_str());
    _mqtt->subscribe(gridVoltageMQTTTopic.c_str(), [this](const String& topic, const String& payload) {
      _mqttGridVoltage = payload.toFloat();
      _mqttGridVoltageLastTime = millis();
      logger.debug(TAG, "MQTT Grid Voltage: %f", _mqttGridVoltage);
    });
    _readGridVoltageFromMQTT = true;
  }
}

float Mycila::GridClass::getFrequency() const {
  // 1. JSY
  float freq = _jsy == nullptr ? 0 : _jsy->getFrequency();
  if (freq > 0)
    return freq;

  // 2. ZCD
  freq = ZCD.getFrequency();

  // 3. default
  return freq > 0 ? freq : _frequency;
}

float Mycila::GridClass::getActivePower() const {
  // 1. MQTT
  if (_readGridPowerFromMQTT && millis() - _mqttGridPowerLastTime < YASOLR_MQTT_GRID_EXPIRATION)
    return _mqttGridPower;

  // 2. JSY
  if (_jsy != nullptr)
    return _jsy->getPower2();

  return 0;
}

float Mycila::GridClass::getActiveEnergy() const {
  return _jsy != nullptr ? _jsy->getEnergy2() : 0;
}

float Mycila::GridClass::getActiveEnergyReturned() const {
  return _jsy != nullptr ? _jsy->getEnergyReturned2() : 0;
}

float Mycila::GridClass::getPowerFactor() const {
  return _jsy != nullptr ? _jsy->getPowerFactor2() : 0;
}

float Mycila::GridClass::getVoltage() const {
  // 1. MQTT
  if (_readGridVoltageFromMQTT && millis() - _mqttGridVoltageLastTime < YASOLR_MQTT_GRID_EXPIRATION)
    return _mqttGridVoltage;

  // 2. JSY
  float volt = _jsy == nullptr ? 0 : _jsy->getVoltage2();

  // 3 .default
  return volt > 0 ? volt : _voltage;
}

bool Mycila::GridClass::isConnected() const {
  if (ZCD.getFrequency() > 0)
    return true;

  if (_jsy && _jsy->getFrequency() > 0)
    return true;

  return _readGridVoltageFromMQTT && millis() - _mqttGridVoltageLastTime < YASOLR_MQTT_GRID_EXPIRATION;
}

void Mycila::GridClass::toJson(const JsonObject& root) const {
  root["energy"] = getActiveEnergy();
  root["energy_returned"] = getActiveEnergyReturned();
  root["frequency"] = getFrequency();
  root["online"] = isConnected();
  root["power"] = getActivePower();
  root["power_factor"] = getPowerFactor();
  root["voltage"] = getVoltage();
}

namespace Mycila {
  GridClass Grid;
} // namespace Mycila
