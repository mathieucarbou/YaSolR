// SPDX-License-Identifier: %GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task debugTask("Debug", [](void* params) {
  logger.info(TAG, "Free Heap: %" PRIu32, ESP.getFreeHeap());
  Mycila::TaskMonitor.log();
  coreTaskManager.log();
  unsafeTaskManager.log();
  jsyTaskManager.log();
  pzemTaskManager.log();
});
