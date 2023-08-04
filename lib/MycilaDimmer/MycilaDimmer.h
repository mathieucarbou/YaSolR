// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <dimmable_light_linearized.h>
#include <esp32-hal-gpio.h>
#include <functional>

namespace Mycila {
  enum class DimmerType { TRIAC,
                          SSR_RANDOM,
                          SSR_ZC };

  enum class DimmerLevel { OFF,
                           FULL,
                           DIM };

  typedef std::function<void(DimmerLevel event)> DimmerLevelCallback;

  class DimmerConfigClass {
    public:
      bool isEnabled(const char* name) const;
      int32_t getPin(const char* name) const;
      DimmerType getType(const char* name) const;
  };

  class Dimmer {
    public:
      explicit Dimmer(const char* name) : name(name) {}

      void begin();
      void end();

      void listen(DimmerLevelCallback callback) { _callback = callback; }

      // level: 0-100 (%)
      void setLevel(uint8_t level);
      // level: 0-100 (%)
      uint8_t getLevel() const { return _level; }

      inline void off() { setLevel(0); }
      bool isOff() const { return _level == 0; }
      bool isOn() const { return _level > 0; }
      bool isOnAtFullPower() const { return _level >= 100; }

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _enabled; }

      void toJson(const JsonObject& root) const;

    public:
      const char* name;

    private:
      bool _enabled = false;
      uint8_t _level = 0;
      gpio_num_t _pin = GPIO_NUM_NC;
      DimmerLevelCallback _callback = nullptr;
      DimmableLightLinearized* _dimmer = nullptr;
  };

  extern DimmerConfigClass DimmerConfig;
} // namespace Mycila
