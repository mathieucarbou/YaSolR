// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaExpiringValue.h>

#include <optional>
#include <utility>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  class Grid {
    public:
      enum class Source {
        UNKNOWN,
        JSY,
        JSY_REMOTE,
        MQTT,
        PZEM,
        VICTRON,
      };

      class Metrics {
        public:
          Source source = Source::UNKNOWN;
          float apparentPower = NAN;
          float current = NAN;
          uint32_t energy = 0;
          uint32_t energyReturned = 0;
          float frequency = NAN;
          float power = NAN;
          float powerFactor = NAN;
          float voltage = NAN;

          Metrics() = default;
          Metrics(Metrics&& other) noexcept {
            source = other.source;
            apparentPower = other.apparentPower;
            current = other.current;
            energy = other.energy;
            energyReturned = other.energyReturned;
            frequency = other.frequency;
            power = other.power;
            powerFactor = other.powerFactor;
            voltage = other.voltage;

            other.source = Source::UNKNOWN;
            other.apparentPower = NAN;
            other.current = NAN;
            other.energy = 0;
            other.energyReturned = 0;
            other.frequency = NAN;
            other.power = NAN;
            other.powerFactor = NAN;
            other.voltage = NAN;
          }
          Metrics& operator=(Metrics&& other) noexcept {
            if (this != &other) {
              source = other.source;
              apparentPower = other.apparentPower;
              current = other.current;
              energy = other.energy;
              energyReturned = other.energyReturned;
              frequency = other.frequency;
              power = other.power;
              powerFactor = other.powerFactor;
              voltage = other.voltage;

              other.source = Source::UNKNOWN;
              other.apparentPower = NAN;
              other.current = NAN;
              other.energy = 0;
              other.energyReturned = 0;
              other.frequency = NAN;
              other.power = NAN;
              other.powerFactor = NAN;
              other.voltage = NAN;
            }
            return *this;
          }
          Metrics(const Metrics& other) = delete;
          Metrics& operator=(const Metrics& other) = delete;
      };

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

      void updateMetrics(Metrics metrics) {
        switch (metrics.source) {
          case Source::JSY: {
            if (_jsyMetrics == nullptr) {
              _jsyMetrics = new ExpiringValue<Metrics>();
              _jsyMetrics->setExpiration(10000);
            }
            _jsyMetrics->update(std::move(metrics));
            break;
          }
          case Source::JSY_REMOTE: {
            if (_jsyRemoteMetrics == nullptr) {
              _jsyRemoteMetrics = new ExpiringValue<Metrics>();
              _jsyRemoteMetrics->setExpiration(10000);
            }
            _jsyRemoteMetrics->update(std::move(metrics));
            break;
          }
          case Source::MQTT: {
            if (_mqttMetrics == nullptr) {
              _mqttMetrics = new ExpiringValue<Metrics>();
              _mqttMetrics->setExpiration(60000);
            }
            _mqttMetrics->update(std::move(metrics));
            break;
          }
          case Source::PZEM: {
            if (_pzemMetrics == nullptr) {
              _pzemMetrics = new ExpiringValue<Metrics>();
              _pzemMetrics->setExpiration(10000);
            }
            _pzemMetrics->update(std::move(metrics));
            break;
          }
          case Source::VICTRON: {
            if (_victronMetrics == nullptr) {
              _victronMetrics = new ExpiringValue<Metrics>();
              _victronMetrics->setExpiration(10000);
            }
            _victronMetrics->update(std::move(metrics));
            break;
          }
          default:
            break;
        }
      }

      bool isConnected() const { return getVoltage().has_value(); }

      bool isUsing(Source source) const {
        return currentSource().has_value() && currentSource().value() == source;
      }

      std::optional<Source> currentSource() const {
        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent() && !std::isnan(_mqttMetrics->get().power)) {
          return Source::MQTT;
        }
        if (_victronMetrics != nullptr && _victronMetrics->isPresent() && !std::isnan(_victronMetrics->get().power)) {
          return Source::VICTRON;
        }
        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent() && !std::isnan(_jsyRemoteMetrics->get().power)) {
          return Source::JSY_REMOTE;
        }
        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent() && !std::isnan(_jsyMetrics->get().power)) {
          return Source::JSY;
        }
        return std::nullopt;
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

        {
          Metrics* measurements = new Metrics();
          readMeasurements(*measurements);
          toJson(root["measurements"].to<JsonObject>(), *measurements);
          delete measurements;
        }

        JsonArray sources = root["sources"].to<JsonArray>();

        if (_mqttMetrics != nullptr && _mqttMetrics->isPresent()) {
          JsonObject mqtt = sources.add<JsonObject>();
          toJson(mqtt, _mqttMetrics->get());
          mqtt["time"] = _mqttMetrics->getLastUpdateTime();
        }

        if (_victronMetrics != nullptr && _victronMetrics->isPresent()) {
          JsonObject victron = sources.add<JsonObject>();
          toJson(victron, _victronMetrics->get());
          victron["time"] = _victronMetrics->getLastUpdateTime();
        }

        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent()) {
          JsonObject jsyRemote = sources.add<JsonObject>();
          toJson(jsyRemote, _jsyRemoteMetrics->get());
          jsyRemote["time"] = _jsyRemoteMetrics->getLastUpdateTime();
        }

        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent()) {
          JsonObject jsy = sources.add<JsonObject>();
          toJson(jsy, _jsyMetrics->get());
          jsy["time"] = _jsyMetrics->getLastUpdateTime();
        }

        if (_pzemMetrics != nullptr && _pzemMetrics->isPresent()) {
          JsonObject pzem = sources.add<JsonObject>();
          toJson(pzem, _pzemMetrics->get());
          pzem["time"] = _pzemMetrics->getLastUpdateTime();
        }
      }

      static void toJson(const JsonObject& dest, const Metrics& metrics) {
        switch (metrics.source) {
          case Source::JSY:
            dest["source"] = "jsy";
            break;
          case Source::JSY_REMOTE:
            dest["source"] = "jsy_remote";
            break;
          case Source::MQTT:
            dest["source"] = "mqtt";
            break;
          case Source::PZEM:
            dest["source"] = "pzem";
            break;
          case Source::VICTRON:
            dest["source"] = "victron";
            break;
          default:
            dest["source"] = "unknown";
            break;
        }
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
