// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

float detectGridFrequency() {
  // first get res from the measurement tools
  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);

  float frequency = round(gridMetrics.frequency);
  if (frequency)
    return frequency;

  frequency = round(pzemO1.getFrequency());
  if (frequency)
    return frequency;

  frequency = round(pzemO2.getFrequency());
  if (frequency)
    return frequency;

  frequency = config.get(KEY_GRID_FREQUENCY).toInt();
  if (frequency)
    return frequency;

  frequency = round(pulseAnalyzer.getFrequency() / 2);
  if (frequency)
    return frequency;

  return 0;
}
