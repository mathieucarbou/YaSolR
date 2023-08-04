// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaButton.h>
#include <MycilaLogger.h>

#define TAG "BUTTON"

void Mycila::ButtonClass::begin(const uint8_t pin) {
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

  _button->attachSingleClickEventCb(
    +[](void* handle, void* data) {
      Mycila::ButtonClass* self = (Mycila::ButtonClass*)data;
      if (self->_callback) {
        self->_callback(ButtonEvent::BUTTON_CLICKED);
      }
    },
    this);

  _button->attachLongPressHoldEventCb(
    +[](void* handle, void* data) {
      Mycila::ButtonClass* self = (Mycila::ButtonClass*)data;
      if (self->_callback) {
        self->_callback(ButtonEvent::BUTTON_LONG_PRESS_HOLD);
      }
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

namespace Mycila {
  ButtonClass Button;
} // namespace Mycila
