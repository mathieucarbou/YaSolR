// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task resetTask("Reset", Mycila::TaskType::ONCE, [](void* params) {
  logger.warn("YaSolR", "Resetting %s", Mycila::AppInfo.nameModelVersion.c_str());
  config.clear();
  Mycila::System::restart(500);
});
