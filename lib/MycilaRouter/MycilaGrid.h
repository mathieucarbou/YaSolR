// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaJSY.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
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
    SOURCE_JSY,
    SOURCE_MQTT
  } GridSource;

  class Grid {
    public:
      explicit Grid(JSY& jsy) : _jsy(&jsy) {}

      // MQTT support
      void setMQTTExpiration(uint32_t seconds) { _expiration = seconds * 1000; }
      void setMQTTGridVoltage(float voltage) {
        _mqttVoltage = voltage;
        _mqttVoltageTime = millis();
      }
      void setMQTTGridPower(float power) {
        _mqttPower = power;
        _mqttPowerTime = millis();
      }
      void applyExpiration() {
        if (_mqttVoltageTime > 0 && millis() - _mqttVoltageTime >= _expiration) {
          _mqttVoltage = 0;
          _mqttVoltageTime = 0;
        }
        if (_mqttPowerTime > 0 && millis() - _mqttPowerTime >= _expiration) {
          _mqttPower = 0;
          _mqttPowerTime = 0;
        }
      }

      // grid source logic:
      // for power: MQTT has priority
      // for voltage: JSY has priority
      GridSource getPowerSource() const {
        if (_mqttPowerTime)
          return SOURCE_MQTT;
        if (_jsy->isConnected() && _jsy->getVoltage2() > 0)
          return SOURCE_JSY;
        return SOURCE_NONE;
      }
      GridSource getVoltageSource() const {
        if (_jsy->isConnected() && _jsy->getVoltage2())
          return SOURCE_JSY;
        if (_mqttVoltageTime && _mqttVoltage)
          return SOURCE_MQTT;
        return SOURCE_NONE;
      }

      bool isConnected() const { return getVoltageSource() != GridSource::SOURCE_NONE; }

      float getPower() const {
        switch (getPowerSource()) {
          case SOURCE_JSY:
            return _jsy->getPower2();
          case SOURCE_MQTT:
            return _mqttPower;
          default:
            return 0;
            break;
        }
      }

      float getVoltage() const {
        switch (getVoltageSource()) {
          case SOURCE_JSY:
            return _jsy->getVoltage2();
          case SOURCE_MQTT:
            return _mqttVoltage;
          default:
            return 0;
            break;
        }
      }

      void getMetrics(GridMetrics& metrics) const {
        metrics.voltage = getVoltage();
        metrics.power = getPower();
        // only available if connected to JSY
        metrics.apparentPower = _jsy->getApparentPower2();
        metrics.current = _jsy->getCurrent2();
        metrics.energy = _jsy->getEnergy2();
        metrics.energyReturned = _jsy->getEnergyReturned2();
        metrics.frequency = _jsy->getFrequency();
        metrics.powerFactor = _jsy->getPowerFactor2();
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        GridMetrics metrics;
        getMetrics(metrics);
        root["online"] = isConnected();
        root["metrics"]["apparent_power"] = metrics.apparentPower;
        root["metrics"]["current"] = metrics.current;
        root["metrics"]["energy"] = metrics.energy;
        root["metrics"]["energy_returned"] = metrics.energyReturned;
        root["metrics"]["frequency"] = metrics.frequency;
        root["metrics"]["power"] = metrics.power;
        root["metrics"]["power_factor"] = metrics.powerFactor;
        root["metrics"]["voltage"] = metrics.voltage;
        root["mqtt"]["power"] = _mqttPower;
        root["mqtt"]["voltage"] = _mqttVoltage;
        root["source"]["power"] = getPowerSource() == SOURCE_JSY ? "jsy" : (getPowerSource() == SOURCE_MQTT ? "mqtt" : "none");
        root["source"]["voltage"] = getVoltageSource() == SOURCE_JSY ? "jsy" : (getVoltageSource() == SOURCE_MQTT ? "mqtt" : "none");
      }
#endif

    private:
      JSY* _jsy;

      // mqtt
      volatile float _mqttPower = 0;
      volatile float _mqttVoltage = 0;
      volatile uint32_t _mqttPowerTime = 0;
      volatile uint32_t _mqttVoltageTime = 0;
      uint32_t _expiration = 0;
  };
} // namespace Mycila
