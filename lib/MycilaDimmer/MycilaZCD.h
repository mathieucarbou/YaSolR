// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <esp32-hal-gpio.h>

namespace Mycila {
  class ZCD {
    public:
      void begin(const int8_t pin, const uint8_t nominalFrequency);
      void end();

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _enabled; }

      // in microseconds
      // 50Hz => 10000
      // 60Hz => 8333
      uint16_t getNominalSemiPeriod() const;

      // Zero-Cross pulse information
      uint32_t getPulsePeriod() const;

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["enabled"] = isEnabled();
        root["nominal_semiperiod_us"] = getNominalSemiPeriod();
        root["pulse_period_us"] = getPulsePeriod();
      }
#endif

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
  };
} // namespace Mycila
