// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#define LOG_STREAM_FILE_SIZE 24 * 1024

#include <memory>

class LogStream : public Print {
  public:
    LogStream() {
      File file = LittleFS.open(YASOLR_LOG_FILE, "r");
      _fileSize = file ? file.size() : 0;
      file.close();
    }

    size_t write(uint8_t c) override {
      return write(&c, 1);
    }

    size_t write(const uint8_t* buffer, size_t size) override {
      if (_fileSize > LOG_STREAM_FILE_SIZE)
        return 0;
      File file = LittleFS.open(YASOLR_LOG_FILE, "a");
      if (!file || !size)
        return 0;
      const size_t written = file.write(buffer, size);
      _fileSize += written;
      if (buffer[size - 1] == '\n') {
        const std::string timestamp = Mycila::Time::getLocalStr(); // NOLINT
        _fileSize += file.write((const uint8_t*)timestamp.c_str(), timestamp.length());
        _fileSize += file.write(' ');
      }
      file.close();
      return written;
    }

  private:
    size_t _fileSize = 0;
};

static Mycila::Task* loggingTask = nullptr;
static LogStream* logStream = nullptr;
static bool alreadyLogging = false;

static int log_redirect_vprintf(const char* format, va_list args) {
  if (alreadyLogging) {
    // prevent re-entry in WebSerial when we have some logs in esp_async_ws like WS message queue overflow
    Serial.println("WARNING: Re-entry prevented in log_redirect_vprintf");
    return Serial.vprintf(format, args);
  } else {
    alreadyLogging = true;
    size_t written = Serial.vprintf(format, args);
    if (logStream != nullptr) logStream->vprintf(format, args);
    if (WebSerial.getConnectionCount()) WebSerial.vprintf(format, args);
    alreadyLogging = false;
    return written;
  }
}

static void set_debug_levels() {
  esp_log_level_set("*", ESP_LOG_DEBUG);
  esp_log_level_set("esp_core_dump_elf", ESP_LOG_INFO);
  esp_log_level_set("esp_core_dump_port", ESP_LOG_INFO);
  esp_log_level_set("esp_netif_lwip", ESP_LOG_INFO);
  esp_log_level_set("nvs", ESP_LOG_INFO);
  esp_log_level_set("ARDUINO", ESP_LOG_DEBUG);
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

  ESP_LOGI(TAG, "Initialize logging");
  set_debug_levels();
  esp_log_set_vprintf(log_redirect_vprintf);
}

void yasolr_init_persistent_logging() {
  if (config.get<bool>(KEY_ENABLE_DEBUG_BOOT)) {
    ESP_LOGI(TAG, "Enable Persistent Logging for this boot session");

    if (LittleFS.remove(YASOLR_LOG_FILE))
      ESP_LOGI(TAG, "Previous log file removed");

    logStream = new LogStream();
  }
}

void yasolr_configure_logging() {
  if (config.get<bool>(KEY_ENABLE_DEBUG)) {
    ESP_LOGI(TAG, "Enable Debug Mode");

    set_debug_levels();

    if (loggingTask == nullptr) {
      loggingTask = new Mycila::Task("Debug", []() {
        std::unique_ptr<Mycila::System::Memory> memory = std::make_unique<Mycila::System::Memory>();
        Mycila::System::getMemory(*memory);
        ESP_LOGI(TAG, "HEAP: Total: %" PRIu32 ", Used: %" PRIu32 ", Free: %" PRIu32 ", MinFree: %" PRIu32, memory->total, memory->used, memory->free, memory->minimumFree);
        Mycila::TaskMonitor.log();
        coreTaskManager.log();
        unsafeTaskManager.log();
        if (jsyTaskManager)
          jsyTaskManager->log();
      });

      loggingTask->setInterval(30000);
      loggingTask->enableProfiling();

      unsafeTaskManager.addTask(*loggingTask);
    }

  } else {
    ESP_LOGI(TAG, "Disable Debug Mode");

    esp_log_level_set("*", ESP_LOG_INFO);

    // prevents loop with WebSerial in case we reach some error logs in esp_async_ws like WS message queue overflow
    esp_log_level_set("async_tcp", ESP_LOG_NONE);
    esp_log_level_set("async_ws", ESP_LOG_NONE);

    if (loggingTask != nullptr) {
      unsafeTaskManager.removeTask(*loggingTask);
      delete loggingTask;
      loggingTask = nullptr;
    }
  }
}
