// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaButton.h>
#include <MycilaLogger.h>

#define TAG "BUTTON"

void Mycila::ButtonClass::begin() {
  if (_enabled)
    return;

  if (!ButtonConfig.isEnabled())
    return;

  int32_t pin = ButtonConfig.getPin();
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
  _button->attachLongPressHoldEventCb(
    +[](void* handle, void* data) {
      Mycila::ButtonClass* self = (Mycila::ButtonClass*)data;
      self->_lastEvent = ButtonEvent::BUTTON_LONG_PRESS_HOLD;
    },
    this);
  _button->attachSingleClickEventCb(
    +[](void* handle, void* data) {
      Mycila::ButtonClass* self = (Mycila::ButtonClass*)data;
      self->_lastEvent = ButtonEvent::BUTTON_CLICKED;
    },
    this);

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
}

void Mycila::ButtonClass::loop() {
  if (!_enabled)
    return;
  if (_lastEvent == ButtonEvent::BUTTON_CLICKED) {
    _lastEvent = ButtonEvent::IDLE;
    _callback(ButtonConfig.getPressAction());
  } else if (_lastEvent == ButtonEvent::BUTTON_LONG_PRESS_HOLD) {
    _lastEvent = ButtonEvent::IDLE;
    _callback(ButtonConfig.getLongPressAction());
  }
}

void Mycila::ButtonClass::toJson(const JsonObject& root) const {
  root["enabled"] = _enabled;
}

namespace Mycila {
  ButtonClass Button;
  ButtonConfigClass ButtonConfig;
} // namespace Mycila
