// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaGrid.h>

#include <MycilaJSY.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaZeroCrossDetection.h>

#define TAG "POWER"

void Mycila::GridClass::begin(const String& gridPowerMQTTTopic) {
  if (!_topic.isEmpty())
    return;

  _topic = gridPowerMQTTTopic;

  Logger.debug(TAG, "Enable MQTT Grid Power...");
  Logger.debug(TAG, "- MQTT Topic: %s", _topic.c_str());

  if (!_topic.isEmpty()) {
    MQTT.subscribe(_topic, [this](const String& topic, const String& payload) {
      _mqttGridPower = payload.toFloat();
      _mqttGridPowerUpdateTime = millis();
    });
  }
}

void Mycila::GridClass::end() {
  if (!_topic.isEmpty()) {
    Logger.debug(TAG, "Disable Grid Power from MQTT...");
    MQTT.unsubscribe(_topic);
    _topic = emptyString;
    _mqttGridPower = 0;
    _mqttGridPowerUpdateTime = 0;
  }
}

float Mycila::GridClass::getFrequency() const {
  float freq = JSY.frequency;
  return freq > 0 ? freq : ZCD.getFrequency();
}

float Mycila::GridClass::getPower() const {
  return !_topic.isEmpty() && !isMQTTGridPowerExpired() ? _mqttGridPower : (JSY.isEnabled() ? JSY.power2 : 0);
}

float Mycila::GridClass::getVoltage() const {
  return JSY.voltage2;
}

void Mycila::GridClass::toJson(const JsonObject& root) const {
  const uint8_t gridFrequency = getFrequency();
  root["frequency"] = gridFrequency;
  root["power"] = getPower();
  root["voltage"] = getVoltage();
  root["online"] = gridFrequency > 0;
  JsonObject jsy = root["jsy"].to<JsonObject>();
  Mycila::JSY.toJson(jsy);
  JsonObject zcd = root["zcd"].to<JsonObject>();
  Mycila::ZCD.toJson(zcd);
}

namespace Mycila {
  GridClass Grid;
} // namespace Mycila
