// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#pragma once

#include <math.h>
#include <stdint.h>
#include <string.h>

#include <utility>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  namespace metric {
      enum class Kind {
        UNKNOWN,

        COMPUTED,

        // The metrics are shared between multiple outputs (e.g. same clamp, 2 wires) and cannot be attributed to a single output
        SHARED,

        JSY_MK_163,
        JSY_MK_193,
        JSY_MK_193_CH1,
        JSY_MK_193_CH2,
        JSY_MK_194,
        JSY_MK_194_CH1,
        JSY_MK_194_CH2,
        JSY_MK_227,
        JSY_MK_229,
        JSY_MK_333,

        JSY_REMOTE,
        JSY_SERIAL1,
        JSY_SERIAL2,

        PZEM,

        MQTT,
        VICTRON,
      };

      enum class Source {
        UNKNOWN,

        COMPUTED,

        // The metrics are shared between multiple outputs (e.g. same clamp, 2 wires) and cannot be attributed to a single output
        SHARED,

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

        PZEM_SERIAL1,
        PZEM_SERIAL2,

        MQTT,
        VICTRON,
      };

      static bool isUsing(Source source, Kind kind) {
        switch (source) {
          case Source::COMPUTED:               return kind == Kind::COMPUTED;
          case Source::SHARED:                 return kind == Kind::SHARED;
          case Source::MQTT:                   return kind == Kind::MQTT;
          case Source::VICTRON:                return kind == Kind::VICTRON;
          case Source::JSY_MK_163_SERIAL1:     return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_163;
          case Source::JSY_MK_163_SERIAL2:     return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_163;
          case Source::JSY_MK_163_REMOTE:      return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_163;
          case Source::JSY_MK_227_SERIAL1:     return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_227;
          case Source::JSY_MK_227_SERIAL2:     return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_227;
          case Source::JSY_MK_227_REMOTE:      return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_227;
          case Source::JSY_MK_229_SERIAL1:     return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_229;
          case Source::JSY_MK_229_SERIAL2:     return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_229;
          case Source::JSY_MK_229_REMOTE:      return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_229;
          case Source::JSY_MK_193_CH1_SERIAL1: return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_193 || kind == Kind::JSY_MK_193_CH1;
          case Source::JSY_MK_193_CH1_SERIAL2: return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_193 || kind == Kind::JSY_MK_193_CH1;
          case Source::JSY_MK_193_CH1_REMOTE:  return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_193 || kind == Kind::JSY_MK_193_CH1;
          case Source::JSY_MK_193_CH2_SERIAL1: return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_193 || kind == Kind::JSY_MK_193_CH2;
          case Source::JSY_MK_193_CH2_SERIAL2: return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_193 || kind == Kind::JSY_MK_193_CH2;
          case Source::JSY_MK_193_CH2_REMOTE:  return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_193 || kind == Kind::JSY_MK_193_CH2;
          case Source::JSY_MK_194_CH1_SERIAL1: return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_194 || kind == Kind::JSY_MK_194_CH1;
          case Source::JSY_MK_194_CH1_SERIAL2: return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_194 || kind == Kind::JSY_MK_194_CH1;
          case Source::JSY_MK_194_CH1_REMOTE:  return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_194 || kind == Kind::JSY_MK_194_CH1;
          case Source::JSY_MK_194_CH2_SERIAL1: return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_194 || kind == Kind::JSY_MK_194_CH2;
          case Source::JSY_MK_194_CH2_SERIAL2: return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_194 || kind == Kind::JSY_MK_194_CH2;
          case Source::JSY_MK_194_CH2_REMOTE:  return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_194 || kind == Kind::JSY_MK_194_CH2;
          case Source::JSY_MK_333_SERIAL1:     return kind == Kind::JSY_SERIAL1 || kind == Kind::JSY_MK_333;
          case Source::JSY_MK_333_SERIAL2:     return kind == Kind::JSY_SERIAL2 || kind == Kind::JSY_MK_333;
          case Source::JSY_MK_333_REMOTE:      return kind == Kind::JSY_REMOTE || kind == Kind::JSY_MK_333;
          case Source::PZEM_SERIAL1:           return kind == Kind::PZEM;
          case Source::PZEM_SERIAL2:           return kind == Kind::PZEM;
          default:                             return kind == Kind::UNKNOWN;
        }
        return false;
      }

      static const char* sourceToString(Source source) {
        switch (source) {
          case Source::COMPUTED:               return "Computed";
          case Source::SHARED:                 return "Shared";
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
          case Source::PZEM_SERIAL1:           return "PZEM (Serial1)";
          case Source::PZEM_SERIAL2:           return "PZEM (Serial2)";
          default:                             return "Unknown";
        }
      }

      static Source sourceFromString(const char* str) {
        if (strcmp(str, "Computed") == 0) return Source::COMPUTED;
        if (strcmp(str, "Shared") == 0) return Source::SHARED;
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
        if (strcmp(str, "PZEM (Serial1)") == 0) return Source::PZEM_SERIAL1;
        if (strcmp(str, "PZEM (Serial2)") == 0) return Source::PZEM_SERIAL2;
        return Source::UNKNOWN;
      }

      class Metrics {
        public:
          float apparentPower = NAN;
          float current = NAN;
          uint32_t energy = 0;
          uint32_t energyReturned = 0;
          float frequency = NAN;
          float power = NAN;
          float powerFactor = NAN;
          float resistance = NAN;
          float thdi = NAN;
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
            resistance = other.resistance;
            thdi = other.thdi;
            voltage = other.voltage;

            other.apparentPower = NAN;
            other.current = NAN;
            other.energy = 0;
            other.energyReturned = 0;
            other.frequency = NAN;
            other.power = NAN;
            other.powerFactor = NAN;
            other.resistance = NAN;
            other.thdi = NAN;
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
              resistance = other.resistance;
              thdi = other.thdi;
              voltage = other.voltage;

              other.apparentPower = NAN;
              other.current = NAN;
              other.energy = 0;
              other.energyReturned = 0;
              other.frequency = NAN;
              other.power = NAN;
              other.powerFactor = NAN;
              other.resistance = NAN;
              other.thdi = NAN;
              other.voltage = NAN;
            }
            return *this;
          }
          Metrics(const Metrics& other) = delete;
          Metrics& operator=(const Metrics& other) = delete;

          void zeroNaN() {
            // zero NaN values
            if (std::isnan(apparentPower))
              apparentPower = 0.0f;
            if (std::isnan(current))
              current = 0.0f;
            if (std::isnan(frequency))
              frequency = 0.0f;
            if (std::isnan(power))
              power = 0.0f;
            if (std::isnan(powerFactor))
              powerFactor = 0.0f;
            if (std::isnan(resistance))
              resistance = 0.0f;
            if (std::isnan(thdi))
              thdi = 0.0f;
            if (std::isnan(voltage))
              voltage = 0.0f;
          }

#ifdef MYCILA_JSON_SUPPORT
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
      };

      class MetricSupport {
          public:
            virtual ~MetricSupport() = default;

            virtual bool readMetrics(Metrics& metrics) const {
              if (_metrics.isPresent()) {
                memcpy(&metrics, &_metrics.get(), sizeof(Metrics));
                return true;
              }
              return false;
            }

            void setSource(Source source) {
              _source = source;
            }

            Source getSource() const {
              return _source;
            }

            void setSource(const char* src) {
              setSource(sourceFromString(src));
            }

            const char* getSourceString() const {
              return sourceToString(_source);
            }

            bool isUsing(Kind kind) const {
              return metric::isUsing(_source, kind);
            }

            bool isUsing(Source source) const {
              return _source == source;
            }

            void clearMetrics() {
              _metrics.reset();
            }

            void updateMetrics(Metrics metrics) {
              if (_metrics.neverUpdated()) {
                _metrics.setExpiration(_source == Source::MQTT ? 120000 : 10000);
              }
              _metrics.update(std::move(metrics));
            }

      protected:
          Source _source = Source::UNKNOWN;
          ExpiringValue<Metrics> _metrics;
      };
    } // namespace metric
} // namespace Mycila
