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

      ExpiringValue<Metrics>& jsyMetrics() { return _jsyMetrics; }
      // For JSY Remote (UDP) or Victron
      ExpiringValue<Metrics>& remoteMetrics() { return _remoteMetrics; }
      ExpiringValue<Metrics>& pzemMetrics() { return _pzemMetrics; }
      ExpiringValue<float>& mqttPower() { return _mqttPower; }
      ExpiringValue<float>& mqttVoltage() { return _mqttVoltage; }

      bool isConnected() const { return getVoltage().has_value(); }

      // get the current grid power
      // - if MQTT is connected, it has priority
      // - if JSY remote is connected, it has second priority
      // - if JSY is connected, it has lowest priority
      std::optional<float> getPower() const;

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
      // - if MQTT is connected, it has priority
      // - if JSY remote is connected, it has second priority
      // - if JSY is connected, it has lowest priority
      bool readMeasurements(Metrics& metrics) const;

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const;
      static void toJson(const JsonObject& dest, const Metrics& metrics);
#endif

    private:
      ExpiringValue<Metrics> _jsyMetrics;
      ExpiringValue<Metrics> _remoteMetrics;
      ExpiringValue<Metrics> _pzemMetrics;
      ExpiringValue<float> _mqttPower;
      ExpiringValue<float> _mqttVoltage;
  };
} // namespace Mycila
