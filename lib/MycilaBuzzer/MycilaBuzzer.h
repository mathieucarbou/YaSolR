// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <esp32-hal-gpio.h>

namespace Mycila {
  class BuzzerClass {
    public:
      void begin(const uint8_t pin);
      void end();

      void toJson(const JsonObject& root) const;

      void beep(uint8_t count = 1, uint32_t duration = 100) const;
      bool isEnabled() const { return _enabled; }
      gpio_num_t getPin() const { return _pin; }

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
  };

  extern BuzzerClass Buzzer;
} // namespace Mycila
