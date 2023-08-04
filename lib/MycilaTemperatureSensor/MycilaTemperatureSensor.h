// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#ifndef MYCILA_TEMPERATURE_READ_INTERVAL
#define MYCILA_TEMPERATURE_READ_INTERVAL 10
#endif

namespace Mycila {
  typedef std::function<void(const char* name, float temperature)> TemperatureChangeCallback;

  class TemperatureSensorConfigClass {
    public:
      bool isEnabled(const char* name) const;
      int32_t getPin(const char* name) const;
  };

  class TemperatureSensor {
    public:
      explicit TemperatureSensor(const char* name) : name(name) {}
      ~TemperatureSensor() { end(); }

      void begin();
      void loop();
      void end();

      void listen(TemperatureChangeCallback callback) { _callback = callback; }

      float getTemperature() const { return _temperature; }
      bool isEnabled() const { return _enabled; }
      bool isValid() const { return millis() - _lastUpdate < 6 * MYCILA_TEMPERATURE_READ_INTERVAL * 1000; }
      u_int32_t getLastUpdate() const { return _lastUpdate; }
      String getTemperatureAsString() const;
      gpio_num_t getPin() const { return _pin; };

      void toJson(const JsonObject& root) const;

    public:
      const char* name;

    private:
      u_int32_t _lastRead = 0;
      OneWire _oneWire;
      DallasTemperature _dallas;
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
      float _temperature = 0;
      uint32_t _lastUpdate = 0;
      TemperatureChangeCallback _callback = nullptr;

    private:
      void _read();
  };

  extern TemperatureSensorConfigClass TemperatureSensorConfig;
} // namespace Mycila
