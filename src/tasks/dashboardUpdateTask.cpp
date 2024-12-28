// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task dashboardUpdateTask("Dashboard Update", [](void* params) {
  if (website.pidCharts())
    website.updatePID();
  website.updateCards();
  website.updateCharts();
  dashboard.sendUpdates();
});
