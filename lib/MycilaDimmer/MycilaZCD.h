// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <Ticker.h>

#include <esp32-hal-gpio.h>

namespace Mycila {
  class ZCD {
    public:
      void begin(const int8_t pin, const uint8_t nominalFrequency);
      void end();

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _enabled; }
      bool isConnected() const { return _measuredGridFrequency > 0; }

      // in microseconds
      // 50Hz => 10000
      // 60Hz => 8333
      uint16_t getSemiPeriod() const;

      // Grid frequency in Hz based on the Zero-Cross pulse frequency
      float getCurrentFrequency() const { return _enabled ? _measuredGridFrequency : 0; }

      // Zero-Cross pulse information
      float getPulseFrequency() const { return _enabled ? _measuredGridFrequency * 2 : 0; }
      uint16_t getAvgPulseWidth() const;
      uint16_t getLastPulseWidth() const;
      uint16_t getMaxPulseWidth() const;
      uint16_t getMinPulseWidth() const;

      void toJson(const JsonObject& root) const {
        root["enabled"] = isEnabled();
        root["pulse_freq"] = getPulseFrequency();
        root["pulse_width_avg"] = getAvgPulseWidth();
        root["pulse_width_last"] = getLastPulseWidth();
        root["pulse_width_max"] = getMaxPulseWidth();
        root["pulse_width_min"] = getMinPulseWidth();
        root["semi_period"] = getSemiPeriod();
      }

    private:
      bool _enabled = false;
      volatile float _measuredGridFrequency = 0;
      Ticker _frequencyUpdater;
      gpio_num_t _pin = GPIO_NUM_NC;
  };
} // namespace Mycila
