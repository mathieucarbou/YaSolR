// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#pragma once

#include <ModbusClientTCPasync.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <memory>
#include <string>
#include <utility>

namespace Mycila {
  class Fronius {
    public:
      enum class EventType {
        EVT_READ,
        EVT_ERROR,
      };

      typedef std::function<void(EventType eventType)> Callback;

      Fronius() = default;
      ~Fronius() { end(); }

      // Non-copyable, non-movable: holds async callbacks bound to `this`.
      Fronius(const Fronius&) = delete;
      Fronius& operator=(const Fronius&) = delete;
      Fronius(Fronius&&) = delete;
      Fronius& operator=(Fronius&&) = delete;

      void setCallback(Callback callback) { _callback = std::move(callback); }

      void begin(const char* host, uint16_t port = 502);
      void end();

      /**
       * @brief Request to read the Fronius metrics
       */
      void read();

      float getFrequency() const { return _frequency; }
      float getPower() const { return _power; }
      float getVoltage() const { return _voltage; }
      float getCurrent() const { return _current; }
      std::string getLastError() const { return _lastError; }
      bool hasError() const { return !_lastError.empty(); }

      /**
       * @brief The Modbus device/slave ID of the meter, once auto-detection
       * has confirmed it by successfully parsing a valid meter model ID.
       * Returns 0 if not yet confirmed (0 is reserved for Modbus broadcast,
       * so no real device will ever report itself as a confirmed ID of 0).
       */
      uint8_t getMeterDeviceId() const { return _meterDeviceIdConfirmed ? _currentMeterDeviceId() : 0; }

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
      std::unique_ptr<ModbusClientTCPasync> _client;
      Callback _callback = nullptr;
      float _frequency = NAN;
      float _current = NAN;
      float _power = NAN;
      float _voltage = NAN;
      std::string _lastError;

      // Meter device/slave ID auto-detection state (200/240).
      // See FroniusMeterRegisters::CANDIDATE_DEVICE_IDS in the .cpp.
      size_t _meterDeviceIdIndex = 0;
      bool _meterDeviceIdConfirmed = false;
      uint8_t _currentMeterDeviceId() const;
      void _advanceMeterDeviceIdCandidate();

      void _setError(ModbusError&& error, uint32_t token);

      // Lifetime guard for async callbacks. Callbacks capture a weak_ptr to
      // this token instead of raw `this`; if end() runs (or the object is
      // destroyed) before a pending response/error fires, the weak_ptr
      // fails to lock and the callback becomes a safe no-op instead of
      // touching a destroyed/torn-down object.
      std::shared_ptr<bool> _aliveToken = std::make_shared<bool>(true);
  };
} // namespace Mycila
