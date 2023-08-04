// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaBuzzer.h>
#include <MycilaLogger.h>

#define TAG "BUZZER"

void Mycila::BuzzerClass::begin(const uint8_t pin) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    Logger.error(TAG, "Invalid pin: %u", _pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  ledcSetup(0, 1000, 8);
  ledcAttachPin(_pin, 0);
  Logger.debug(TAG, "Enable Buzzer...");
  Logger.debug(TAG, "- Pin: %u", _pin);

  _enabled = true;
}

void Mycila::BuzzerClass::end() {
  if (_enabled) {
    Logger.debug(TAG, "Disable Buzzer...");
    _enabled = false;
    ledcDetachPin(_pin);
    digitalWrite(_pin, LOW);
    pinMode(_pin, INPUT);
    _pin = GPIO_NUM_NC;
  }
}

void Mycila::BuzzerClass::beep(uint8_t count, uint32_t duration) const {
  if (_enabled && count > 0)
    while (count-- > 0) {
      ledcWriteTone(0, 1000);
      delay(duration);
      ledcWriteTone(0, 0);
      if (count > 0)
        delay(duration + duration / 2);
    }
}

void Mycila::BuzzerClass::toJson(const JsonObject& root) const {
  root["enabled"] = _enabled;
}

namespace Mycila {
  BuzzerClass Buzzer;
} // namespace Mycila
