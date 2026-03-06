// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
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
      enum class SourceKind {
        UNKNOWN,
        JSY,
        JSY_REMOTE,
        MQTT,
        VICTRON,
      };

      enum class Source {
        UNKNOWN,

        MQTT,
        VICTRON,

        JSY_MK_163_SERIAL1,
        JSY_MK_163_SERIAL2,
        JSY_MK_163_REMOTE,

        JSY_MK_227_SERIAL1,
        JSY_MK_227_SERIAL2,
        JSY_MK_227_REMOTE,

        JSY_MK_229_SERIAL1,
        JSY_MK_229_SERIAL2,
        JSY_MK_229_REMOTE,

        JSY_MK_193_CH1_SERIAL1,
        JSY_MK_193_CH1_SERIAL2,
        JSY_MK_193_CH1_REMOTE,

        JSY_MK_193_CH2_SERIAL1,
        JSY_MK_193_CH2_SERIAL2,
        JSY_MK_193_CH2_REMOTE,

        JSY_MK_194_CH1_SERIAL1,
        JSY_MK_194_CH1_SERIAL2,
        JSY_MK_194_CH1_REMOTE,

        JSY_MK_194_CH2_SERIAL1,
        JSY_MK_194_CH2_SERIAL2,
        JSY_MK_194_CH2_REMOTE,

        JSY_MK_333_SERIAL1,
        JSY_MK_333_SERIAL2,
        JSY_MK_333_REMOTE,
      };

      class Metrics {
        public:
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
            apparentPower = other.apparentPower;
            current = other.current;
            energy = other.energy;
            energyReturned = other.energyReturned;
            frequency = other.frequency;
            power = other.power;
            powerFactor = other.powerFactor;
            voltage = other.voltage;

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
              apparentPower = other.apparentPower;
              current = other.current;
              energy = other.energy;
              energyReturned = other.energyReturned;
              frequency = other.frequency;
              power = other.power;
              powerFactor = other.powerFactor;
              voltage = other.voltage;

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

      void setSource(Source source) { this->_source = source; }
      Source getSource() const { return _source; }

      void setSource(const char* src) { setSource(sourceFromString(src)); }
      const char* getSourceString() const { return sourceToString(_source); }

      SourceKind getSourceKind() { return sourceToKind(_source); }
      bool isUsing(SourceKind kind) const { return sourceToKind(_source) == kind; }

      bool isUsing(Source source) const { return _source == source; }

      void clearMetrics() { _metrics.reset(); }

      void updateMetrics(Metrics metrics) {
        if (_metrics.neverUpdated()) {
          _metrics.setExpiration(_source == Source::MQTT ? 60000 : 10000);
        }
        _metrics.update(std::move(metrics));
      }

      bool isConnected() const { return _metrics.isPresent() && _metrics.get().voltage > 0; }

      std::optional<float> getPower() const {
        if (_metrics.isPresent() && !std::isnan(_metrics.get().power)) {
          return _metrics.get().power;
        }
        return std::nullopt;
      }

      std::optional<float> getVoltage() const {
        if (_metrics.isPresent() && _metrics.get().voltage > 0) {
          return _metrics.get().voltage;
        }
        return std::nullopt;
      }

      std::optional<float> getFrequency() const {
        if (_metrics.isPresent() && _metrics.get().frequency > 0) {
          return _metrics.get().frequency;
        }
        return std::nullopt;
      }

      // get the current grid measurements
      bool readMeasurements(Metrics& metrics) const {
        if (_metrics.isPresent()) {
          memcpy(&metrics, &_metrics.get(), sizeof(Metrics));
          return true;
        }
        return false;
      }

      static SourceKind sourceToKind(Source source) {
        switch (source) {
          case Source::MQTT:                   return SourceKind::MQTT;
          case Source::VICTRON:                return SourceKind::VICTRON;
          case Source::JSY_MK_163_SERIAL1:     return SourceKind::JSY;
          case Source::JSY_MK_163_SERIAL2:     return SourceKind::JSY;
          case Source::JSY_MK_163_REMOTE:      return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_227_SERIAL1:     return SourceKind::JSY;
          case Source::JSY_MK_227_SERIAL2:     return SourceKind::JSY;
          case Source::JSY_MK_227_REMOTE:      return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_229_SERIAL1:     return SourceKind::JSY;
          case Source::JSY_MK_229_SERIAL2:     return SourceKind::JSY;
          case Source::JSY_MK_229_REMOTE:      return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_193_CH1_SERIAL1: return SourceKind::JSY;
          case Source::JSY_MK_193_CH1_SERIAL2: return SourceKind::JSY;
          case Source::JSY_MK_193_CH1_REMOTE:  return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_193_CH2_SERIAL1: return SourceKind::JSY;
          case Source::JSY_MK_193_CH2_SERIAL2: return SourceKind::JSY;
          case Source::JSY_MK_193_CH2_REMOTE:  return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_194_CH1_SERIAL1: return SourceKind::JSY;
          case Source::JSY_MK_194_CH1_SERIAL2: return SourceKind::JSY;
          case Source::JSY_MK_194_CH1_REMOTE:  return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_194_CH2_SERIAL1: return SourceKind::JSY;
          case Source::JSY_MK_194_CH2_SERIAL2: return SourceKind::JSY;
          case Source::JSY_MK_194_CH2_REMOTE:  return SourceKind::JSY_REMOTE;
          case Source::JSY_MK_333_SERIAL1:     return SourceKind::JSY;
          case Source::JSY_MK_333_SERIAL2:     return SourceKind::JSY;
          case Source::JSY_MK_333_REMOTE:      return SourceKind::JSY_REMOTE;
          default:                             return SourceKind::UNKNOWN;
        }
      }

      static const char* sourceToString(Source source) {
        switch (source) {
          case Source::MQTT:                   return "MQTT";
          case Source::VICTRON:                return "Victron";
          case Source::JSY_MK_163_SERIAL1:     return "JSY-MK-163 (Serial1)";
          case Source::JSY_MK_163_SERIAL2:     return "JSY-MK-163 (Serial2)";
          case Source::JSY_MK_163_REMOTE:      return "JSY-MK-163 (Remote)";
          case Source::JSY_MK_227_SERIAL1:     return "JSY-MK-227 (Serial1)";
          case Source::JSY_MK_227_SERIAL2:     return "JSY-MK-227 (Serial2)";
          case Source::JSY_MK_227_REMOTE:      return "JSY-MK-227 (Remote)";
          case Source::JSY_MK_229_SERIAL1:     return "JSY-MK-229 (Serial1)";
          case Source::JSY_MK_229_SERIAL2:     return "JSY-MK-229 (Serial2)";
          case Source::JSY_MK_229_REMOTE:      return "JSY-MK-229 (Remote)";
          case Source::JSY_MK_193_CH1_SERIAL1: return "JSY-MK-193 Channel 1 (Serial1)";
          case Source::JSY_MK_193_CH1_SERIAL2: return "JSY-MK-193 Channel 1 (Serial2)";
          case Source::JSY_MK_193_CH1_REMOTE:  return "JSY-MK-193 Channel 1 (Remote)";
          case Source::JSY_MK_193_CH2_SERIAL1: return "JSY-MK-193 Channel 2 (Serial1)";
          case Source::JSY_MK_193_CH2_SERIAL2: return "JSY-MK-193 Channel 2 (Serial2)";
          case Source::JSY_MK_193_CH2_REMOTE:  return "JSY-MK-193 Channel 2 (Remote)";
          case Source::JSY_MK_194_CH1_SERIAL1: return "JSY-MK-194 Channel 1 (Serial1)";
          case Source::JSY_MK_194_CH1_SERIAL2: return "JSY-MK-194 Channel 1 (Serial2)";
          case Source::JSY_MK_194_CH1_REMOTE:  return "JSY-MK-194 Channel 1 (Remote)";
          case Source::JSY_MK_194_CH2_SERIAL1: return "JSY-MK-194 Channel 2 (Serial1)";
          case Source::JSY_MK_194_CH2_SERIAL2: return "JSY-MK-194 Channel 2 (Serial2)";
          case Source::JSY_MK_194_CH2_REMOTE:  return "JSY-MK-194 Channel 2 (Remote)";
          case Source::JSY_MK_333_SERIAL1:     return "JSY-MK-333 (Serial1)";
          case Source::JSY_MK_333_SERIAL2:     return "JSY-MK-333 (Serial2)";
          case Source::JSY_MK_333_REMOTE:      return "JSY-MK-333 (Remote)";
          default:                             return "Unknown";
        }
      }

      static Source sourceFromString(const char* str) {
        if (strcmp(str, "MQTT") == 0) return Source::MQTT;
        if (strcmp(str, "Victron") == 0) return Source::VICTRON;
        if (strcmp(str, "JSY-MK-163 (Serial1)") == 0) return Source::JSY_MK_163_SERIAL1;
        if (strcmp(str, "JSY-MK-163 (Serial2)") == 0) return Source::JSY_MK_163_SERIAL2;
        if (strcmp(str, "JSY-MK-163 (Remote)") == 0) return Source::JSY_MK_163_REMOTE;
        if (strcmp(str, "JSY-MK-227 (Serial1)") == 0) return Source::JSY_MK_227_SERIAL1;
        if (strcmp(str, "JSY-MK-227 (Serial2)") == 0) return Source::JSY_MK_227_SERIAL2;
        if (strcmp(str, "JSY-MK-227 (Remote)") == 0) return Source::JSY_MK_227_REMOTE;
        if (strcmp(str, "JSY-MK-229 (Serial1)") == 0) return Source::JSY_MK_229_SERIAL1;
        if (strcmp(str, "JSY-MK-229 (Serial2)") == 0) return Source::JSY_MK_229_SERIAL2;
        if (strcmp(str, "JSY-MK-229 (Remote)") == 0) return Source::JSY_MK_229_REMOTE;
        if (strcmp(str, "JSY-MK-193 Channel 1 (Serial1)") == 0) return Source::JSY_MK_193_CH1_SERIAL1;
        if (strcmp(str, "JSY-MK-193 Channel 1 (Serial2)") == 0) return Source::JSY_MK_193_CH1_SERIAL2;
        if (strcmp(str, "JSY-MK-193 Channel 1 (Remote)") == 0) return Source::JSY_MK_193_CH1_REMOTE;
        if (strcmp(str, "JSY-MK-193 Channel 2 (Serial1)") == 0) return Source::JSY_MK_193_CH2_SERIAL1;
        if (strcmp(str, "JSY-MK-193 Channel 2 (Serial2)") == 0) return Source::JSY_MK_193_CH2_SERIAL2;
        if (strcmp(str, "JSY-MK-193 Channel 2 (Remote)") == 0) return Source::JSY_MK_193_CH2_REMOTE;
        if (strcmp(str, "JSY-MK-194 Channel 1 (Serial1)") == 0) return Source::JSY_MK_194_CH1_SERIAL1;
        if (strcmp(str, "JSY-MK-194 Channel 1 (Serial2)") == 0) return Source::JSY_MK_194_CH1_SERIAL2;
        if (strcmp(str, "JSY-MK-194 Channel 1 (Remote)") == 0) return Source::JSY_MK_194_CH1_REMOTE;
        if (strcmp(str, "JSY-MK-194 Channel 2 (Serial1)") == 0) return Source::JSY_MK_194_CH2_SERIAL1;
        if (strcmp(str, "JSY-MK-194 Channel 2 (Serial2)") == 0) return Source::JSY_MK_194_CH2_SERIAL2;
        if (strcmp(str, "JSY-MK-194 Channel 2 (Remote)") == 0) return Source::JSY_MK_194_CH2_REMOTE;
        if (strcmp(str, "JSY-MK-333 (Serial1)") == 0) return Source::JSY_MK_333_SERIAL1;
        if (strcmp(str, "JSY-MK-333 (Serial2)") == 0) return Source::JSY_MK_333_SERIAL2;
        if (strcmp(str, "JSY-MK-333 (Remote)") == 0) return Source::JSY_MK_333_REMOTE;
        return Source::UNKNOWN;
      }

      static const char* getSources() { return ",MQTT,Victron,JSY-MK-163 (Serial1),JSY-MK-163 (Serial2),JSY-MK-163 (Remote),JSY-MK-227 (Serial1),JSY-MK-227 (Serial2),JSY-MK-227 (Remote),JSY-MK-229 (Serial1),JSY-MK-229 (Serial2),JSY-MK-229 (Remote),JSY-MK-193 Channel 1 (Serial1),JSY-MK-193 Channel 1 (Serial2),JSY-MK-193 Channel 1 (Remote),JSY-MK-193 Channel 2 (Serial1),JSY-MK-193 Channel 2 (Serial2),JSY-MK-193 Channel 2 (Remote),JSY-MK-194 Channel 1 (Serial1),JSY-MK-194 Channel 1 (Serial2),JSY-MK-194 Channel 1 (Remote),JSY-MK-194 Channel 2 (Serial1),JSY-MK-194 Channel 2 (Serial2),JSY-MK-194 Channel 2 (Remote),JSY-MK-333 (Serial1),JSY-MK-333 (Serial2),JSY-MK-333 (Remote)"; }

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

        if (_metrics.isPresent()) {
          root["source"]["type"] = getSourceString();
          root["source"]["time"] = _metrics.getLastUpdateTime();
          toJson(root["source"], _metrics.get());
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
      Source _source = Source::UNKNOWN;
      ExpiringValue<Metrics> _metrics;
  };
} // namespace Mycila
