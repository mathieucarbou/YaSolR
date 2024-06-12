// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRouterOutput.h>

#include <MycilaJSY.h>
#include <vector>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {

  typedef struct {
      float apparentPower = 0;
      float current = 0;
      float energy = 0;
      float power = 0;
      float powerFactor = 0;
      float thdi = 0;
  } RouterMetrics;

  class Router {
    public:
      explicit Router(JSY& jsy) : _jsy(&jsy) {}

      void addOutput(RouterOutput& output) { _outputs.push_back(&output); }
      const std::vector<RouterOutput*>& getOutputs() const { return _outputs; }

      bool isRouting() const {
        for (const auto& output : _outputs) {
          if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
            return true;
          }
        }
        return false;
      }

      float getPower() const {
        float power = 0;
        bool routing = false;

        for (const auto& output : _outputs) {
          if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
            power += output->getPower();
            routing = true;
          }
        }

        if (routing && power == 0)
          power = _jsy->getPower1();

        return power;
      }

      void getMetrics(RouterMetrics& metrics) const {
        bool routing = false;

        for (const auto& output : _outputs) {
          if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
            routing = true;
            Mycila::RouterOutputMetrics outputMetrics;
            output->getMetrics(outputMetrics);
            metrics.apparentPower += outputMetrics.apparentPower;
            metrics.current += outputMetrics.current;
            metrics.energy += outputMetrics.energy;
            metrics.power += outputMetrics.power;
          }
        }

        if (routing) {
          if (metrics.apparentPower == 0)
            metrics.apparentPower = _jsy->getApparentPower1();

          if (metrics.current == 0)
            metrics.current = _jsy->getCurrent1();

          if (metrics.power == 0)
            metrics.power = _jsy->getPower1();

          metrics.powerFactor = metrics.apparentPower == 0 ? 0 : metrics.power / metrics.apparentPower;
          if (metrics.powerFactor == 0)
            metrics.powerFactor = _jsy->getPowerFactor1();

          metrics.thdi = metrics.powerFactor == 0 ? 0 : sqrt(1 / pow(metrics.powerFactor, 2) - 1);
        }

        if (metrics.energy == 0)
          metrics.energy = _jsy->getEnergy1() + _jsy->getEnergyReturned1();
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        RouterMetrics metrics;
        getMetrics(metrics);
        root["metrics"]["apparent_power"] = metrics.apparentPower;
        root["metrics"]["current"] = metrics.current;
        root["metrics"]["energy"] = metrics.energy;
        root["metrics"]["power"] = metrics.power;
        root["metrics"]["power_factor"] = metrics.powerFactor;
        root["metrics"]["thdi"] = metrics.thdi;
        for (const auto& output : _outputs)
          output->toJson(root[output->getName()].to<JsonObject>());
      }
#endif

    private:
      JSY* _jsy;
      std::vector<RouterOutput*> _outputs;
  };
} // namespace Mycila
