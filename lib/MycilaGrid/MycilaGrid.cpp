// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaGrid.h>

#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaZeroCrossDetection.h>

#define TAG "POWER"

extern Mycila::Logger logger;

void Mycila::GridClass::setMQTTGridPowerTopic(const String& gridPowerMQTTTopic) {
  if (_mqttGridPowerTopic != gridPowerMQTTTopic) {
    if (!_mqttGridPowerTopic.isEmpty()) {
      logger.info(TAG, "Unsubscribing from MQTT topic: %s", _mqttGridPowerTopic.c_str());
      _mqtt->unsubscribe(_mqttGridPowerTopic);
    }
    _mqttGridPower = 0;
    _mqttGridDataUpdateTime = 0;
    _mqttGridPowerTopic = gridPowerMQTTTopic;
    if (!_mqttGridPowerTopic.isEmpty()) {
      logger.info(TAG, "Subscribing to MQTT topic: %s", _mqttGridPowerTopic.c_str());
      _mqtt->subscribe(_mqttGridPowerTopic, [this](const String& topic, const String& payload) {
        _mqttGridPower = payload.toFloat();
        _mqttGridDataUpdateTime = millis();
        logger.debug(TAG, "MQTT Grid Power: %f", _mqttGridPower);
      });
    }
  }
}

void Mycila::GridClass::setMQTTGridVoltageTopic(const String& gridVoltageMQTTTopic) {
  if (_mqttGridVoltageTopic != gridVoltageMQTTTopic) {
    if (!_mqttGridVoltageTopic.isEmpty()) {
      logger.info(TAG, "Unsubscribing from MQTT topic: %s", _mqttGridVoltageTopic.c_str());
      _mqtt->unsubscribe(_mqttGridVoltageTopic);
    }
    _mqttGridVoltage = 0;
    _mqttGridVoltageTopic = gridVoltageMQTTTopic;
    if (!_mqttGridVoltageTopic.isEmpty()) {
      logger.info(TAG, "Subscribing to MQTT topic: %s", _mqttGridVoltageTopic.c_str());
      _mqtt->subscribe(_mqttGridVoltageTopic, [this](const String& topic, const String& payload) {
        _mqttGridVoltage = payload.toFloat();
        _mqttGridDataUpdateTime = millis();
        logger.debug(TAG, "MQTT Grid Voltage: %f", _mqttGridVoltage);
      });
    }
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
  if (!_mqttGridPowerTopic.isEmpty() && !isMQTTGridDataExpired())
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
  if (!_mqttGridVoltageTopic.isEmpty() && !isMQTTGridDataExpired())
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

  return false;
}

void Mycila::GridClass::toJson(const JsonObject& root) const {
  root["energy_returned"] = getActiveEnergyReturned();
  root["energy"] = getActiveEnergy();
  root["frequency"] = getFrequency();
  root["online"] = isConnected();
  root["power"] = getActivePower();
  root["power_factor"] = getPowerFactor();
  root["voltage"] = getVoltage();
}

namespace Mycila {
  GridClass Grid;
} // namespace Mycila
