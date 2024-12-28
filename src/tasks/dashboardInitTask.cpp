// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task dashboardInitTask("Dashboard Init", Mycila::TaskType::ONCE, [](void* params) {
  website.initCards();
  website.updateCards();
  dashboard.sendUpdates();
});
