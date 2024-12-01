// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
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
  class Grid {
    public:
      typedef struct {
          float apparentPower = 0;
          float current = 0;
          float energy = 0;
          float energyReturned = 0;
          float frequency = 0;
          float power = 0;
          float powerFactor = 0;
          float voltage = 0;
      } Metrics;

      ExpiringValue<Metrics>& localMetrics() { return _localMetrics; }
      const ExpiringValue<Metrics>& localMetrics() const { return _localMetrics; }

      ExpiringValue<Metrics>& remoteMetrics() { return _remoteMetrics; }
      const ExpiringValue<Metrics>& remoteMetrics() const { return _remoteMetrics; }

      ExpiringValue<float>& mqttPower() { return _mqttPower; }
      const ExpiringValue<float>& mqttPower() const { return _mqttPower; }

      ExpiringValue<float>& mqttVoltage() { return _mqttVoltage; }
      const ExpiringValue<float>& mqttVoltage() const { return _mqttVoltage; }

      ExpiringValue<float>& pzemVoltage() { return _pzemVoltage; }
      const ExpiringValue<float>& pzemVoltage() const { return _pzemVoltage; }

      ExpiringValue<float>& power() { return _power; }
      const ExpiringValue<float>& power() const { return _power; }

      bool isConnected() const { return getVoltage().has_value(); }

      // called after having updated the values from MQTT, JSY and JSy Remote
      // returns true if the power has been updated and routing must be updated too
      bool updatePower() {
        float update = NAN;

        // try in order of priority
        // - if MQTT is connected, it has priority
        // - if JSY remote is connected, it has second priority
        // - if JSY is connected, it has lowest priority
        if (_mqttPower.isPresent()) {
          update = _mqttPower.get();
        } else if (_remoteMetrics.isPresent()) {
          update = _remoteMetrics.get().power;
        } else if (_localMetrics.isPresent()) {
          update = _localMetrics.get().power;
        }

        if (isnan(update) && _power.neverUpdated()) {
          return false;
        }

        // all became unavailable ?
        if (isnan(update)) {
          _power.reset();
          return true;
        }

        // one became available ?
        if (_power.neverUpdated()) {
          _power.update(update);
          return true;
        }

        // check if update is significant
        if (abs(update - _power.get()) > MYCILA_GRID_POWER_DELTA_FILTER) {
          _power.update(update);
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
        if (_localMetrics.isPresent() && _localMetrics.get().voltage > 0)
          return _localMetrics.get().voltage;
        if (_remoteMetrics.isPresent() && _remoteMetrics.get().voltage > 0)
          return _remoteMetrics.get().voltage;
        if (_pzemVoltage.isPresent() && _pzemVoltage.get() > 0)
          return _pzemVoltage.get();
        if (_mqttVoltage.isPresent() && _mqttVoltage.get() > 0)
          return _mqttVoltage.get();
        return std::nullopt;
      }

      // get the current grid measurements
      // returns false if no measurements are available
      bool getMeasurements(Metrics& metrics) const {
        if (_mqttPower.isPresent()) {
          metrics.power = _mqttPower.get();
          metrics.voltage = getVoltage().value_or(0);
          return true;
        }

        if (_remoteMetrics.isPresent()) {
          metrics.apparentPower = _remoteMetrics.get().apparentPower;
          metrics.current = _remoteMetrics.get().current;
          metrics.energy = _remoteMetrics.get().energy;
          metrics.energyReturned = _remoteMetrics.get().energyReturned;
          metrics.frequency = _remoteMetrics.get().frequency;
          metrics.power = _remoteMetrics.get().power;
          metrics.powerFactor = _remoteMetrics.get().powerFactor;
          metrics.voltage = _remoteMetrics.get().voltage;
          return true;
        }

        if (_localMetrics.isPresent()) {
          metrics.apparentPower = _localMetrics.get().apparentPower;
          metrics.current = _localMetrics.get().current;
          metrics.energy = _localMetrics.get().energy;
          metrics.energyReturned = _localMetrics.get().energyReturned;
          metrics.frequency = _localMetrics.get().frequency;
          metrics.power = _localMetrics.get().power;
          metrics.powerFactor = _localMetrics.get().powerFactor;
          metrics.voltage = _localMetrics.get().voltage;
          return true;
        }

        return false;
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["online"] = isConnected();

        if (_power.isPresent()) {
          root["power"] = _power.get();
        }

        std::optional<float> voltage = getVoltage();
        if (voltage.has_value()) {
          root["voltage"] = voltage.value();
        }

        Metrics measurements;
        getMeasurements(measurements);
        toJson(root["measurements"].to<JsonObject>(), measurements);

        JsonObject local = root["source"]["local"].to<JsonObject>();
        if (_localMetrics.isPresent()) {
          local["enabled"] = true;
          local["time"] = _localMetrics.getLastUpdateTime();
          toJson(local, _localMetrics.get());
        } else {
          local["enabled"] = false;
        }

        JsonObject remote = root["source"]["remote"].to<JsonObject>();
        if (_remoteMetrics.isPresent()) {
          remote["enabled"] = true;
          remote["time"] = _remoteMetrics.getLastUpdateTime();
          toJson(remote, _remoteMetrics.get());
        } else {
          remote["enabled"] = false;
        }

        JsonObject mqttPower = root["source"]["mqtt_power"].to<JsonObject>();
        if (_mqttPower.isPresent()) {
          mqttPower["enabled"] = true;
          mqttPower["time"] = _mqttPower.getLastUpdateTime();
          mqttPower["value"] = _mqttPower.get();
        } else {
          mqttPower["enabled"] = false;
        }

        JsonObject mqttVoltage = root["source"]["mqtt_voltage"].to<JsonObject>();
        if (_mqttVoltage.isPresent()) {
          mqttVoltage["enabled"] = true;
          mqttVoltage["time"] = _mqttVoltage.getLastUpdateTime();
          mqttVoltage["value"] = _mqttVoltage.get();
        } else {
          mqttVoltage["enabled"] = false;
        }
      }

      static void toJson(const JsonObject& dest, const Metrics& metrics) {
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
      ExpiringValue<Metrics> _localMetrics;
      ExpiringValue<Metrics> _remoteMetrics;
      ExpiringValue<float> _mqttPower;
      ExpiringValue<float> _mqttVoltage;
      ExpiringValue<float> _pzemVoltage;
      ExpiringValue<float> _power;
  };
} // namespace Mycila
