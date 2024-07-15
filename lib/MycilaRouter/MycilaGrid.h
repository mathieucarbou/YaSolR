// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaJSY.h>

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

  typedef enum {
    SOURCE_NONE,
    SOURCE_METER,
    SOURCE_METER_REMOTE,
    SOURCE_MQTT
  } GridSource;

  class Grid {
    public:
      // expiration for remote data
      void setExpiration(uint32_t seconds) { _expiration = seconds * 1000; }

      void invalidate() {
        const uint32_t now = millis();

        if (_meterTime > 0 && now - _meterTime >= _expiration) {
          _meter.apparentPower = 0;
          _meter.current = 0;
          _meter.energy = 0;
          _meter.energyReturned = 0;
          _meter.frequency = 0;
          _meter.power = 0;
          _meter.powerFactor = 0;
          _meter.voltage = 0;
          _meterTime = 0;
          _meterConnected = false;
        }

        if (_meterRemoteTime > 0 && now - _meterRemoteTime >= _expiration) {
          _meterRemote.apparentPower = 0;
          _meterRemote.current = 0;
          _meterRemote.energy = 0;
          _meterRemote.energyReturned = 0;
          _meterRemote.frequency = 0;
          _meterRemote.power = 0;
          _meterRemote.powerFactor = 0;
          _meterRemote.voltage = 0;
          _meterRemoteTime = 0;
          _meterRemoteConnected = false;
        }

        if (_mqttVoltageTime > 0 && now - _mqttVoltageTime >= _expiration) {
          _mqtt.voltage = 0;
          _mqttVoltageTime = 0;
          _mqttConnected = false;
        }

        if (_mqttPowerTime > 0 && now - _mqttPowerTime >= _expiration) {
          _mqtt.power = 0;
          _mqttPowerTime = 0;
        }
      }

      // MQTT support
      void updateMQTTGridVoltage(float voltage) {
        _mqtt.voltage = voltage;
        _mqttVoltageTime = millis();
        _mqttConnected = voltage > 0;
      }

      // Update grid power from MQTT and returns true if the update is significant to trigger a routing update
      bool updateMQTTGridPower(float power) {
        _mqtt.power = power;
        _mqttPowerTime = millis();
        return getGridSource() == SOURCE_MQTT;
      }

      // JSY support for local wired JSY
      // Update data and returns true if the update is significant to trigger a routing update
      bool updateJSYData(const JSYData& data) {
        _meter.apparentPower = data.powerFactor2 == 0 ? 0 : data.power2 / data.powerFactor2;
        _meter.current = data.current2;
        _meter.energy = data.energy2;
        _meter.energyReturned = data.energyReturned2;
        _meter.frequency = data.frequency;
        _meter.power = data.power2;
        _meter.powerFactor = data.powerFactor2;
        _meter.voltage = data.voltage2;
        _meterTime = millis();
        _meterConnected = _meter.voltage > 0;

        if (!_meterConnected)
          return false;

        if (getGridSource() != SOURCE_METER)
          return false;

        bool updated = abs(_meter.power - _lastPower) > MYCILA_GRID_POWER_DELTA_FILTER;
        if (updated)
          _lastPower = _meter.power;

        return updated;
      }

      // JSY Remote support
      // Update data and returns true if the update is significant to trigger a routing update
      bool updateRemoteJSYData(const JSYData& data) {
        _meterRemote.apparentPower = data.powerFactor2 == 0 ? 0 : data.power2 / data.powerFactor2;
        _meterRemote.current = data.current2;
        _meterRemote.energy = data.energy2;
        _meterRemote.energyReturned = data.energyReturned2;
        _meterRemote.frequency = data.frequency;
        _meterRemote.power = data.power2;
        _meterRemote.powerFactor = data.powerFactor2;
        _meterRemote.voltage = data.voltage2;
        _meterRemoteTime = millis();
        _meterRemoteConnected = _meterRemote.voltage > 0;

        if (!_meterRemoteConnected)
          return false;

        if (getGridSource() != SOURCE_METER_REMOTE)
          return false;

        bool updated = abs(_meterRemote.power - _lastPower) > MYCILA_GRID_POWER_DELTA_FILTER;
        if (updated)
          _lastPower = _meterRemote.power;

        return updated;
      }

      bool isConnected() const { return _meterConnected || _meterRemoteConnected || _mqttConnected; }

      // get the current grid voltage
      // - if JSY are connected, they have priority
      // - if JSY remote is connected, it has second priority
      // - if MQTT is connected, it has lowest priority
      float getVoltage() const {
        if (_meterConnected)
          return _meter.voltage;
        if (_meterRemoteConnected)
          return _meterRemote.voltage;
        if (_mqttConnected)
          return _mqtt.voltage;
        return 0;
      }

      // get the current grid power
      // - if MQTT is connected, it has priority
      // - if JSY remote is connected, it has second priority
      // - if JSY is connected, it has lowest priority
      float getPower() const {
        if (_mqttPowerTime > 0)
          return _mqtt.power;
        if (_meterRemoteConnected)
          return _meterRemote.power;
        if (_meterConnected)
          return _meter.power;
        return 0;
      }

      // grid source logic:
      // - MQTT has higher priority
      // - JSY remote has second priority
      // - JSY local has lowest priority
      GridSource getGridSource() const {
        if (_mqttPowerTime > 0)
          return SOURCE_MQTT;
        if (_meterRemoteConnected)
          return SOURCE_METER_REMOTE;
        if (_meterConnected)
          return SOURCE_METER;
        return SOURCE_NONE;
      }

      void getMeasurements(GridMetrics& metrics) const {
        metrics.power = getPower();
        metrics.voltage = getVoltage();
        switch (getGridSource()) {
          case SOURCE_METER: {
            metrics.apparentPower = _meter.apparentPower;
            metrics.current = _meter.current;
            metrics.energy = _meter.energy;
            metrics.energyReturned = _meter.energyReturned;
            metrics.frequency = _meter.frequency;
            metrics.powerFactor = _meter.powerFactor;
            break;
          }
          case SOURCE_METER_REMOTE: {
            metrics.apparentPower = _meterRemote.apparentPower;
            metrics.current = _meterRemote.current;
            metrics.energy = _meterRemote.energy;
            metrics.energyReturned = _meterRemote.energyReturned;
            metrics.frequency = _meterRemote.frequency;
            metrics.powerFactor = _meterRemote.powerFactor;
            break;
          }
          default:
            break;
        }
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        GridMetrics metrics;
        getMeasurements(metrics);
        root["online"] = isConnected();
        toJson(root["metrics"].to<JsonObject>(), metrics);
        toJson(root["source"]["jsy"].to<JsonObject>(), _meter);
        toJson(root["source"]["jsy_remote"].to<JsonObject>(), _meterRemote);
        toJson(root["source"]["mqtt"].to<JsonObject>(), _mqtt);
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
      // Local JSY
      GridMetrics _meter;
      uint32_t _meterTime = 0;
      bool _meterConnected = false;

      // JSY remote data
      GridMetrics _meterRemote;
      uint32_t _meterRemoteTime = 0;
      bool _meterRemoteConnected = false;

      // mqtt
      GridMetrics _mqtt;
      uint32_t _mqttPowerTime = 0;
      uint32_t _mqttVoltageTime = 0;
      bool _mqttConnected = false;

      // expiration
      uint32_t _expiration = 0;

      // last power value
      float _lastPower = 0;
  };
} // namespace Mycila
