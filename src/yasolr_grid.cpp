// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Grid grid;

void yasolr_init_grid() {
  grid.setSource(config.getString(KEY_GRID_SOURCE));
}

float yasolr_frequency() {
  // 1. check if frequency is set in config
  float frequency = config.get<uint8_t>(KEY_GRID_FREQUENCY);
  if ((frequency >= 48.0f && frequency <= 52.0f) || (frequency >= 58.0f && frequency <= 62.0f))
    return frequency;

  // 2. check if frequency is set from a measurement devices (PZEM, JSY, ...)
  frequency = std::round(grid.getFrequency().value_or(0));
  if ((frequency >= 48.0f && frequency <= 52.0f) || (frequency >= 58.0f && frequency <= 62.0f))
    return frequency;

  // 3. check if frequency is set in pulse analyzer
  // PulseAnalyzer can sometimes detect a semi-period longer or smaller...
  if (pulseAnalyzer) {
    frequency = pulseAnalyzer->getNominalGridFrequency();
    if (frequency >= 48.0f && frequency <= 52.0f)
      return 50.0f;
    if (frequency >= 58.0f && frequency <= 62.0f)
      return 60.0f;
  }

  return 0.0f;
}
