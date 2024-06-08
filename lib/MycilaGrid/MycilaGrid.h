// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>

#include <MycilaJSY.h>
#include <MycilaMQTT.h>

namespace Mycila {
  class GridClass {
    public:
      void setJSY(JSY& jsy) { _jsy = &jsy; }
      void setMQTT(MQTT& mqtt) { _mqtt = &mqtt; }

      void setDefaultFrequency(uint8_t frequency) { _frequency = frequency; }
      void setDefaultVoltage(uint8_t voltage) { _voltage = voltage; }

      void setMQTTGridPowerTopic(const String& gridPowerMQTTTopic);
      void setMQTTGridVoltageTopic(const String& gridVoltageMQTTTopic);

      void toJson(const JsonObject& root) const;

      float getFrequency() const;
      float getActivePower() const;
      float getActiveEnergy() const;
      float getActiveEnergyReturned() const;
      float getPowerFactor() const;
      float getVoltage() const;

      bool isConnected() const;

    private:
      JSY* _jsy;

      // default nominal values
      uint8_t _frequency = 0;
      uint8_t _voltage = 0;

      // mqtt
      MQTT* _mqtt;
      // mqtt grid power
      float _mqttGridPower = 0;
      bool _readGridPowerFromMQTT = false;
      uint32_t _mqttGridPowerLastTime = 0;
      // mqtt grid voltage
      float _mqttGridVoltage = 0;
      bool _readGridVoltageFromMQTT = false;
      uint32_t _mqttGridVoltageLastTime = 0;
  };

  extern GridClass Grid;
} // namespace Mycila
