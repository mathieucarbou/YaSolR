// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaLogger.h>
#include <MycilaZeroCrossDetection.h>

#include <assert.h>
#include <thyristor.h>

#define TAG "ZCD"

void Mycila::ZeroCrossDetectionClass::begin(const uint8_t pin, const uint8_t frequency) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    Logger.error(TAG, "Invalid Zero-Cross Detection pin: %u", _pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  Logger.info(TAG, "Enable Zero-Cross Detection...");
  Logger.debug(TAG, "- Pin: %u", _pin);
  Logger.debug(TAG, "- Grid Frequency: %u Hz", frequency);

#if defined(ESP32) && defined(FILTER_INT_PERIOD)
  // https://github.com/fabianoriccardi/dimmable-light/wiki/Notes-about-specific-architectures#interrupt-issue
  Thyristor::semiPeriodShrinkMargin = 400;
#endif
  Thyristor::setFrequency(frequency);
  Thyristor::frequencyMonitorAlwaysOn(true);
  Thyristor::setSyncPin(_pin);
  Thyristor::begin();

  _enabled = true;
}

void Mycila::ZeroCrossDetectionClass::end() {
  if (_enabled) {
    Logger.info(TAG, "Disable Zero-Cross Detection...");
    _enabled = false;
    Thyristor::setFrequency(0);
    _pin = GPIO_NUM_NC;
  }
}

float Mycila::ZeroCrossDetectionClass::getFrequency() const { return _enabled ? Thyristor::getDetectedFrequency() : 0; }

void Mycila::ZeroCrossDetectionClass::toJson(const JsonObject& root) {
  root["enabled"] = _enabled;
  root["frequency"] = getFrequency();
}

namespace Mycila {
  ZeroCrossDetectionClass ZCD;
} // namespace Mycila
