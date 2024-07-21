// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaExpiringValue.h>

#include <optional>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

// - filter to not trigger routing task on small power changes
#ifndef MYCILA_GRID_POWER_DELTA_FILTER
  #define MYCILA_GRID_POWER_DELTA_FILTER 1
#endif

namespace Mycila {
  typedef struct {
      float apparentPower = 0;
      float current = 0;
      float energy = 0;
      float energyReturned = 0;
      float frequency = 0;
      float power = 0;
      float powerFactor = 0;
      float voltage = 0;
  } GridMetrics;

  class Grid {
    public:
      ExpiringValue<GridMetrics>& localGridMetrics() { return _localGridMetrics; }
      ExpiringValue<GridMetrics>& remoteGridMetrics() { return _remoteGridMetrics; }
      ExpiringValue<float>& mqttPower() { return _mqttPower; }
      ExpiringValue<float>& mqttVoltage() { return _mqttVoltage; }
      ExpiringValue<float>& pzemVoltage() { return _pzemVoltage; }

      bool isPowerUpdated() {
        float update = NAN;

        // try in order of priority
        if (_mqttPower.isPresent())
          update = _mqttPower.get();
        else if (_remoteGridMetrics.isPresent())
          update = _remoteGridMetrics.get().power;
        else if (_localGridMetrics.isPresent())
          update = _localGridMetrics.get().power;
        else
          update = NAN;

        // all became unavailable ?
        if (isnan(update) && !isnan(_lastPower)) {
          _lastPower = NAN;
          return true;
        }

        // one became available ?
        if (!isnan(update) && isnan(_lastPower)) {
          _lastPower = update;
          return true;
        }

        // update is significant ?
        if (abs(update - _lastPower) > MYCILA_GRID_POWER_DELTA_FILTER) {
          _lastPower = update;
          return true;
        }

        return false;
      }

      // get the current grid voltage
      // - if JSY are connected, they have priority
      // - if JSY remote is connected, it has second priority
      // - if PZEM is connected, it has third priority
      // - if MQTT is connected, it has lowest priority
      std::optional<float> getVoltage() const {
        if (_localGridMetrics.isPresent() && _localGridMetrics.get().voltage > 0)
          return _localGridMetrics.get().voltage;
        if (_remoteGridMetrics.isPresent() && _remoteGridMetrics.get().voltage > 0)
          return _remoteGridMetrics.get().voltage;
        if (_pzemVoltage.isPresent() && _pzemVoltage.get() > 0)
          return _pzemVoltage.get();
        if (_mqttVoltage.isPresent() && _mqttVoltage.get() > 0)
          return _mqttVoltage.get();
        return std::nullopt;
      }

      bool isConnected() const { return getVoltage().has_value(); }

      // get the current grid power
      // - if MQTT is connected, it has priority
      // - if JSY remote is connected, it has second priority
      // - if JSY is connected, it has lowest priority
      std::optional<float> getPower() const {
        if (_mqttPower.isPresent())
          return _mqttPower.get();
        if (_remoteGridMetrics.isPresent())
          return _remoteGridMetrics.get().power;
        if (_localGridMetrics.isPresent())
          return _localGridMetrics.get().power;
        return std::nullopt;
      }

      // get the current grid measurements
      // returns false if no measurements are available
      bool getMeasurements(GridMetrics& metrics) const {
        if (_mqttPower.isPresent()) {
          metrics.power = _mqttPower.get();
          metrics.voltage = getVoltage().value_or(0);
          return true;
        }

        if (_remoteGridMetrics.isPresent()) {
          metrics.apparentPower = _remoteGridMetrics.get().apparentPower;
          metrics.current = _remoteGridMetrics.get().current;
          metrics.energy = _remoteGridMetrics.get().energy;
          metrics.energyReturned = _remoteGridMetrics.get().energyReturned;
          metrics.frequency = _remoteGridMetrics.get().frequency;
          metrics.power = _remoteGridMetrics.get().power;
          metrics.powerFactor = _remoteGridMetrics.get().powerFactor;
          metrics.voltage = _remoteGridMetrics.get().voltage;
          return true;
        }

        if (_localGridMetrics.isPresent()) {
          metrics.apparentPower = _localGridMetrics.get().apparentPower;
          metrics.current = _localGridMetrics.get().current;
          metrics.energy = _localGridMetrics.get().energy;
          metrics.energyReturned = _localGridMetrics.get().energyReturned;
          metrics.frequency = _localGridMetrics.get().frequency;
          metrics.power = _localGridMetrics.get().power;
          metrics.powerFactor = _localGridMetrics.get().powerFactor;
          metrics.voltage = _localGridMetrics.get().voltage;
          return true;
        }

        return false;
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        GridMetrics measurements;
        getMeasurements(measurements);
        root["online"] = isConnected();
        toJson(root["measurements"].to<JsonObject>(), measurements);

        JsonObject local = root["source"]["local"].to<JsonObject>();
        if (_localGridMetrics.isPresent()) {
          local["enabled"] = true;
          local["expired"] = _localGridMetrics.isExpired();
          local["time"] = _localGridMetrics.getLastUpdateTime();
          toJson(local, _localGridMetrics.get());
        } else {
          local["enabled"] = false;
        }

        JsonObject remote = root["source"]["remote"].to<JsonObject>();
        if (_remoteGridMetrics.isPresent()) {
          remote["enabled"] = true;
          remote["expired"] = _remoteGridMetrics.isExpired();
          remote["time"] = _remoteGridMetrics.getLastUpdateTime();
          toJson(remote, _remoteGridMetrics.get());
        } else {
          remote["enabled"] = false;
        }

        JsonObject mqttPower = root["source"]["mqtt_power"].to<JsonObject>();
        if (_mqttPower.isPresent()) {
          mqttPower["enabled"] = true;
          mqttPower["expired"] = _mqttPower.isExpired();
          mqttPower["time"] = _mqttPower.getLastUpdateTime();
          mqttPower["value"] = _mqttPower.get();
        } else {
          mqttPower["enabled"] = false;
        }

        JsonObject mqttVoltage = root["source"]["mqtt_voltage"].to<JsonObject>();
        if (_mqttVoltage.isPresent()) {
          mqttVoltage["enabled"] = true;
          mqttVoltage["expired"] = _mqttVoltage.isExpired();
          mqttVoltage["time"] = _mqttVoltage.getLastUpdateTime();
          mqttVoltage["value"] = _mqttVoltage.get();
        } else {
          mqttVoltage["enabled"] = false;
        }
      }

      static void toJson(const JsonObject& dest, const GridMetrics& metrics) {
        dest["apparent_power"] = metrics.apparentPower;
        dest["current"] = metrics.current;
        dest["energy"] = metrics.energy;
        dest["energy_returned"] = metrics.energyReturned;
        dest["frequency"] = metrics.frequency;
        dest["power"] = metrics.power;
        dest["power_factor"] = metrics.powerFactor;
        dest["voltage"] = metrics.voltage;
      }
#endif

    private:
      ExpiringValue<GridMetrics> _localGridMetrics;
      ExpiringValue<GridMetrics> _remoteGridMetrics;
      ExpiringValue<float> _mqttPower;
      ExpiringValue<float> _mqttVoltage;
      ExpiringValue<float> _pzemVoltage;
      float _lastPower = NAN;
  };
} // namespace Mycila
