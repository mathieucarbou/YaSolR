// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaPID.h>
#include <MycilaRouterOutput.h>

#include <algorithm>
#include <vector>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  class Router {
    public:
      enum class Source {
        METRICS_INDIVIDUAL,
        METRICS_COMBINED,
        METRICS_CALCULATED,
        METRICS_UNKNOWN
      };
      typedef struct {
          Source source = Source::METRICS_UNKNOWN;
          float apparentPower = 0;
          float current = 0;
          uint32_t energy = 0;
          float power = 0;
          float powerFactor = NAN;
          float resistance = NAN;
          float thdi = NAN;
      } Metrics;

      explicit Router(PID& pidController) : _pidController(&pidController) {}

      void addOutput(RouterOutput& output) { _outputs.push_back(&output); }
      const std::vector<RouterOutput*>& getOutputs() const { return _outputs; }

      // aggregated measurements for all outputs combined, if available
      ExpiringValue<Metrics>& metrics() { return _metrics; }
      const ExpiringValue<Metrics>& metrics() const { return _metrics; }

      bool isAutoDimmerEnabled() const {
        for (const auto& output : _outputs) {
          if (output->isAutoDimmerEnabled()) {
            return true;
          }
        }
        return false;
      }

      void divert(float gridVoltage, float gridPower) {
        if (isCalibrationRunning())
          return;
        const float powerToDivert = _pidController->compute(gridPower);
        float routedPower = 0;
        for (const auto& output : _outputs) {
          routedPower += output->autoDivert(gridVoltage, std::max(0.0f, powerToDivert - routedPower));
        }
        // we have some grid power to divert and grid voltage but we cannot divert
        // reset the PID so that we can start fresh once we will divert
        if (!routedPower) {
          _pidController->reset(0);
        }
      }

      void noDivert() {
        if (isCalibrationRunning())
          return;
        for (const auto& output : _outputs) {
          output->autoDivert(0, 0);
        }
      }

      typedef std::function<void()> CalibrationCallback;
      void beginCalibration(size_t outputIndex, CalibrationCallback cb = nullptr);
      void continueCalibration();
      bool isCalibrationRunning() const { return _calibrationRunning; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root, float voltage) const {
        Metrics routerMeasurements;
        readMeasurements(routerMeasurements);
        toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

        JsonObject source = root["source"].to<JsonObject>();
        if (_metrics.isPresent()) {
          source["enabled"] = true;
          source["time"] = _metrics.getLastUpdateTime();
          toJson(source, _metrics.get());
        } else {
          source["enabled"] = false;
        }
      }

      static void toJson(const JsonObject& dest, const Metrics& metrics) {
        switch (metrics.source) {
          case Source::METRICS_COMBINED:
            dest["source"] = "combined";
            break;
          case Source::METRICS_CALCULATED:
            dest["source"] = "calculated";
            break;
          case Source::METRICS_INDIVIDUAL:
            dest["source"] = "individual";
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
        if (!std::isnan(metrics.power))
          dest["power"] = metrics.power;
        if (!std::isnan(metrics.powerFactor))
          dest["power_factor"] = metrics.powerFactor;
        if (!std::isnan(metrics.resistance))
          dest["resistance"] = metrics.resistance;
        if (!std::isnan(metrics.thdi))
          dest["thdi"] = metrics.thdi;
      }
#endif

      // get router measurements based on the connected JSY (for an aggregated view of all outputs) or PZEM per output
      bool readMeasurements(Metrics& metrics) const {
        RouterOutput::Metrics outputMetrics;
        metrics.source = Source::METRICS_UNKNOWN;

        for (size_t i = 0; i < _outputs.size(); i++) {
          if (_outputs[i]->readMeasurements(outputMetrics)) {
            metrics.source = Source::METRICS_INDIVIDUAL;
            // Note: energy is not accurate in the case of a virtual bypass through dimmer
            metrics.energy += outputMetrics.energy;
            metrics.apparentPower += outputMetrics.apparentPower;
            metrics.current += outputMetrics.current;
            metrics.power += outputMetrics.power;
          }
        }

        // we found some pzem ? we are done
        if (metrics.source == Source::METRICS_INDIVIDUAL) {
          metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
          metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
          metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);
          return true;
        }

        // no pzem found, let's check if we have a local JSY or remote JSY
        if (_metrics.isPresent()) {
          // Note: if one output is routing and the other one is doing a virtual bypass through dimmer, sadly we cannot have accurate measurements
          memcpy(&metrics, &_metrics.get(), sizeof(Metrics));
          metrics.source = Source::METRICS_COMBINED;
          return true;
        }

        return false;
      }

      bool calculateMetrics(Metrics& metrics, float gridVoltage) const {
        if (gridVoltage > 0) {
          RouterOutput::Metrics outputMetrics;
          metrics.source = Source::METRICS_CALCULATED;
          for (size_t i = 0; i < _outputs.size(); i++) {
            if (_outputs[i]->calculateMetrics(outputMetrics, gridVoltage)) {
              metrics.energy += outputMetrics.energy;
              metrics.apparentPower += outputMetrics.apparentPower;
              metrics.current += outputMetrics.current;
              metrics.power += outputMetrics.power;
            }
          }
          metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
          metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
          metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);
          return true;
        }
        return false;
      }

      std::optional<float> readTotalRoutedPower() const {
        float power = 0;
        for (size_t i = 0; i < _outputs.size(); i++) {
          power += _outputs[i]->readRoutedPower().value_or(NAN);
        }
        if (!std::isnan(power)) {
          return power;
        }
        if (_metrics.isPresent())
          return _metrics.get().power;
        return std::nullopt;
      }

      std::optional<float> calculateTotalRoutedPower(float gridVoltage) const {
        float power = 0;
        for (size_t i = 0; i < _outputs.size(); i++) {
          power += _outputs[i]->calculateRoutedPower(gridVoltage).value_or(NAN);
        }
        if (!std::isnan(power)) {
          return power;
        }
        return std::nullopt;
      }

      std::optional<float> readResistance() const {
        if (_metrics.isAbsent())
          return std::nullopt;
        return _metrics.get().resistance;
      }

    private:
      PID* _pidController;
      std::vector<RouterOutput*> _outputs;
      ExpiringValue<Metrics> _metrics;

      // calibration
      // 0: idle
      // 1: prepare
      // 2: output 1 50%
      // 3: output 1 100%
      // 4: cleanup
      uint8_t _calibrationStep = 0;
      uint32_t _calibrationStartTime = 0;
      size_t _calibrationOutputIndex = 0;
      bool _calibrationRunning = false;
      CalibrationCallback _calibrationCallback = nullptr;
  };
} // namespace Mycila
