// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRouter.h>

#include <MycilaGrid.h>
#include <MycilaLogger.h>

#define TAG "ROUTER"

extern Mycila::Grid grid;

float Mycila::RouterClass::getVirtualGridPower() const {
  return grid.getActivePower() - getTotalRoutedPower();
}

float Mycila::RouterClass::getTotalRoutedPower() const {
  if (!isRouting())
    return 0;

  // 1. check pzem
  float total = 0;
  for (auto& output : _outputs)
    total += output->getActivePower();

  if (total > 0)
    return total;

  // 2. fallback to JSY
  return _jsy != nullptr ? abs(_jsy->getPower1()) : 0;
}

float Mycila::RouterClass::getTotalPowerFactor() const {
  if (!isRouting())
    return 0;

  // 1. check pzem
  float va = 0;
  float power = 0;
  for (auto& output : _outputs) {
    va += output->getApparentPower();
    power += output->getActivePower();
  }

  if (va > 0 && power > 0)
    return power / va;

  // 2. fallback to JSY
  return _jsy != nullptr ? _jsy->getPowerFactor1() : 0;
}

float Mycila::RouterClass::getTotalTHDi() const {
  const float pf = getTotalPowerFactor();

  if (pf == 0)
    return 0;

  // https://fr.electrical-installation.org/frwiki/Indicateur_de_distorsion_harmonique_:_facteur_de_puissance
  constexpr float phi = 0; // no phase shift for resistive load between voltage and current (cos(phi) = 1)
  return sqrt(pow(cos(phi), 2) / pow(pf, 2) - 1);
}

float Mycila::RouterClass::getTotalRoutedEnergy() const {
  // 1. check pzem
  float total = 0;
  for (auto& output : _outputs)
    total += output->getEnergy();

  if (total > 0)
    return total;

  // 2. fallback to JSY
  return _jsy != nullptr ? _jsy->getEnergy1() + _jsy->getEnergyReturned1() : 0;
}

bool Mycila::RouterClass::isRouting() const {
  for (const auto& output : _outputs) {
    if (output->getState() == RouterOutputState::OUTPUT_ROUTING) {
      return true;
    }
  }
  return false;
}

void Mycila::RouterClass::adjustRouting() {
  // TODO: implement routing
}

namespace Mycila {
  RouterClass Router;
} // namespace Mycila
