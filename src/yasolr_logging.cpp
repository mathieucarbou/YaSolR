// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

class LogStream : public Print {
  public:
    LogStream(const char* path, size_t limit, std::function<void()> onEnd) : _path(path), _limit(limit), _onEnd(onEnd) {
      File file = LittleFS.open(_path, "r");
      _logSize = file ? file.size() : 0;
      file.close();
      if (_logSize >= _limit) {
        if (_onEnd)
          _onEnd();
        _onEnd = nullptr;
      }
    }

    size_t write(const uint8_t* buffer, size_t size) override {
      if (_onEnd == nullptr)
        return 0;
      if (_logSize + size > _limit) {
        _onEnd();
        _onEnd = nullptr;
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
      return write(&c, 1);
    }

  private:
    const char* _path;
    const size_t _limit;
    std::function<void()> _onEnd;
    size_t _logSize = 0;
};

static Mycila::Task* loggingTask = nullptr;
static WebSerial* webSerial = nullptr;
static LogStream* logStream = nullptr;

static int log_redirect_vprintf(const char* format, va_list args) {
  size_t written = Serial.vprintf(format, args);
  if (logStream != nullptr)
    logStream->vprintf(format, args);
  if (webSerial != nullptr)
    webSerial->vprintf(format, args);
  return written;
}

void yasolr_init_console_logging() {
  Serial.begin(YASOLR_SERIAL_BAUDRATE);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_set_vprintf(log_redirect_vprintf);
  LOGI(TAG, "Logging initialized");
}

void yasolr_init_startup_logging() {
  LOGI(TAG, "Saving startup logs...");

  if (LittleFS.remove(YASOLR_LOG_FILE))
    LOGI(TAG, "Previous log file removed");

  LOGI(TAG, "Redirecting logs to " YASOLR_LOG_FILE);
  logStream = new LogStream(YASOLR_LOG_FILE, 16 * 1024, []() {
    delete logStream;
    logStream = nullptr;
    LOGW(TAG, "Startup log size limit reached!");
  });

  LOGI(TAG, "Redirecting logs to WebSerial...");
  webSerial = new WebSerial();
#ifdef APP_MODEL_PRO
  webSerial->setID(Mycila::AppInfo.firmware.c_str());
  webSerial->setTitle((Mycila::AppInfo.name + " Web Console").c_str());
  webSerial->setInput(false);
#endif
  webSerial->setBuffer(256); // max log line size
  webSerial->begin(&webServer, "/console");
}

void yasolr_configure_logging() {
  if (config.getBool(KEY_ENABLE_DEBUG)) {
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_log_level_set("ARDUINO", ESP_LOG_DEBUG);
    LOGI(TAG, "Debug logging enabled");

    if (loggingTask == nullptr) {
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
    }

  } else {
    esp_log_level_set("*", ESP_LOG_INFO);
    if (loggingTask != nullptr) {
      unsafeTaskManager.removeTask(*loggingTask);
      delete loggingTask;
      loggingTask = nullptr;
    }
    LOGI(TAG, "Debug logging disabled");
  }
}
