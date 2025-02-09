// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <MycilaDimmerPWM.h>

#include <driver/ledc.h>

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

#define TAG "PWM_DIMMER"

void Mycila::PWMDimmer::begin() {
  if (_enabled)
    return;

  if (!GPIO_IS_VALID_OUTPUT_GPIO(_pin)) {
    LOGE(TAG, "Disable PWM Dimmer: Invalid pin: %" PRId8, _pin);
    return;
  }

  LOGI(TAG, "Enable PWM Dimmer on pin %" PRId8 " with semi-period %" PRIu16 " us", _pin, _semiPeriod);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);

  if (ledcAttach(_pin, _frequency, _resolution) && ledcWrite(_pin, 0)) {
    _enabled = true;
  } else {
    LOGE(TAG, "Failed to attach ledc driver on pin %" PRId8, _pin);
    return;
  }

  // restart with last saved value
  setDutyCycle(_dutyCycle);
}

void Mycila::PWMDimmer::end() {
  if (!_enabled)
    return;
  _enabled = false;
  LOGI(TAG, "Disable PWM Dimmer on pin %" PRId8, _pin);
  // Note: do not set _dutyCycle to 0 in order to keep last set user value
  _delay = UINT16_MAX;
  ledcDetach(_pin);
  digitalWrite(_pin, LOW);
}

bool Mycila::PWMDimmer::apply(float mappedDutyCycle) {
  uint32_t duty = mappedDutyCycle * ((1 << _resolution) - 1);
  // LOGD(TAG, "Set PWM duty cycle on pin %" PRId8 " to %lu", _pin, duty);
  return ledcWrite(_pin, duty);
}
