// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

extern YaSolR::Website website;

Mycila::Task initDashboard("Init Dashboard", [](void* params) {
  logger.info(TAG, "Initializing dashboard");
  website.initLayout();
  website.initCards();
  website.updateCards();
});
