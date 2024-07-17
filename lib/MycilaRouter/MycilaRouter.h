// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaGrid.h>
#include <MycilaJSY.h>
#include <MycilaPID.h>
#include <MycilaRouterOutput.h>

#include <algorithm>
#include <vector>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#ifndef MYCILA_ROUTER_OUTPUT_COUNT
  #define MYCILA_ROUTER_OUTPUT_COUNT 2
#endif

namespace Mycila {

  typedef struct {
      float apparentPower = 0;
      float current = 0;
      float dimmedVoltage = 0;
      float energy = 0;
      float power = 0;
      float powerFactor = 0;
      float resistance = 0;
      float thdi = 0;
      float voltage = 0;
  } RouterMetrics;

  class Router {
    public:
      Router(PID& pidController, JSY& jsy) : _pidController(&pidController), _jsy(&jsy) {}

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

      void divert(float gridVoltage, float gridPower) {
        float powerToDivert = _pidController->compute(gridPower);
        for (size_t i = 0; i < MYCILA_ROUTER_OUTPUT_COUNT; i++) {
          RouterOutput& output = *_outputs[i];
          if (output.isAutoDimmerEnabled()) {
            const float usedPower = output.autoDivert(gridVoltage, powerToDivert);
            powerToDivert -= usedPower;
          }
        }
      }

      void noDivert() {
        for (size_t i = 0; i < MYCILA_ROUTER_OUTPUT_COUNT; i++) {
          RouterOutput& output = *_outputs[i];
          if (output.isAutoDimmerEnabled()) {
            output.autoDivert(0, 0);
          }
        }
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root, float voltage) const {
        RouterMetrics routerMeasurements;
        getMeasurements(routerMeasurements);
        toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

        RouterMetrics routerMetrics;
        getRouterMetrics(routerMetrics, voltage);
        toJson(root["metrics"].to<JsonObject>(), routerMetrics);

        for (const auto& output : _outputs)
          output->toJson(root[output->getName()].to<JsonObject>(), voltage);

        _pidController->toJson(root["pid"].to<JsonObject>());
      }

      static void toJson(const JsonObject& dest, const RouterMetrics& metrics) {
        dest["apparent_power"] = metrics.apparentPower;
        dest["current"] = metrics.current;
        dest["energy"] = metrics.energy;
        dest["power"] = metrics.power;
        dest["power_factor"] = metrics.powerFactor;
        dest["resistance"] = metrics.resistance;
        dest["thdi"] = metrics.thdi;
        dest["voltage"] = metrics.voltage;
        dest["voltage_dimmed"] = metrics.dimmedVoltage;
      }
#endif

      // get router theoretical metrics based on the dimmer states and the grid voltage
      void getRouterMetrics(RouterMetrics& metrics, float voltage) const {
        metrics.voltage = voltage;

        for (const auto& output : _outputs) {
          RouterOutputMetrics outputMetrics;
          output->getDimmerMetrics(outputMetrics, voltage);
          metrics.energy += outputMetrics.energy;
          metrics.apparentPower += outputMetrics.apparentPower;
          metrics.current += outputMetrics.current;
          metrics.power += outputMetrics.power;
        }

        metrics.powerFactor = metrics.apparentPower == 0 ? 0 : metrics.power / metrics.apparentPower;
        metrics.dimmedVoltage = metrics.powerFactor * metrics.voltage;
        metrics.resistance = metrics.current == 0 ? 0 : metrics.power / (metrics.current * metrics.current);
        metrics.thdi = metrics.powerFactor == 0 ? 0 : sqrt(1 / pow(metrics.powerFactor, 2) - 1);

        if (!metrics.energy)
          metrics.energy = _jsy->getEnergy1() + _jsy->getEnergyReturned1();
      }

      void getMeasurements(RouterMetrics& metrics) const {
        metrics.voltage = _jsy->getVoltage1();
        metrics.energy = _jsy->getEnergy1() + _jsy->getEnergyReturned1();
        for (const auto& output : _outputs) {
          if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
            metrics.apparentPower = _jsy->getApparentPower1();
            metrics.current = _jsy->getCurrent1();
            metrics.dimmedVoltage = _jsy->getDimmedVoltage1();
            metrics.power = _jsy->getPower1();
            metrics.powerFactor = _jsy->getPowerFactor1();
            metrics.resistance = _jsy->getResistance1();
            metrics.thdi = _jsy->getTHDi1(0);
            break;
          }
        }
      }

    private:
      PID* _pidController;
      JSY* _jsy;
      std::vector<RouterOutput*> _outputs;
  };
} // namespace Mycila
