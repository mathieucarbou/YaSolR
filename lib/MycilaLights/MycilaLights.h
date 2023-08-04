// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <esp32-hal-gpio.h>

namespace Mycila {
  enum class LightState {
    NONE = 0,
    ON,
    OFF
  };

  class LightsClass {
    public:
      void begin(const uint8_t greenPin, const uint8_t yellowPin, const uint8_t redPin);
      void end();

      bool isEnabled() const { return _greenPin != GPIO_NUM_NC || _yellowPin != GPIO_NUM_NC || _redPin != GPIO_NUM_NC; }

      inline void setAllOn() { set(LightState::ON, LightState::ON, LightState::ON); }
      inline void setAllOff() { set(LightState::OFF, LightState::OFF, LightState::OFF); }

      inline void setGreen(bool state) { set(state ? LightState::ON : LightState::OFF, LightState::NONE, LightState::NONE); }
      inline void setYellow(bool state) { set(LightState::NONE, state ? LightState::ON : LightState::OFF, LightState::NONE); }
      inline void setRed(bool state) { set(LightState::NONE, LightState::NONE, state ? LightState::ON : LightState::OFF); }
      void set(LightState green, LightState yellow, LightState red);

      LightState getGreen() const { return _green ? LightState::ON : LightState::OFF; }
      LightState getYellow() const { return _yellow ? LightState::ON : LightState::OFF; }
      LightState getRed() const { return _red ? LightState::ON : LightState::OFF; }

      inline bool isGreenOn() const { return getGreen() == LightState::ON; }
      inline bool isYellowOn() const { return getYellow() == LightState::ON; }
      inline bool isRedOn() const { return getRed() == LightState::ON; }

      inline bool areAllOn() const { return isGreenOn() && isYellowOn() && isRedOn(); }
      inline bool areAllOff() const { return !isGreenOn() && !isYellowOn() && !isRedOn(); }

      inline String toString() const { return String(isGreenOn() ? "🟢 " : "⚫ ") + (isYellowOn() ? "🟡 " : "⚫ ") + (isRedOn() ? "🔴" : "⚫"); }

      void toJson(const JsonObject& root) const;

      gpio_num_t getGreenPin() const { return _greenPin; }
      gpio_num_t getYellowPin() const { return _yellowPin; }
      gpio_num_t getRedPin() const { return _redPin; }

    private:
      gpio_num_t _greenPin = GPIO_NUM_NC;
      gpio_num_t _yellowPin = GPIO_NUM_NC;
      gpio_num_t _redPin = GPIO_NUM_NC;
      bool _green = false;
      bool _yellow = false;
      bool _red = false;
  };

  extern LightsClass Lights;
} // namespace Mycila
