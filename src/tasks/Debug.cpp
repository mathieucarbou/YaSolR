// SPDX-License-Identifier: %GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task debugTask("Debug", [](void* params) {
  Mycila::TaskMonitor.log();
  coreTaskManager.log();
  pioTaskManager.log();
  mqttTaskManager.log();
  jsyTaskManager.log();
  pzemTaskManager.log();
  routingTaskManager.log();
});
