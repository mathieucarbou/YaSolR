// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>
#include <cstdio>

extern YaSolR::Website website;

Mycila::Task dashboardInitTask("Dashboard Init", Mycila::TaskType::ONCE, [](void* params) {
  website.initCards();
  website.updateCards();
  dashboard.sendUpdates();
});

Mycila::Task dashboardUpdateTask("Dashboard Update", [](void* params) {
  if (config.getBool(KEY_ENABLE_PID_VIEW)) {
    // WebSocket
    if (wsDebugPID.count()) {
      AsyncWebSocketMessageBuffer* buffer = wsDebugPID.makeBuffer(256);
      snprintf(reinterpret_cast<char*>(buffer->get()),
               256,
               "%d,%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
               static_cast<int>(pidController.getProportionalMode()),
               static_cast<int>(pidController.getDerivativeMode()),
               static_cast<int>(pidController.getIntegralCorrectionMode()),
               pidController.isReversed(),
               static_cast<int>(pidController.getSetPoint()),
               pidController.getKp(),
               pidController.getKi(),
               pidController.getKd(),
               static_cast<int>(pidController.getOutputMin()),
               static_cast<int>(pidController.getOutputMax()),
               pidController.getInput(),
               pidController.getOutput(),
               pidController.getError(),
               pidController.getSum(),
               pidController.getPTerm(),
               pidController.getITerm(),
               pidController.getDTerm()); // NOLINT
      wsDebugPID.textAll(buffer);
    }
  }

  website.updatePID();
  website.updateCards();
  website.updateCharts();
  dashboard.sendUpdates();
});
