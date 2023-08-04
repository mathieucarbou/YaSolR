// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#ifndef MYCILA_TEMPERATURE_EXPIRATION_DELAY
#define MYCILA_TEMPERATURE_EXPIRATION_DELAY 60
#endif

namespace Mycila {
  typedef std::function<void(float temperature)> TemperatureChangeCallback;
  class TemperatureSensor {
    public:
      ~TemperatureSensor() { end(); }

      void begin(const uint8_t pin, uint32_t expirationDelay = 0);
      void end();

      float read();

      void listen(TemperatureChangeCallback callback) { _callback = callback; }

      float getTemperature() const { return _temperature; }
      bool isEnabled() const { return _enabled; }
      bool isValid() const { return _expirationDelay == 0 ? _enabled : (millis() - _lastUpdate < _expirationDelay * 1000); }
      uint32_t getLastUpdate() const { return _lastUpdate; }
      uint32_t getExpirationDelay() const { return _expirationDelay; }

      String getTemperatureAsString() const;
      gpio_num_t getPin() const { return _pin; };

      void toJson(const JsonObject& root) const;

    private:
      OneWire _oneWire;
      DallasTemperature _dallas;
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
      float _temperature = 0;
      uint32_t _lastUpdate = 0;
      uint32_t _expirationDelay = 0;
      TemperatureChangeCallback _callback = nullptr;
  };
} // namespace Mycila
