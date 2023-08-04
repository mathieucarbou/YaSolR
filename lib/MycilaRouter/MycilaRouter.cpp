// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRouter.h>

#include <MycilaGrid.h>
#include <MycilaJSY.h>
#include <MycilaLogger.h>

#define TAG "ROUTER"

void Mycila::RouterClass::toJson(const JsonObject& root) const {
  root["energy"] = getTotalRoutedEnergy();
  root["power_factor"] = getTotalPowerFactor();
  root["power"] = getTotalRoutedPower();
  root["thdi"] = getTotalTHDi();
  root["virtual_grid_power"] = getVirtualGridPower();
  for (const auto& output : _outputs) {
    output->toJson(root[output->getName()].to<JsonObject>());
  }
}

float Mycila::RouterClass::getVirtualGridPower() const {
  return Grid.getPower() - getTotalRoutedPower();
}

float Mycila::RouterClass::getTotalRoutedPower() const {
  for (auto& output : _outputs) {
    if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
      return abs(JSY.power1);
    }
  }
  return 0;
}

float Mycila::RouterClass::getTotalTHDi() const {
  for (auto& output : _outputs) {
    if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
      // https://fr.electrical-installation.org/frwiki/Indicateur_de_distorsion_harmonique_:_facteur_de_puissance
      const float phi = 0; // no phase shift for resistive load between voltage and current (cos(phi) = 1)
      const float pf = getTotalPowerFactor();
      return sqrt(pow(cos(phi), 2) / pow(pf, 2) - 1);
    }
  }
  return 0;
}

float Mycila::RouterClass::getTotalPowerFactor() const {
  for (auto& output : _outputs) {
    if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
      return JSY.powerFactor1;
    }
  }
  return 0;
}

float Mycila::RouterClass::getTotalRoutedEnergy() const {
  return JSY.energy1 + JSY.energyReturned1;
}

bool Mycila::RouterClass::isRouting() const {
  for (const auto& output : _outputs) {
    if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
      return true;
    }
  }
  return false;
}

namespace Mycila {
  RouterClass Router;
} // namespace Mycila

//TODO: do we need power per output ?
// float Mycila::RouterClass::getOutputPower(const RouterOutput* output) const {
//   const int idx = &output - &_outputs[0];

//   int routing = 0;
//   float dimmerLevel = 0;

//   for (size_t i = 0, max = _outputs.size(); i < max; i++) {
//     const uint8_t level = _outputs[i].getDimmer()->getLevel();
//     if (level > 0) {
//       routing++;
//       if (i == idx)
//         dimmerLevel = level;
//     } else if (i == idx) {
//       return 0;
//     }
//   }

//   // more accurate
//   if (routing == 1)
//     return abs(JSY.power1);

//   // approximation with P = U * U / R
//   const float u = Grid.getVoltage();
//   const float r = output.getResistance(idx);

//   return (u * u / r) * (dimmerLevel / 100.0);
// }
