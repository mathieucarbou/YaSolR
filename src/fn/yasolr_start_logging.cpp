// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

static const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  logger.debug(TAG, "Task '%s' finished in %" PRIu32 " us", me.getName(), elapsed);
};

Mycila::Logger logger;

Mycila::Task loggingTask("Debug", [](void* params) {
  logger.info(TAG, "Free Heap: %" PRIu32, ESP.getFreeHeap());
  Mycila::TaskMonitor.log();
  coreTaskManager.log();
  unsafeTaskManager.log();
  if (jsyTaskManager)
    jsyTaskManager->log();
  if (pzemTaskManager)
    pzemTaskManager->log();
});

void yasolr_start_logging() {
#ifdef APP_MODEL_PRO
  WebSerial.setID(Mycila::AppInfo.firmware.c_str());
  WebSerial.setTitle((Mycila::AppInfo.name + " Web Console").c_str());
  WebSerial.setInput(false);
#endif

  WebSerial.begin(&webServer, "/console");
  logger.forwardTo(&WebSerial);

  loggingTask.setInterval(20 * Mycila::TaskDuration::SECONDS);
  loggingTask.setManager(coreTaskManager);
}

void yasolr_configure_logging() {
  logger.info(TAG, "Configuring logging...");

  const bool debug = config.getBool(KEY_ENABLE_DEBUG);

  logger.setLevel(debug ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
  esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));

  loggingMiddleware.setEnabled(debug);
  loggingTask.setEnabled(debug);

  if (debug) {
    Mycila::TaskMonitor.begin();

    dashboardUpdateTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    loggingTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (ds18Task)
      ds18Task->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (displayTask)
      displayTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (jsyTask)
      jsyTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (mqttPublishTask)
      mqttPublishTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    if (pzemTask)
      pzemTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);

  } else {
    Mycila::TaskMonitor.end();

    dashboardUpdateTask.disableProfiling();
    loggingTask.disableProfiling();
    if (ds18Task)
      ds18Task->disableProfiling();
    if (displayTask)
      displayTask->disableProfiling();
    if (jsyTask)
      jsyTask->disableProfiling();
    if (mqttPublishTask)
      mqttPublishTask->disableProfiling();
    if (pzemTask)
      pzemTask->disableProfiling();
  }

  // Log execution time for some "ONCE" tasks
  dashboardInitTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (ds18Task)
    ds18Task->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (haDiscoveryTask)
    haDiscoveryTask->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (mqttPublishConfigTask)
    mqttPublishConfigTask->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (mqttPublishStaticTask)
    mqttPublishStaticTask->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (mqttPublishTask)
    mqttPublishTask->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  networkStartTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  safeBootTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  loggingTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (pzemO1PairingTask)
    pzemO1PairingTask->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  if (pzemO2PairingTask)
    pzemO2PairingTask->setCallback(debug ? LOG_EXEC_TIME : nullptr);
  relayTask.setCallback(debug ? LOG_EXEC_TIME : nullptr);
}
