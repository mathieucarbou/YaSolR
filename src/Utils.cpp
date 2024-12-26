// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

float detectGridFrequency() {
  float frequency = round(grid.getFrequency().value_or(NAN));
  if (!isnanf(frequency) && frequency)
    return frequency;

  frequency = round(pzemO1.data.frequency);
  if (!isnanf(frequency) && frequency)
    return frequency;

  frequency = round(pzemO2.data.frequency);
  if (!isnanf(frequency) && frequency)
    return frequency;

  frequency = config.getFloat(KEY_GRID_FREQUENCY);
  if (frequency)
    return frequency;

  frequency = pulseAnalyzer.getNominalGridFrequency();
  if (frequency)
    return frequency;

  return 0;
}
