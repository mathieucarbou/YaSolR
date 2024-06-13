// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task initDashboardTask("Init Dashboard", [](void* params) {
  logger.info(TAG, "Initializing dashboard...");
  YaSolR::Website.initLayout();
  YaSolR::Website.initCards();
  YaSolR::Website.updateCards();
});
