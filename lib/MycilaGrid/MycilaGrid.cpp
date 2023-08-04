// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaGrid.h>

#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaZeroCrossDetection.h>

#define TAG "POWER"

void Mycila::GridClass::setMQTTGridPowerTopic(const String& gridPowerMQTTTopic) {
  if (_topic != gridPowerMQTTTopic) {
    if (!_topic.isEmpty())
      MQTT.unsubscribe(_topic);
    _mqttGridPower = 0;
    _mqttGridPowerUpdateTime = 0;
    _topic = gridPowerMQTTTopic;
    if (!_topic.isEmpty()) {
      MQTT.subscribe(_topic, [this](const String& topic, const String& payload) {
        _mqttGridPower = payload.toFloat();
        _mqttGridPowerUpdateTime = millis();
      });
    }
  }
}

float Mycila::GridClass::getFrequency() const {
  float freq = _jsy == nullptr ? 0 : _jsy->getFrequency();
  return freq > 0 ? freq : ZCD.getFrequency();
}

float Mycila::GridClass::getPower() const {
  if (!_topic.isEmpty() && !isMQTTGridPowerExpired())
    return _mqttGridPower;
  if (_jsy != nullptr && _jsy->isEnabled())
    return _jsy->getPower2();
  return 0;
}

float Mycila::GridClass::getVoltage() const {
  return _jsy != nullptr && _jsy->isEnabled() ? _jsy->getVoltage2() : 0;
}

void Mycila::GridClass::toJson(const JsonObject& root) const {
  const uint8_t gridFrequency = getFrequency();
  root["frequency"] = gridFrequency;
  root["online"] = gridFrequency > 0;
  root["power"] = getPower();
  root["voltage"] = getVoltage();
  if (_jsy != nullptr)
    _jsy->toJson(root["jsy"].to<JsonObject>());
  Mycila::ZCD.toJson(root["zcd"].to<JsonObject>());
}

namespace Mycila {
  GridClass Grid;
} // namespace Mycila
