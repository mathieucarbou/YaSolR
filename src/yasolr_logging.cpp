// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Logger logger;

static Mycila::Task* loggingTask = nullptr;
static WebSerial* webSerial = nullptr;

class LogStream : public Print {
  public:
    LogStream(const char* path, size_t limit) : _path(path), _limit(limit) {
      File file = LittleFS.open(_path, "r");
      _logSize = file ? file.size() : 0;
      file.close();
      _accepting = _logSize < _limit;
    }
    size_t write(const uint8_t* buffer, size_t size) override {
      if (!_accepting)
        return 0;
      if (_logSize + size > _limit) {
        _accepting = false;
        return 0;
      }
      File file = LittleFS.open(_path, "a");
      if (!file)
        return 0;
      size_t written = file.write(buffer, size);
      file.close();
      _logSize += written;
      return written;
    }
    size_t write(uint8_t c) override {
      assert(false);
      return 0;
    }

  private:
    const char* _path;
    const size_t _limit;
    size_t _logSize = 0;
    bool _accepting = false;
};

static void initWebSerial() {
  LOGI(TAG, "Redirecting logs to WebSerial");
  webSerial = new WebSerial();
#ifdef APP_MODEL_PRO
  webSerial->setID(Mycila::AppInfo.firmware.c_str());
  webSerial->setTitle((Mycila::AppInfo.name + " Web Console").c_str());
  webSerial->setInput(false);
#endif
  webSerial->begin(&webServer, "/console");
  logger.forwardTo(webSerial);
}

static void initLogDump() {
  LOGI(TAG, "Redirecting logs to " YASOLR_LOG_FILE);
  logger.forwardTo(new LogStream(YASOLR_LOG_FILE, 32 * 1024));
}

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
}

void yasolr_configure_logging() {
  LOGI(TAG, "Initialize logging");

  if (LittleFS.remove(YASOLR_LOG_FILE))
    LOGI(TAG, "Previous log file removed");

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    logger.setLevel(ARDUHAL_LOG_LEVEL_DEBUG);
    esp_log_level_set("*", ESP_LOG_DEBUG);

    initWebSerial();
    initLogDump();

    loggingTask = new Mycila::Task("Debug", [](void* params) {
      LOGI(TAG, "Free Heap: %" PRIu32, ESP.getFreeHeap());
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
    esp_log_level_set("*", ESP_LOG_INFO);
  }
}
