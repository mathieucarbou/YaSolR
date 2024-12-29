// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

static const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  logger.debug(TAG, "Task '%s' finished in %" PRIu32 " us", me.getName(), elapsed);
};

Mycila::Logger logger;

Mycila::Task debugTask("Debug", [](void* params) {
  logger.info(TAG, "Free Heap: %" PRIu32, ESP.getFreeHeap());
  Mycila::TaskMonitor.log();
  coreTaskManager.log();
  unsafeTaskManager.log();
  if (jsyTaskManager)
    jsyTaskManager->log();
  pzemTaskManager.log();
});

void yasolr_start_logging() {
#ifdef APP_MODEL_PRO
  WebSerial.setID(Mycila::AppInfo.firmware.c_str());
  WebSerial.setTitle((Mycila::AppInfo.name + " Web Console").c_str());
  WebSerial.setInput(false);
#endif
  WebSerial.begin(&webServer, "/console");
  logger.forwardTo(&WebSerial);

  debugTask.setInterval(20 * Mycila::TaskDuration::SECONDS);
  debugTask.setManager(coreTaskManager);
};

void yasolr_configure_logging() {
  logger.info(TAG, "Configuring logging...");

  const bool debug = config.getBool(KEY_ENABLE_DEBUG);

  logger.setLevel(debug ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
  esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));

  loggingMiddleware.setEnabled(debug);
  debugTask.setEnabled(debug);

  if (debug) {
    // Enable profiling for some FOREVER tasks
    dashboardUpdateTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    debugTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (ds18Task)
      ds18Task->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (displayTask)
      displayTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (jsyTask)
      jsyTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    mqttPublishTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    pzemTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  } else {
    dashboardUpdateTask.disableProfiling();
    debugTask.disableProfiling();
    if (ds18Task)
      ds18Task->disableProfiling();
    if (displayTask)
      displayTask->disableProfiling();
    if (jsyTask)
      jsyTask->disableProfiling();
    mqttPublishTask.disableProfiling();
    pzemTask.disableProfiling();
  }

  // Log execution time for some "ONCE" tasks
  dashboardInitTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (ds18Task)
    ds18Task->setCallback(debug ? LOG_EXEC_TIME : nullptr);
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
}