// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

static const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  logger.debug(TAG, "Task '%s' finished in %" PRIu32 " us", me.getName(), elapsed);
};

Mycila::Logger logger;
Mycila::Task* loggingTask;

void yasolr_init_logging() {
  logger.info(TAG, "Initialize logging...");

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    logger.setLevel(ARDUHAL_LOG_LEVEL_DEBUG);
    esp_log_level_set("*", static_cast<esp_log_level_t>(ARDUHAL_LOG_LEVEL_DEBUG));

#ifdef APP_MODEL_PRO
    WebSerial.setID(Mycila::AppInfo.firmware.c_str());
    WebSerial.setTitle((Mycila::AppInfo.name + " Web Console").c_str());
    WebSerial.setInput(false);
#endif
    WebSerial.begin(&webServer, "/console");
    logger.forwardTo(&WebSerial);

    loggingTask = new Mycila::Task("Debug", [](void* params) {
      logger.info(TAG, "Free Heap: %" PRIu32, ESP.getFreeHeap());
      Mycila::TaskMonitor.log();
      coreTaskManager.log();
      unsafeTaskManager.log();
      if (jsyTaskManager)
        jsyTaskManager->log();
      if (pzemTaskManager)
        pzemTaskManager->log();
    });

    loggingTask->setInterval(20 * Mycila::TaskDuration::SECONDS);
    loggingTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    loggingTask->setManager(unsafeTaskManager);
  } else {
    logger.setLevel(ARDUHAL_LOG_LEVEL_INFO);
    esp_log_level_set("*", static_cast<esp_log_level_t>(ARDUHAL_LOG_LEVEL_INFO));
  }
}
