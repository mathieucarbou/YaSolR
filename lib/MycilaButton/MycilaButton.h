// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ArduinoJson.h>
#include <Button.h>
#include <WString.h>
#include <functional>

namespace Mycila {
  enum class ButtonEvent {
    IDLE,
    BUTTON_CLICKED,
    BUTTON_LONG_PRESS_HOLD
  };

  typedef std::function<void(const String& action)> ButtonActionCallback;

  class ButtonConfigClass {
    public:
      int32_t getPin() const;
      bool isEnabled() const;
      String getPressAction() const;
      String getLongPressAction() const;
  };

  class ButtonClass {
    public:
      ~ButtonClass() { end(); }

      void begin();
      void loop();
      void end();

      void toJson(const JsonObject& root) const;

      void listen(ButtonActionCallback callback) { _callback = callback; }
      bool isEnabled() const { return _enabled; }
      gpio_num_t getPin() const { return _pin; }

    private:
      bool _enabled = false;
      ::Button* _button = nullptr;
      volatile ButtonEvent _lastEvent = ButtonEvent::IDLE;
      ButtonActionCallback _callback;
      gpio_num_t _pin = GPIO_NUM_NC;
  };

  extern ButtonConfigClass ButtonConfig;
  extern ButtonClass Button;
} // namespace Mycila
