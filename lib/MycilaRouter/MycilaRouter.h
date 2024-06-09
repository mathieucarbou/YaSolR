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
  class Router {
    public:
      explicit Router(JSY& jsy) : _jsy(&jsy) {}

      void addOutput(RouterOutput& output) { _outputs.push_back(&output); }

      bool isRouting() const {
        for (const auto& output : _outputs) {
          if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
            return true;
          }
        }
        return false;
      }

      float getEnergy() const {
        bool aggregate = false;
        float total = 0;
        for (auto& output : _outputs) {
          total += output->getEnergy();
          aggregate |= output->hasMetrics();
        }
        return aggregate ? total : _jsy->getEnergy1() + _jsy->getEnergyReturned1();
      }
      float getCurrent() const {
        if (!isRouting())
          return 0;
        bool aggregate = false;
        float total = 0;
        for (auto& output : _outputs) {
          total += output->getCurrent();
          aggregate |= output->hasMetrics();
        }
        return aggregate ? total : _jsy->getCurrent1();
      }
      float getApparentPower() const {
        if (!isRouting())
          return 0;
        bool aggregate = false;
        float total = 0;
        for (auto& output : _outputs) {
          total += output->getApparentPower();
          aggregate |= output->hasMetrics();
        }
        return aggregate ? total : _jsy->getApparentPower1();
      }
      float getPowerFactor() const {
        if (!isRouting())
          return 0;
        bool aggregate = false;
        float va = 0;
        float power = 0;
        for (auto& output : _outputs) {
          va += output->getApparentPower();
          power += output->getActivePower();
          aggregate |= output->hasMetrics();
        }
        return aggregate ? (va == 0 ? 0 : power / va) : _jsy->getPowerFactor1();
      }
      float getVoltage() const {
        float v = _jsy->getVoltage1();
        if (v > 0)
          return v;
        for (auto& output : _outputs) {
          v = output->getVoltage();
          if (v > 0)
            return v;
        }
        return 0;
      }
      float getActivePower() const {
        if (!isRouting())
          return 0;
        bool aggregate = false;
        float total = 0;
        for (auto& output : _outputs) {
          total += output->getActivePower();
          aggregate |= output->hasMetrics();
        }
        return aggregate ? total : abs(_jsy->getPower1());
      }
      float getTHDi() const {
        // https://fr.electrical-installation.org/frwiki/Indicateur_de_distorsion_harmonique_:_facteur_de_puissance
        // https://www.salicru.com/files/pagina/72/278/jn004a01_whitepaper-armonics_(1).pdf
        // For a resistive load, phi = 0 (no displacement between voltage and current)
        float pf = getPowerFactor();
        return pf == 0 ? 0 : sqrt(1 / pow(pf, 2) - 1);
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["apparent_power"] = getApparentPower();
        root["current"] = getCurrent();
        root["energy"] = getEnergy();
        root["power_factor"] = getPowerFactor();
        root["power"] = getActivePower();
        root["thdi"] = getTHDi();
        root["voltage"] = getVoltage();
      }
#endif

    private:
      JSY* _jsy;
      std::vector<RouterOutput*> _outputs;
  };
} // namespace Mycila
