// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Grid grid;

float yasolr_frequency() {
  // 1. check if frequency is set in config
  float frequency = config.get<uint8_t>(KEY_GRID_FREQUENCY);
  if ((frequency >= 49.0f && frequency <= 51.0f) || (frequency >= 59.0f && frequency <= 61.0f))
    return frequency;

  // 2. check if frequency is set from a measurement devices (PZEM, JSY, ...)
  frequency = std::round(grid.getFrequency().value_or(NAN));
  if ((frequency >= 49.0f && frequency <= 51.0f) || (frequency >= 59.0f && frequency <= 61.0f))
    return frequency;

  // 3. check if frequency is set in pulse analyzer
  if (pulseAnalyzer) {
    frequency = pulseAnalyzer->getNominalGridFrequency();
    if ((frequency >= 49.0f && frequency <= 51.0f) || (frequency >= 59.0f && frequency <= 61.0f))
      return frequency;
  }

  return NAN;
}
