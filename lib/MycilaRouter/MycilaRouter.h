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
      typedef struct {
          float apparentPower = 0;
          float current = 0;
          uint32_t energy = 0;
          float power = 0;
          float powerFactor = NAN;
          float resistance = NAN;
          float thdi = NAN;
          float voltage = NAN;
      } Metrics;

      explicit Router(PID& pidController) : _pidController(&pidController) {}

      void addOutput(RouterOutput& output) { _outputs.push_back(&output); }
      const std::vector<RouterOutput*>& getOutputs() const { return _outputs; }

      // aggregated measurements for all outputs combined, if available
      ExpiringValue<Metrics>& aggregatedMetrics() { return _aggregatedMetrics; }
      const ExpiringValue<Metrics>& aggregatedMetrics() const { return _aggregatedMetrics; }

      bool isRouting() const {
        for (const auto& output : _outputs) {
          if (output->getState() == RouterOutput::State::OUTPUT_ROUTING) {
            return true;
          }
        }
        return false;
      }

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
        float powerToDivert = _pidController->compute(gridPower);
        for (const auto& output : _outputs) {
          const float usedPower = output->autoDivert(gridVoltage, powerToDivert);
          powerToDivert = std::max(0.0f, powerToDivert - usedPower);
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
        Metrics* routerMeasurements = new Metrics();
        readMeasurements(*routerMeasurements);
        toJson(root["measurements"].to<JsonObject>(), *routerMeasurements);
        delete routerMeasurements;
        routerMeasurements = nullptr;

        JsonObject source = root["source"].to<JsonObject>();
        if (_aggregatedMetrics.isPresent()) {
          source["enabled"] = true;
          source["time"] = _aggregatedMetrics.getLastUpdateTime();
          toJson(source, _aggregatedMetrics.get());
        } else {
          source["enabled"] = false;
        }
      }

      static void toJson(const JsonObject& dest, const Metrics& metrics) {
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
        if (!std::isnan(metrics.voltage))
          dest["voltage"] = metrics.voltage;
      }
#endif

      // get router measurements based on the connected JSY (for an aggregated view of all outputs) or PZEM per output
      void readMeasurements(Metrics& metrics) const {
        for (size_t i = 0; i < _outputs.size(); i++) {
          RouterOutput::Metrics pzemMetrics;
          if (_outputs[i]->readMeasurements(pzemMetrics)) {
            metrics.voltage = pzemMetrics.voltage;
            metrics.energy += pzemMetrics.energy;
            metrics.apparentPower += pzemMetrics.apparentPower;
            metrics.current += pzemMetrics.current;
            metrics.power += pzemMetrics.power;
          }
        }

        // we found some pzem ? we are done
        if (metrics.voltage > 0) {
          metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
          metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
          metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);
          return;
        }

        // no pzem found, let's check if we have a local JSY or remote JSY
        if (_aggregatedMetrics.isPresent()) {
          if (isRouting()) {
            // Note: if one output is routing and the other one is doing a virtual bypass through dimmer, sadly we cannot have accurate measurements
            memcpy(&metrics, &_aggregatedMetrics.get(), sizeof(Metrics));
          } else {
            metrics.voltage = _aggregatedMetrics.get().voltage;
            metrics.energy = _aggregatedMetrics.get().energy;
          }
        }
      }

    private:
      PID* _pidController;
      std::vector<RouterOutput*> _outputs;
      ExpiringValue<Metrics> _aggregatedMetrics;

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
