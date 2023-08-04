// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaDimmer.h>

#include <MycilaLogger.h>
#include <MycilaZeroCrossDetection.h>

#define TAG "DIMMER"

static const char* DimmerTypeNames[] = {
  "Robodyn / TRIAC-based",
  "Random SSR",
  "Zero-Cross SSR"};

void Mycila::Dimmer::begin() {
  if (_enabled)
    return;

  if (!DimmerConfig.isEnabled(name)) {
    Logger.warn(TAG, "Disable Dimmer '%s'", name);
    return;
  }

  int32_t pin = DimmerConfig.getPin(name);
  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    Logger.error(TAG, "Disable Dimmer '%s': Invalid pin: %u", name, _pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  Logger.info(TAG, "Enable Dimmer '%s'...", name);
  Logger.debug(TAG, "- Pin: %u", _pin);
  Logger.debug(TAG, "- Type: %s", DimmerTypeNames[static_cast<int>(DimmerConfig.getType(name))]);

  _dimmer = new DimmableLightLinearized(_pin);
  _level = 0;
  _dimmer->setBrightness(0);
  _enabled = true;
}

void Mycila::Dimmer::end() {
  if (_enabled) {
    Logger.info(TAG, "Disable Dimmer '%s'", name);
    _enabled = false;
    _level = 0;
    _dimmer->setBrightness(0);
    delete _dimmer;
    _dimmer = nullptr;
    _pin = GPIO_NUM_NC;
  }
}

void Mycila::Dimmer::setLevel(uint8_t newLevel) {
  if (!_enabled)
    return;
  if (newLevel > 100)
    newLevel = 100;
  if (_level == newLevel)
    return;
  const uint8_t oldLevel = _level;
  _level = newLevel;
  _dimmer->setBrightness(map(_level, 0, 100, 0, 255));
  if (_callback && (oldLevel == 0 || oldLevel == 100 || newLevel == 0 || newLevel == 100))
    _callback(newLevel == 0 ? DimmerLevel::OFF : (newLevel == 100 ? DimmerLevel::FULL : DimmerLevel::DIM));
}

void Mycila::Dimmer::toJson(const JsonObject& root) const {
  root["enabled"] = _enabled;
  root["level"] = _level;
  root["state"] = _level > 0 ? "on" : "off";
}

namespace Mycila {
  DimmerConfigClass DimmerConfig;
} // namespace Mycila
