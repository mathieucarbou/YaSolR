// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <esp32-hal-gpio.h>

namespace Mycila {
  class ZCD {
    public:
      void begin(const int8_t pin, const uint8_t frequency);
      void end();

      bool isEnabled() const { return _enabled; }
      gpio_num_t getPin() const { return _pin; }

      bool isConnected() const { return readFrequency() > 0; }
      float readFrequency() const;

      void toJson(const JsonObject& root) const {
        root["enabled"] = _enabled;
        root["frequency"] = readFrequency();
      }

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
  };
} // namespace Mycila
