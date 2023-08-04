// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <esp32-hal-timer.h>

namespace Mycila {
  class Throttle {
    public:
      explicit Throttle(bool enabled = false, bool touch = true) { setEnable(enabled, touch); };

      void setEnable(bool enabled, bool touch = true) {
        _enabled = enabled;
        if (enabled && touch)
          _last = millis();
      }
      bool isEnabled() const { return _enabled; }
      void touch() { _last = millis(); }
      void touch(uint32_t time) { _last = time; }

      // interval in seconds
      bool __attribute__((hot)) isAllowed(const uint32_t interval) {
        if (_enabled) {
          const uint32_t now = millis();
          if (now - _last >= interval * (uint32_t)1000) {
            _last = now;
            return true;
          }
        }
        return false;
      }

      bool __attribute__((hot)) isAllowedMillis(const uint32_t intervalMillis) {
        if (_enabled) {
          const uint32_t now = millis();
          if (now - _last >= intervalMillis) {
            _last = now;
            return true;
          }
        }
        return false;
      }

    private:
      uint32_t _last = 0;
      bool _enabled = false;
  };
} // namespace Mycila
