// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaThrottle.h>

#include <U8g2lib.h>
#include <WString.h>

#define MYCILA_DISPLAY_LINE_COUNT 6
#define MYCILA_DISPLAY_LINE_LENGTH 32

namespace Mycila {
  enum DisplayType {
    SH1106 = 0,
    SH1107,
    SSD1306
  };

  class DisplayConfigClass {
    public:
      bool isEnabled() const;
      DisplayType getType() const;
      int32_t getClockPin() const;
      int32_t getDataPin() const;
      uint32_t getRotation() const;
      uint32_t getPowerSaveDelay() const;
  };

  class DisplayClass {
    public:
      ~DisplayClass() { end(); }

      void begin();
      void end();
      void loop();

      void setActive(bool active);
      // Up to 6 lines supported, 25 chars long
      void print(const char lines[MYCILA_DISPLAY_LINE_COUNT][MYCILA_DISPLAY_LINE_LENGTH]);

      bool isEnabled() const { return _enabled; }
      // true if on, false if off or power saving
      bool isActive() const { return _active; }

      gpio_num_t getClockPin() const { return _clkPin; }
      gpio_num_t getDataPin() const { return _dataPin; }

    private:
      bool _enabled = false;
      bool _active = false;
      DisplayType _display_type = DisplayType::SH1106;
      U8G2* _display;
      Throttle _powerSave = Throttle(true);
      gpio_num_t _clkPin = GPIO_NUM_NC;
      gpio_num_t _dataPin = GPIO_NUM_NC;
      char _lines[MYCILA_DISPLAY_LINE_COUNT][MYCILA_DISPLAY_LINE_LENGTH];
  };

  extern DisplayConfigClass DisplayConfig;
  extern DisplayClass Display;
} // namespace Mycila
