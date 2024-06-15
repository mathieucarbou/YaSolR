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
      void begin(const int8_t pin, const uint8_t nominalFrequency);
      void end();

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _enabled; }
      bool isConnected() const { return getZCFrequency() > 0; }

      // in microseconds
      // 50Hz => 10000
      // 60Hz => 8333
      uint16_t getSemiPeriod() const;

      // Detect the Zero-Cross pulse frequency in Hz
      float getZCFrequency() const;

      // Grid frequency in Hz based on the Zero-Cross pulse frequency
      float getCurrentFrequency() const;

      // Detect the Zero-Cross pulse width in microseconds
      uint16_t getZCPulseWidthAvg() const;
      uint16_t getZCPulseWidthLast() const;
      uint16_t getZCPulseWidthMax() const;

      void toJson(const JsonObject& root) const {
        root["enabled"] = isEnabled();
        root["zc_pulse_freq"] = getZCFrequency();
        root["zc_pulse_width_avg"] = getZCPulseWidthAvg();
        root["zc_pulse_width_last"] = getZCPulseWidthLast();
        root["zc_pulse_width_max"] = getZCPulseWidthMax();
        root["semi_period"] = getSemiPeriod();
      }

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
  };
} // namespace Mycila
