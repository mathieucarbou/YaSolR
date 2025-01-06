// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Logger logger;
Mycila::Task* loggingTask = nullptr;
WebSerial* webSerial = nullptr;

void yasolr_init_logging() {
  logger.info(TAG, "Initialize logging...");

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    logger.setLevel(ARDUHAL_LOG_LEVEL_DEBUG);
    esp_log_level_set("*", static_cast<esp_log_level_t>(ARDUHAL_LOG_LEVEL_DEBUG));

#ifdef APP_MODEL_PRO
    webSerial = new WebSerial();
    webSerial->setID(Mycila::AppInfo.firmware.c_str());
    webSerial->setTitle((Mycila::AppInfo.name + " Web Console").c_str());
    webSerial->setInput(false);
#endif
    webSerial->begin(&webServer, "/console");
    logger.forwardTo(webSerial);

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
