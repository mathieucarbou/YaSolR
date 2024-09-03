// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task restartTask("Restart", Mycila::TaskType::ONCE, [](void* params) {
  logger.warn("YaSolR", "Restarting %s", Mycila::AppInfo.nameModelVersion.c_str());
  Mycila::System::restart(500);
});
