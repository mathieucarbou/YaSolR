// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaLogger.h>
#include <MycilaZeroCrossDetection.h>

#include <assert.h>
#include <thyristor.h>

#define TAG "ZCD"

#ifndef GPIO_IS_VALID_GPIO
#define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                      (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif


extern Mycila::Logger logger;

void Mycila::ZeroCrossDetectionClass::begin(const int8_t pin, const uint8_t frequency) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    logger.error(TAG, "Invalid Zero-Cross Detection pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  logger.info(TAG, "Enable Zero-Cross Detection on pin %" PRId8 " with frequency %" PRIu8 " Hz...", _pin, frequency);

#if defined(ESP32) && defined(FILTER_INT_PERIOD)
  // https://github.com/fabianoriccardi/dimmable-light/wiki/Notes-about-specific-architectures#interrupt-issue
  Thyristor::semiPeriodShrinkMargin = 400;
#endif
  Thyristor::setFrequency(frequency);
  Thyristor::frequencyMonitorAlwaysOn(true);
  Thyristor::setSyncPin(_pin);
  Thyristor::setSyncDir(RISING);
  Thyristor::begin();

  _enabled = true;
}

void Mycila::ZeroCrossDetectionClass::end() {
  if (_enabled) {
    logger.info(TAG, "Disable Zero-Cross Detection...");
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
