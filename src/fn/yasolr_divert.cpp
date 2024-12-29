// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

void yasolr_divert() {
  if (router.isCalibrationRunning())
    return;

  if (!output1.isAutoDimmerEnabled() && !output2.isAutoDimmerEnabled())
    return;

  std::optional<float> voltage = grid.getVoltage();

  if (voltage.has_value() && grid.getPower().isPresent()) {
    router.divert(voltage.value(), grid.getPower().get());
    if (website.pidCharts()) {
      dashboardUpdateTask.requestEarlyRun();
    }
  }
}
