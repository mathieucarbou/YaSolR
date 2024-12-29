// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::TaskManager coreTaskManager("y-core");
Mycila::TaskManager unsafeTaskManager("y-unsafe");

Mycila::Task resetTask("Reset", Mycila::TaskType::ONCE, [](void* params) {
  logger.warn("YaSolR", "Resetting %s", Mycila::AppInfo.nameModelVersion.c_str());
  config.clear();
  Mycila::System::restart(500);
});

Mycila::Task restartTask("Restart", Mycila::TaskType::ONCE, [](void* params) {
  logger.warn("YaSolR", "Restarting %s", Mycila::AppInfo.nameModelVersion.c_str());
  Mycila::System::restart(500);
});

Mycila::Task safeBootTask("SafeBoot", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Restarting %s in SafeBoot mode...", Mycila::AppInfo.nameModelVersion.c_str());
  Mycila::System::restartFactory("safeboot");
});

void yasolr_start_system() {
  resetTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
  safeBootTask.setManager(coreTaskManager);

  Mycila::TaskMonitor.addTask(coreTaskManager.getName());   // YaSolR
  Mycila::TaskMonitor.addTask(unsafeTaskManager.getName()); // YaSolR
}
