// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>

#include <MycilaJSY.h>

#ifndef YASOLR_MQTT_GRID_POWER_EXPIRATION
#define YASOLR_MQTT_GRID_POWER_EXPIRATION 60
#endif

namespace Mycila {
  class GridClass {
    public:
      void setJSY(const JSY* jsy) { _jsy = jsy; }

      void setFrequency(uint8_t frequency) { _frequency = frequency; }
      void setVoltage(uint8_t voltage) { _voltage = voltage; }
      void setMQTTGridPowerTopic(const String& gridPowerMQTTTopic);
      void setMQTTGridVoltageTopic(const String& gridVoltageMQTTTopic);

      void toJson(const JsonObject& root) const;

      float getFrequency() const;
      float getActivePower() const;
      float getActiveEnergy() const;
      float getActiveEnergyReturned() const;
      float getPowerFactor() const;
      float getVoltage() const;

      bool isConnected() const { return getVoltage() > 0; }
      bool isMQTTGridDataExpired() const { return millis() - _mqttGridDataUpdateTime >= YASOLR_MQTT_GRID_POWER_EXPIRATION * 1000; }

    private:
      const JSY* _jsy;
      uint8_t _frequency = 0;
      uint8_t _voltage = 0;
      uint32_t _mqttGridDataUpdateTime = 0;
      float _mqttGridPower = 0;
      float _mqttGridVoltage = 0;
      String _mqttGridPowerTopic;
      String _mqttGridVoltageTopic;
  };

  extern GridClass Grid;
} // namespace Mycila
