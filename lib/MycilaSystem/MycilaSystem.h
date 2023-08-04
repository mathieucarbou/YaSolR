// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <esp_timer.h>
#include <functional>

namespace Mycila {
  enum class SystemEvent {
    BEFORE_RESET,
    BEFORE_RESTART,
  };

  typedef std::function<void(SystemEvent event)> SystemEventCallback;

  typedef struct {
      uint32_t total;
      uint32_t used;
      uint32_t free;
      float usage;
  } SystemMemory;

  class SystemClass {
    public:
      void begin();

      void listen(SystemEventCallback callback) { _callback = callback; }

      void reset();
      void restart();

      inline int64_t getUptime() const { return esp_timer_get_time() / (int64_t)1000000; }
      const SystemMemory getMemory() const;
      uint32_t getBootCount() const { return _boots; }

      void toJson(const JsonObject& root) const;

    private:
      uint32_t _boots = 0;

    private:
      SystemEventCallback _callback = nullptr;
  };

  extern SystemClass System;
} // namespace Mycila
