// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

float yasolr_frequency() {
  float frequency = round(grid.getFrequency().value_or(NAN));
  if (frequency > 0)
    return frequency;

  if (pzemO1) {
    frequency = round(pzemO1->data.frequency);
    if (frequency > 0)
      return frequency;
  }

  if (pzemO2) {
    frequency = round(pzemO2->data.frequency);
    if (frequency > 0)
      return frequency;
  }

  frequency = config.getFloat(KEY_GRID_FREQUENCY);
  if (frequency)
    return frequency;

  if (pulseAnalyzer) {
    frequency = pulseAnalyzer->getNominalGridFrequency();
    if (frequency)
      return frequency;
  }

  return 0;
}
