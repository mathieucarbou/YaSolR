// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaJSY.h>
#include <thyristor.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  typedef struct {
      bool connected = false;
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

      bool isConnected() const { return (_jsy->isConnected() && _jsy->getVoltage2()) > 0 || (_mqttVoltageTime > 0 && _mqttVoltage > 0); }

      float getPower() const { return _mqttPowerTime ? _mqttPower : _jsy->getPower2(); }

      void getMetrics(GridMetrics& metrics) const {
        metrics.voltage = _jsy->isConnected() ? _jsy->getVoltage2() : (_mqttVoltageTime && _mqttVoltage ? _mqttVoltage : 0);
        metrics.apparentPower = _jsy->getApparentPower2();
        metrics.connected = metrics.voltage > 0;
        metrics.current = _jsy->getCurrent2();
        metrics.energy = _jsy->getEnergy2();
        metrics.energyReturned = _jsy->getEnergyReturned2();
        metrics.frequency = _jsy->isConnected() ? _jsy->getFrequency() : Thyristor::getDetectedFrequency();
        metrics.power = _mqttPowerTime ? _mqttPower : _jsy->getPower2();
        metrics.powerFactor = _jsy->getPowerFactor2();
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        GridMetrics metrics;
        getMetrics(metrics);
        root["online"] = metrics.connected;
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
