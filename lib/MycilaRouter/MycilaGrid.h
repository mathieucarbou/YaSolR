// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaExpiringValue.h>

#include <optional>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  class Grid {
    public:
      enum class Source {
        JSY,
        JSY_REMOTE,
        MQTT,
        PZEM,
        VICTRON,
      };

      typedef struct {
          float apparentPower = NAN;
          float current = NAN;
          uint32_t energy = 0;
          uint32_t energyReturned = 0;
          float frequency = NAN;
          float power = NAN;
          float powerFactor = NAN;
          float voltage = NAN;
      } Metrics;

      // sources

      void deleteMetrics(Source source) {
        switch (source) {
          case Source::JSY:
            delete _jsyMetrics;
            _jsyMetrics = nullptr;
            break;
          case Source::JSY_REMOTE:
            delete _jsyRemoteMetrics;
            _jsyRemoteMetrics = nullptr;
            break;
          case Source::MQTT:
            delete _mqttMetrics;
            _mqttMetrics = nullptr;
            break;
          case Source::PZEM:
            delete _pzemMetrics;
            _pzemMetrics = nullptr;
            break;
          case Source::VICTRON:
            delete _victronMetrics;
            _victronMetrics = nullptr;
            break;
          default:
            throw std::runtime_error("Unknown Grid Source");
        }
      }

      ExpiringValue<Metrics>& metrics(Source source) {
        switch (source) {
          case Source::JSY: {
            if (_jsyMetrics == nullptr) {
              _jsyMetrics = new ExpiringValue<Metrics>();
            }
            return *_jsyMetrics;
          }
          case Source::JSY_REMOTE: {
            if (_jsyRemoteMetrics == nullptr) {
              _jsyRemoteMetrics = new ExpiringValue<Metrics>();
            }
            return *_jsyRemoteMetrics;
          }
          case Source::MQTT: {
            if (_mqttMetrics == nullptr) {
              _mqttMetrics = new ExpiringValue<Metrics>();
            }
            return *_mqttMetrics;
          }
          case Source::PZEM: {
            if (_pzemMetrics == nullptr) {
              _pzemMetrics = new ExpiringValue<Metrics>();
            }
            return *_pzemMetrics;
          }
          case Source::VICTRON: {
            if (_victronMetrics == nullptr) {
              _victronMetrics = new ExpiringValue<Metrics>();
            }
            return *_victronMetrics;
          }
          default:
            throw std::runtime_error("Unknown Grid Source");
        }
      }

      bool isConnected() const { return getVoltage().has_value(); }

      bool isUsing(Source source) const {
        switch (source) {
          case Source::MQTT:
            return _mqttMetrics != nullptr && _mqttMetrics->isPresent() && !std::isnan(_mqttMetrics->get().power);
          case Source::VICTRON:
            return _victronMetrics != nullptr && _victronMetrics->isPresent() && !std::isnan(_victronMetrics->get().power);
          case Source::JSY_REMOTE:
            return _jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent() && !std::isnan(_jsyRemoteMetrics->get().power);
          case Source::JSY:
            return _jsyMetrics != nullptr && _jsyMetrics->isPresent() && !std::isnan(_jsyMetrics->get().power);
          default:
            // Note: PZEM is no a valid grid source since not bidirectional
            return false;
        }
      }

      std::optional<float> getPower() const {
        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent() && !std::isnan(_mqttMetrics->get().power)) {
          return _mqttMetrics->get().power;
        }
        if (_victronMetrics != nullptr && _victronMetrics->isPresent() && !std::isnan(_victronMetrics->get().power)) {
          return _victronMetrics->get().power;
        }
        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent() && !std::isnan(_jsyRemoteMetrics->get().power)) {
          return _jsyRemoteMetrics->get().power;
        }
        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent() && !std::isnan(_jsyMetrics->get().power)) {
          return _jsyMetrics->get().power;
        }
        return std::nullopt;
      }

      std::optional<float> getVoltage() const {
        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent() && _mqttMetrics->get().voltage > 0) {
          return _mqttMetrics->get().voltage;
        }
        if (_victronMetrics != nullptr && _victronMetrics->isPresent() && _victronMetrics->get().voltage > 0) {
          return _victronMetrics->get().voltage;
        }
        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent() && _jsyRemoteMetrics->get().voltage > 0) {
          return _jsyRemoteMetrics->get().voltage;
        }
        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent() && _jsyMetrics->get().voltage > 0) {
          return _jsyMetrics->get().voltage;
        }
        if (_pzemMetrics != nullptr && _pzemMetrics->isPresent() && _pzemMetrics->get().voltage > 0) {
          return _pzemMetrics->get().voltage;
        }
        return std::nullopt;
      }

      std::optional<float> getFrequency() const {
        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent() && _mqttMetrics->get().frequency > 0) {
          return _mqttMetrics->get().frequency;
        }
        if (_victronMetrics != nullptr && _victronMetrics->isPresent() && _victronMetrics->get().frequency > 0) {
          return _victronMetrics->get().frequency;
        }
        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent() && _jsyRemoteMetrics->get().frequency > 0) {
          return _jsyRemoteMetrics->get().frequency;
        }
        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent() && _jsyMetrics->get().frequency > 0) {
          return _jsyMetrics->get().frequency;
        }
        if (_pzemMetrics != nullptr && _pzemMetrics->isPresent() && _pzemMetrics->get().frequency > 0) {
          return _pzemMetrics->get().frequency;
        }
        return std::nullopt;
      }

      // get the current grid measurements
      // returns false if no measurements are available
      // - if MQTT is connected, it has priority
      // - if JSY remote is connected, it has second priority
      // - if JSY is connected, it has lowest priority
      bool readMeasurements(Metrics& metrics) const {
        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent()) {
          memcpy(&metrics, &_mqttMetrics->get(), sizeof(Metrics));
          return true;
        }
        if (_victronMetrics != nullptr && _victronMetrics->isPresent()) {
          memcpy(&metrics, &_victronMetrics->get(), sizeof(Metrics));
          return true;
        }
        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent()) {
          memcpy(&metrics, &_jsyRemoteMetrics->get(), sizeof(Metrics));
          return true;
        }
        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent()) {
          memcpy(&metrics, &_jsyMetrics->get(), sizeof(Metrics));
          return true;
        }
        return false;
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["online"] = isConnected();

        std::optional<float> power = getPower();
        if (power.has_value()) {
          root["power"] = power.value();
        }

        std::optional<float> voltage = getVoltage();
        if (voltage.has_value()) {
          root["voltage"] = voltage.value();
        }

        std::optional<float> frequency = getFrequency();
        if (frequency.has_value()) {
          root["frequency"] = frequency.value();
        }

        Metrics* measurements = new Metrics();
        readMeasurements(*measurements);
        toJson(root["measurements"].to<JsonObject>(), *measurements);
        delete measurements;
        measurements = nullptr;

        JsonObject mqtt = root["source"]["mqtt"].to<JsonObject>();
        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent()) {
          mqtt["enabled"] = true;
          mqtt["time"] = _mqttMetrics->getLastUpdateTime();
          toJson(mqtt, _mqttMetrics->get());
        } else {
          mqtt["enabled"] = false;
        }

        JsonObject victron = root["source"]["victron"].to<JsonObject>();
        if (_victronMetrics != nullptr && _victronMetrics->isPresent()) {
          victron["enabled"] = true;
          victron["time"] = _victronMetrics->getLastUpdateTime();
          toJson(victron, _victronMetrics->get());
        } else {
          victron["enabled"] = false;
        }

        JsonObject jsyRemote = root["source"]["jsy_remote"].to<JsonObject>();
        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent()) {
          jsyRemote["enabled"] = true;
          jsyRemote["time"] = _jsyRemoteMetrics->getLastUpdateTime();
          toJson(jsyRemote, _jsyRemoteMetrics->get());
        } else {
          jsyRemote["enabled"] = false;
        }

        JsonObject jsy = root["source"]["jsy"].to<JsonObject>();
        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent()) {
          jsy["enabled"] = true;
          jsy["time"] = _jsyMetrics->getLastUpdateTime();
          toJson(jsy, _jsyMetrics->get());
        } else {
          jsy["enabled"] = false;
        }

        JsonObject pzem = root["source"]["pzem"].to<JsonObject>();
        if (_pzemMetrics != nullptr && _pzemMetrics->isPresent()) {
          pzem["enabled"] = true;
          pzem["time"] = _pzemMetrics->getLastUpdateTime();
          toJson(pzem, _pzemMetrics->get());
        } else {
          pzem["enabled"] = false;
        }
      }

      static void toJson(const JsonObject& dest, const Metrics& metrics) {
        if (!std::isnan(metrics.apparentPower))
          dest["apparent_power"] = metrics.apparentPower;
        if (!std::isnan(metrics.current))
          dest["current"] = metrics.current;
        dest["energy"] = metrics.energy;
        dest["energy_returned"] = metrics.energyReturned;
        if (!std::isnan(metrics.frequency))
          dest["frequency"] = metrics.frequency;
        if (!std::isnan(metrics.power))
          dest["power"] = metrics.power;
        if (!std::isnan(metrics.powerFactor))
          dest["power_factor"] = metrics.powerFactor;
        if (!std::isnan(metrics.voltage))
          dest["voltage"] = metrics.voltage;
      }
#endif

    private:
      ExpiringValue<Metrics>* _jsyMetrics = nullptr;
      ExpiringValue<Metrics>* _jsyRemoteMetrics = nullptr;
      ExpiringValue<Metrics>* _mqttMetrics = nullptr;
      ExpiringValue<Metrics>* _victronMetrics = nullptr;
      ExpiringValue<Metrics>* _pzemMetrics = nullptr;
  };
} // namespace Mycila
