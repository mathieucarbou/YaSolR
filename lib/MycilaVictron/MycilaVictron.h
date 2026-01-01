// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#pragma once

#include <ModbusClientTCPasync.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <string>
#include <utility>

namespace Mycila {
  class Victron {
    public:
      enum class EventType {
        EVT_READ,
        EVT_ERROR,
      };

      typedef std::function<void(EventType eventType)> Callback;

      void setCallback(Callback callback) { _callback = std::move(callback); }

      void begin(const char* host, uint16_t port = 502);
      void end();

      /**
       * @brief Request to read the Victron metrics
       */
      void read();

      float getFrequency() const { return _frequency; }
      float getPower() const { return _power; }
      float getVoltage() const { return _voltage; }
      float getCurrent() const { return _current; }
      std::string getLastError() const { return _lastError; }
      bool hasError() const { return !_lastError.empty(); }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        if (!isnan(_current))
          root["current"] = _current;
        if (!isnan(_frequency))
          root["frequency"] = _frequency;
        if (!isnan(_power))
          root["power"] = _power;
        if (!isnan(_voltage))
          root["voltage"] = _voltage;
        if (_lastError.length())
          root["error"] = _lastError;
      }
#endif

    private:
      ModbusClientTCPasync* _client = nullptr;
      Callback _callback = nullptr;
      float _frequency = NAN;
      float _current = NAN;
      float _power = NAN;
      float _voltage = NAN;
      std::string _lastError;

      void _setError(ModbusError&& error, uint32_t token);
  };
} // namespace Mycila
