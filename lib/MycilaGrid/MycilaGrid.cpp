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

void Mycila::GridClass::begin() {
  _topic = GridConfig.getGridPowerMQTTTopic();

  Logger.info(TAG, "Enable Grid Data...");
  Logger.debug(TAG, "- MQTT Grid Power Topic: %s", _topic.c_str());

  if (!_topic.isEmpty()) {
    MQTT.subscribe(_topic, [this](const String& topic, const String& payload) {
      _mqttGridPower = payload.toFloat();
      _mqttGridPowerUpdateTime = millis();
    });
  }
}

void Mycila::GridClass::end() {
  Logger.info(TAG, "Disable Grid Data...");
  if (!_topic.isEmpty()) {
    MQTT.unsubscribe(_topic);
    _mqttGridPower = 0;
    _mqttGridPowerUpdateTime = 0;
  }
}

float Mycila::GridClass::getFrequency() const {
  float freq = JSY.frequency;
  return freq > 0 ? freq : ZCD.getFrequency();
}

float Mycila::GridClass::getPower() const {
  return _topic.isEmpty() ? JSY.power2 : (_isMQTTGridPowerExpired() ? 0 : _mqttGridPower);
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
  GridConfigClass GridConfig;
} // namespace Mycila
