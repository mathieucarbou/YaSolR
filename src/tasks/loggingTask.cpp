// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

static const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  logger.debug(TAG, "Task '%s' finished in %" PRIu32 " us", me.getName(), elapsed);
};

Mycila::Task loggingTask("Logging", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Configuring logging...");

  const bool debug = config.getBool(KEY_ENABLE_DEBUG);

  logger.setLevel(debug ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
  esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));

  loggingMiddleware.setEnabled(debug);

  if (debug) {
    // Enable profiling for some FOREVER tasks
    dashboardUpdateTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    debugTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    ds18Task.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    displayTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (jsyTask)
      jsyTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    mqttPublishTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    pzemTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  } else {
    dashboardUpdateTask.disableProfiling();
    debugTask.disableProfiling();
    ds18Task.disableProfiling();
    displayTask.disableProfiling();
    if (jsyTask)
      jsyTask->disableProfiling();
    mqttPublishTask.disableProfiling();
    pzemTask.disableProfiling();
  }

  // Log execution time for some "ONCE" tasks
  dashboardInitTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  ds18Task.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  haDiscoveryTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttConfigTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttPublishConfigTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttPublishStaticTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttPublishTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  safeBootTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  debugTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  pzemO1PairingTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  pzemO2PairingTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  relayTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
});
