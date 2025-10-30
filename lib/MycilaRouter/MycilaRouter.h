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
      void toJson(const JsonObject& root, float voltage) const;
      static void toJson(const JsonObject& dest, const Metrics& metrics);
#endif

      // get router measurements based on the connected JSY (for an aggregated view of all outputs) or PZEM per output
      void readMeasurements(Metrics& metrics) const;

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
