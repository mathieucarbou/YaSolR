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
      explicit RouterOutput(const char* name);

      void begin();
      void loop();
      void end();

      RouterOutputState getState() const;
      const char* getStateString() const;
      bool isEnabled() const { return dimmer.isEnabled(); }

      void listen(RouterOutputStateCallback callback) { _callback = callback; }

      bool isBypassRelayOn() const { return relay.isEnabled() ? relay.isOn() : dimmer.isOn(); }
      bool isBypassRelayEnabled() const { return relay.isEnabled() || dimmer.isEnabled(); }
      bool tryBypassRelayToggle() { return tryBypassRelayState(!isBypassRelayOn()); }
      bool tryBypassRelayState(bool state);
      // level: 0-100 (%)
      bool tryDimmerLevel(uint8_t level);

      void toJson(const JsonObject& root) const;

    public:
      const char* name;
      Dimmer dimmer;
      Relay relay;
      TemperatureSensor temperatureSensor;

    private:
      bool _autoBypassEnabled = false;
      RouterOutputStateCallback _callback = nullptr;
      u_int32_t _lastCheck = 0;

    private:
      void _autoBypassLoop();
      void _setBypassRelay(bool state);
  };

  extern RouterOutputConfigClass RouterOutputConfig;
} // namespace Mycila
