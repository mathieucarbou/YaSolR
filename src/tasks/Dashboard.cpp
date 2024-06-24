// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task dashboardCards("Dashboard", [](void* params) {
  YaSolR::Website.updateCards();
  dashboard.sendUpdates();
});

Mycila::Task dashboardCharts("Dashboard Charts", [](void* params) {
  YaSolR::Website.updateCharts();
  dashboard.sendUpdates();
});
