// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaZCD.h>

#include <Arduino.h>
#include <assert.h>
#include <thyristor.h>

#ifdef MYCILA_LOGGER_SUPPORT
  #include <MycilaLogger.h>
extern Mycila::Logger logger;
  #define LOGD(tag, format, ...) logger.debug(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) logger.info(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) logger.warn(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) logger.error(tag, format, ##__VA_ARGS__)
#else
  #define LOGD(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

#define TAG "ZCD"

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

// Mycila::ZCD

void Mycila::ZCD::begin(const int8_t pin, const uint8_t frequency) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    LOGE(TAG, "Invalid Zero-Cross Detection pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  LOGI(TAG, "Enable Zero-Cross Detection on pin %" PRId8 " with frequency %" PRIu8 " Hz...", _pin, frequency);

  // https://github.com/fabianoriccardi/dimmable-light/wiki/Notes-about-specific-architectures#interrupt-issue
  Thyristor::semiPeriodShrinkMargin = 400;
  Thyristor::setFrequency(frequency);
  Thyristor::frequencyMonitorAlwaysOn(true);
  Thyristor::setSyncPin(_pin);
  Thyristor::setSyncDir(CHANGE);
  Thyristor::begin();

  // attachInterrupt(digitalPinToInterrupt(_pin), onEdge, CHANGE);

  _enabled = true;
}

void Mycila::ZCD::end() {
  if (_enabled) {
    LOGI(TAG, "Disable Zero-Cross Detection...");
    _enabled = false;
    Thyristor::setFrequency(0);
    _pin = GPIO_NUM_NC;
  }
}

uint16_t Mycila::ZCD::getSemiPeriod() const { return _enabled ? Thyristor::getSemiPeriod() : 0; }
float Mycila::ZCD::getFrequency() const { return getCurrentFrequency() * 2; }
float Mycila::ZCD::getCurrentFrequency() const { return _enabled ? Thyristor::getDetectedFrequency() : 0; }
uint16_t Mycila::ZCD::getAvgPulseWidth() const { return _enabled ? Thyristor::getPulseWidth() : 0; }
uint16_t Mycila::ZCD::getMaxPulseWidth() const { return _enabled ? Thyristor::getMaxPulseWidth() : 0; }
uint16_t Mycila::ZCD::getLastPulseWidth() const { return _enabled ? Thyristor::getLastPulseWidth() : 0; }
