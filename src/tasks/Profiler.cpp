// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task profilerTask("Profiler", [](void* params) {
  Mycila::TaskMonitor.log();
  ioTaskManager.log();
  jsyTaskManager.log();
  loopTaskManager.log();
  pzemO1TaskManager.log();
  pzemO2TaskManager.log();
  routerTaskManager.log();
});