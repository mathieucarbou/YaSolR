// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include "MycilaDimmer.h"
#include <Arduino.h>

#define MYCILA_DIMMER_NOT_LUT

namespace Mycila {
  namespace DimmerInternal {
    static float computeAngle(uint8_t level) {
      // https://electronics.stackexchange.com/questions/414370/calculate-angle-for-triac-phase-control/414399#414399
      return acos(2 * static_cast<float>(level) / MYCILA_DIMMER_MAX_LEVEL - 1);
    }
  } // namespace DimmerInternal
} // namespace Mycila

uint16_t Mycila::Dimmer::_lookupFiringDelay(uint8_t level, float frequency) {
  if (level == 0)
    return 500000 / frequency; // semi-period in microseconds

  if (level == MYCILA_DIMMER_MAX_LEVEL)
    return 0;

  return Mycila::Dimmer::_phaseAngleToDelay(Mycila::DimmerInternal::computeAngle(level), frequency);
}

float Mycila::Dimmer::_lookupVrmsFactor(uint8_t level, float frequency) {
  if (level == 0)
    return 0;

  if (level == MYCILA_DIMMER_MAX_LEVEL)
    return 1;

  return Mycila::Dimmer::_vrmsFactor(Mycila::DimmerInternal::computeAngle(level));
}
