// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Logger logger;

static Mycila::Task* loggingTask = nullptr;
static WebSerial* webSerial = nullptr;

void yasolr_init_logging() {
  Serial.begin(YASOLR_SERIAL_BAUDRATE);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  logger.redirectArduinoLogs();
  logger.forwardTo(&Serial);
  logger.info(TAG, "Booting %s", Mycila::AppInfo.nameModelVersion.c_str());
}

void yasolr_configure_logging() {
  logger.info(TAG, "Initialize logging");

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    logger.setLevel(ARDUHAL_LOG_LEVEL_DEBUG);
    esp_log_level_set("*", static_cast<esp_log_level_t>(ARDUHAL_LOG_LEVEL_DEBUG));

    webSerial = new WebSerial();
#ifdef APP_MODEL_PRO
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

    loggingTask->setInterval(30000);
    loggingTask->enableProfiling();

    unsafeTaskManager.addTask(*loggingTask);

  } else {
    logger.setLevel(ARDUHAL_LOG_LEVEL_INFO);
    esp_log_level_set("*", static_cast<esp_log_level_t>(ARDUHAL_LOG_LEVEL_INFO));
  }
}
