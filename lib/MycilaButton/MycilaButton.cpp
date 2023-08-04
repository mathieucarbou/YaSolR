// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaButton.h>
#include <MycilaLogger.h>

#define TAG "BUTTON"

void Mycila::ButtonClass::begin(const uint8_t pin, const String& pressAction, const String& longPressAction) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    Logger.error(TAG, "Invalid pin: %u", _pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  Logger.info(TAG, "Enable Push Button...");
  Logger.debug(TAG, "- Pin: %u", _pin);

  _button = new ::Button(_pin, false);
  const int32_t duration = 8000;
  _button->setParam(button_param_t::BUTTON_LONG_PRESS_TIME_MS, reinterpret_cast<void*>(duration));

  _pressAction = pressAction;
  _longPressAction = longPressAction;

  if (!_pressAction.isEmpty()) {
    _button->attachSingleClickEventCb(
      +[](void* handle, void* data) {
        Mycila::ButtonClass* self = (Mycila::ButtonClass*)data;
        self->_lastEvent = ButtonEvent::BUTTON_CLICKED;
      },
      this);
  }

  if (!_longPressAction.isEmpty()) {
    _button->attachLongPressHoldEventCb(
      +[](void* handle, void* data) {
        Mycila::ButtonClass* self = (Mycila::ButtonClass*)data;
        self->_lastEvent = ButtonEvent::BUTTON_LONG_PRESS_HOLD;
      },
      this);
  }

  _enabled = true;
}

void Mycila::ButtonClass::end() {
  if (!_enabled)
    return;
  Logger.info(TAG, "Disable Push Button...");
  _enabled = false;
  _button->del();
  delete _button;
  _button = nullptr;
  _pin = GPIO_NUM_NC;
  _lastEvent = ButtonEvent::IDLE;
}

void Mycila::ButtonClass::processEvents() {
  if (!_enabled)
    return;
  ButtonEvent evt = _lastEvent;
  _lastEvent = ButtonEvent::IDLE;
  switch (evt) {
    case ButtonEvent::BUTTON_CLICKED:
      _callback(_pressAction);
      break;
    case ButtonEvent::BUTTON_LONG_PRESS_HOLD:
      _callback(_longPressAction);
      break;
    default:
      break;
  }
}

void Mycila::ButtonClass::toJson(const JsonObject& root) const {
  root["enabled"] = _enabled;
}

namespace Mycila {
  ButtonClass Button;
} // namespace Mycila
