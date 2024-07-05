// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task dashboardTask("Dashboard", [](void* params) {
  YaSolR::Website.updateCards();
  YaSolR::Website.updateCharts();
  if (!routingTask.isEnabled() || !config.getBool(KEY_ENABLE_PID_VIEW)) {
    dashboard.sendUpdates();
  }
});
