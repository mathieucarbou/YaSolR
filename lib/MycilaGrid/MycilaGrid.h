// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>

#ifndef YASOLR_MQTT_GRID_POWER_EXPIRATION
#define YASOLR_MQTT_GRID_POWER_EXPIRATION 60
#endif

namespace Mycila {
  class GridConfigClass {
    public:
      String getGridPowerMQTTTopic() const;
  };

  class GridClass {
    public:
      void begin();
      void end();

      void toJson(const JsonObject& root) const;

      float getFrequency() const;
      float getPower() const;
      float getVoltage() const;
      inline bool isOnline() const { return getFrequency() > 0; }

    private:
      volatile float _mqttGridPower = 0;
      volatile uint32_t _mqttGridPowerUpdateTime = 0;
      String _topic;

    private:
      bool _isMQTTGridPowerExpired() const { return millis() - _mqttGridPowerUpdateTime >= YASOLR_MQTT_GRID_POWER_EXPIRATION * 1000; }
  };

  extern GridClass Grid;
  extern GridConfigClass GridConfig;
} // namespace Mycila
