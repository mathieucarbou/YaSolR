// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <esp32-hal-gpio.h>

namespace Mycila {
  class ZeroCrossDetectionClass {
    public:
      void begin(const uint8_t pin, const uint8_t frequency = 50);
      void end();

      bool isEnabled() const { return _enabled; }
      gpio_num_t getPin() const { return _pin; }
      // if activated, the monitored frequency based on zero-cross detection. Can be close to 50 or 60 Hz or more depending on the ZCD module.
      float getFrequency();

      void toJson(const JsonObject& root);

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
  };

  extern ZeroCrossDetectionClass ZCD;
} // namespace Mycila
