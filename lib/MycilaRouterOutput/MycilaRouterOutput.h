// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaRelay.h>
#include <MycilaTemperatureSensor.h>

#include <ArduinoJson.h>

namespace Mycila {
  enum class RouterOutputState {
    // output disabled
    OUTPUT_DISABLED = 0,
    // idle
    OUTPUT_IDLE,
    // excess power sent to load
    OUTPUT_ROUTING,
    // full power sent to load through relay (manual trigger)
    OUTPUT_BYPASS_MANUAL,
    // full power sent to load through relay (auto trigger)
    OUTPUT_BYPASS_AUTO
  };

  typedef std::function<void()> RouterOutputStateCallback;

  class RouterOutputConfigClass {
    public:
      bool isAutoDimmerEnabled(const char* name) const;
      uint8_t getDimmerLevelLimit(const char* name) const;
      bool isAutoBypassEnabled(const char* name) const;
      uint8_t getAutoStartTemperature(const char* name) const;
      uint8_t getAutoStopTemperature(const char* name) const;
      String getAutoStartTime(const char* name) const;
      String getAutoStopTime(const char* name) const;
      String getWeekDays(const char* name) const;
  };

  class RouterOutput {
    public:
      RouterOutput(const char* name, Dimmer* dimmer, TemperatureSensor* temperatureSensor, Relay* relay) : _name(name),
                                                                                                           _dimmer(dimmer),
                                                                                                           _temperatureSensor(temperatureSensor),
                                                                                                           _relay(relay) {}

      void autoBypass();
      void applyDimmerLimit();

      const char* getName() const { return _name; }
      RouterOutputState getState() const;
      const char* getStateString() const;
      bool isEnabled() const { return _dimmer->isEnabled(); }
      const Dimmer* getDimmer() const { return _dimmer; }
      const TemperatureSensor* getTemperatureSensor() const { return _temperatureSensor; }
      const Relay* getBypassRelay() const { return _relay; }

      void listen(RouterOutputStateCallback callback) { _callback = callback; }

      bool isBypassRelayOn() const { return _relay->isEnabled() ? _relay->isOn() : _dimmer->isOn(); }
      bool isBypassRelayEnabled() const { return _relay->isEnabled() || _dimmer->isEnabled(); }
      bool tryBypassRelayToggle() { return tryBypassRelayState(!isBypassRelayOn()); }
      bool tryBypassRelayState(bool state);
      // level: 0-100 (%)
      bool tryDimmerLevel(uint8_t level);

      void toJson(const JsonObject& root) const;

    private:
    // TODO: ⚠️ PZEM-004T per output
      const char* _name;
      Dimmer* _dimmer;
      const TemperatureSensor* _temperatureSensor;
      Relay* _relay;
      bool _autoBypassEnabled = false;
      RouterOutputStateCallback _callback = nullptr;

    private:
      void _setBypassRelay(bool state);
  };

  extern RouterOutputConfigClass RouterOutputConfig;
} // namespace Mycila
