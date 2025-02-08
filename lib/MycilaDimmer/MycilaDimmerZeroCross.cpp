// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <MycilaDimmerZeroCross.h>

// lock
#include <freertos/FreeRTOS.h>

// gpio
#include <driver/gpio.h>
#include <esp32-hal-gpio.h>
#include <hal/gpio_ll.h>
#include <soc/gpio_struct.h>

// timers
#include <inlined_gptimer.h>

// logging
#include <esp32-hal-log.h>

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

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

#define TAG "ZC_DIMMER"

void Mycila::ZeroCrossDimmer::begin() {
  if (_enabled)
    return;

  if (!GPIO_IS_VALID_OUTPUT_GPIO(_pin)) {
    LOGE(TAG, "Disable ZC Dimmer: Invalid pin: %" PRId8, _pin);
    return;
  }

  LOGI(TAG, "Enable ZC Dimmer on pin %" PRId8 " with semi-period %" PRIu16 " us", _pin, _semiPeriod);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _dimmer = new Thyristor(_pin);
  _enabled = true;

  // restart with last saved value
  setDutyCycle(_dutyCycle);
}

void Mycila::ZeroCrossDimmer::end() {
  if (!_enabled)
    return;
  _enabled = false;
  LOGI(TAG, "Disable ZC Dimmer on pin %" PRId8, _pin);
  // Note: do not set _dutyCycle to 0 in order to keep last set user value
  _delay = UINT16_MAX;
  _dimmer->setDelay(_delay);
  delete _dimmer;
  _dimmer = nullptr;
  digitalWrite(_pin, LOW);
}

void ARDUINO_ISR_ATTR Mycila::ZeroCrossDimmer::onZeroCross(int16_t delayUntilZero, void* arg) {
  Thyristor::zero_cross_int(arg);
}
