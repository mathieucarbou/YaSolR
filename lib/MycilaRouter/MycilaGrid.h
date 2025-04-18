// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaExpiringValue.h>

#include <optional>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  class Grid {
    public:
      typedef struct {
          float apparentPower = NAN;
          float current = NAN;
          uint32_t energy = 0;
          uint32_t energyReturned = 0;
          float frequency = NAN;
          float power = NAN;
          float powerFactor = NAN;
          float voltage = NAN;
      } Metrics;

      // sources

      ExpiringValue<Metrics>& localMetrics() { return _localMetrics; }
      ExpiringValue<Metrics>& remoteMetrics() { return _remoteMetrics; }
      ExpiringValue<Metrics>& pzemMetrics() { return _pzemMetrics; }
      ExpiringValue<float>& mqttPower() { return _mqttPower; }
      ExpiringValue<float>& mqttVoltage() { return _mqttVoltage; }

      // called after having updated the values from MQTT, JSY and JSY Remote
      // returns true if the power has been updated and routing must be updated too
      bool updatePower();

      bool isConnected() const { return getVoltage().has_value(); }

      // available power
      ExpiringValue<float>& power() { return _power; }
      std::optional<float> getPower() { return _power.opt(); }

      // get the current grid voltage
      // - if JSY are connected, they have priority
      // - if JSY remote is connected, it has second priority
      // - if PZEM is connected, it has third priority
      // - if MQTT is connected, it has lowest priority
      std::optional<float> getVoltage() const;

      // get the grid frequency
      // - if JSY are connected, they have priority
      // - if JSY remote is connected, it has second priority
      // - if PZEM is connected, it has third priority
      std::optional<float> getFrequency() const;

      // get the current grid measurements
      // returns false if no measurements are available
      bool getGridMeasurements(Metrics& metrics) const;

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const;
      static void toJson(const JsonObject& dest, const Metrics& metrics);
#endif

    private:
      ExpiringValue<Metrics> _localMetrics;
      ExpiringValue<Metrics> _remoteMetrics;
      ExpiringValue<Metrics> _pzemMetrics;
      ExpiringValue<float> _mqttPower;
      ExpiringValue<float> _mqttVoltage;
      ExpiringValue<float> _power;
  };
} // namespace Mycila
