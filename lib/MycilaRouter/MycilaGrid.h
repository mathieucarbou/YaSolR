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

      bool isConnected() const { return getVoltage() > 0; }
      float getEnergy() const { return _jsy->getEnergy2(); }
      float getEnergyReturned() const { return _jsy->getEnergyReturned2(); }
      float getCurrent() const { return _jsy->getCurrent2(); }
      float getApparentPower() const { return _jsy->getApparentPower2(); }
      float getPowerFactor() const { return _jsy->getPowerFactor2(); }
      float getVoltage() const { return _mqttVoltageTime ? _mqttVoltage : _jsy->getVoltage2(); }
      float getActivePower() const { return _mqttPowerTime ? _mqttPower : _jsy->getPower2(); }
      float getFrequency() const {
        // try first JSY
        float f = _jsy->getFrequency();
        // otherwise ZCD
        return f > 0 ? f : Thyristor::getDetectedFrequency();
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["apparent_power"] = getApparentPower();
        root["current"] = getCurrent();
        root["energy_returned"] = getEnergyReturned();
        root["energy"] = getEnergy();
        root["frequency"] = getFrequency();
        root["jsy_grid_power"] = _jsy->getPower2();
        root["jsy_grid_voltage"] = _jsy->getVoltage2();
        root["mqtt_grid_power"] = _mqttPower;
        root["mqtt_grid_voltage"] = _mqttVoltage;
        root["online"] = isConnected();
        root["power_factor"] = getPowerFactor();
        root["power"] = getActivePower();
        root["voltage"] = getVoltage();
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
