// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
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
      uint16_t getSemiPeriod() const;

      // Zero-Cross pulse information
      float getPulseFrequency() const;
      uint16_t getAvgPulseWidth() const;
      uint16_t getLastPulseWidth() const;
      uint16_t getMaxPulseWidth() const;
      uint16_t getMinPulseWidth() const;

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["enabled"] = isEnabled();
        root["pulse_freq"] = getPulseFrequency();
        root["pulse_width_avg"] = getAvgPulseWidth();
        root["pulse_width_last"] = getLastPulseWidth();
        root["pulse_width_max"] = getMaxPulseWidth();
        root["pulse_width_min"] = getMinPulseWidth();
        root["semi_period"] = getSemiPeriod();
      }
#endif

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
  };
} // namespace Mycila
