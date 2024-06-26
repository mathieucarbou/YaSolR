// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

static const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  logger.debug(TAG, "%s in %" PRIu32 " us", me.getName(), elapsed);
};

Mycila::Task initLoggingTask("Init Logging", [](void* params) {
  logger.info(TAG, "Initializing logging...");

  const bool debug = config.getBool(KEY_ENABLE_DEBUG);

  if (debug) {
    logger.setLevel(debug ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
    esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));
  }

  profilerTask.setEnabled(debug);
  routerDebugTask.setEnabled(false);

  if (debug) {
    // Enable profiling for some FOREVER tasks
    dashboardTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    ds18Task.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    displayTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    jsyTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    mqttPublishTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    profilerTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    pzemTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    routerTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    routingTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  } else {
    dashboardTask.disableProfiling();
    ds18Task.disableProfiling();
    displayTask.disableProfiling();
    jsyTask.disableProfiling();
    mqttPublishTask.disableProfiling();
    profilerTask.disableProfiling();
    pzemTask.disableProfiling();
    routerTask.disableProfiling();
    routingTask.disableProfiling();
  }

  // Log execution time for some "ONCE" tasks
  haDiscoveryTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttPublishConfigTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttPublishStaticTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  mqttPublishTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  otaTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  profilerTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  pzemO1PairingTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  pzemO2PairingTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  relayTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  routerDebugTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);

  if (!debug) {
    logger.setLevel(debug ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
    esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));
  }
});
