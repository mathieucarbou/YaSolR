// SPDX-License-Identifier: %GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task debugTask("Debug", [](void* params) {
  Mycila::TaskMonitor.log();
  pioTaskManager.log();
  coreTaskManager.log();
  routerTaskManager.log();
  jsyTaskManager.log();
  pzemTaskManager.log();
  mqttTaskManager.log();
});
