// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaLights.h>
#include <MycilaLogger.h>

#define TAG "LIGHTS"

void Mycila::LightsClass::begin(const uint8_t greenPin, const uint8_t yellowPin, const uint8_t redPin) {
  Logger.info(TAG, "Enable Physical LEDs...");

  if (GPIO_IS_VALID_OUTPUT_GPIO(greenPin)) {
    _greenPin = (gpio_num_t)greenPin;
    Logger.debug(TAG, "- Pin: %u (green)", _greenPin);
    pinMode(_greenPin, OUTPUT);
  } else {
    Logger.error(TAG, "Invalid pin: %u", _greenPin);
    _greenPin = GPIO_NUM_NC;
  }

  if (GPIO_IS_VALID_OUTPUT_GPIO(yellowPin)) {
    _yellowPin = (gpio_num_t)yellowPin;
    Logger.debug(TAG, "- Pin: %u (yellow)", _yellowPin);
    pinMode(_yellowPin, OUTPUT);
  } else {
    Logger.error(TAG, "Invalid pin: %u", _yellowPin);
    _yellowPin = GPIO_NUM_NC;
  }

  if (GPIO_IS_VALID_OUTPUT_GPIO(redPin)) {
    _redPin = (gpio_num_t)redPin;
    Logger.debug(TAG, "- Pin: %u (red)", _redPin);
    pinMode(_redPin, OUTPUT);
  } else {
    Logger.error(TAG, "Invalid pin: %u", _redPin);
    _redPin = GPIO_NUM_NC;
  }
}

void Mycila::LightsClass::end() {
  set(LightState::OFF, LightState::OFF, LightState::OFF);
  if (_greenPin != GPIO_NUM_NC)
    digitalWrite(_greenPin, LOW);
  if (_yellowPin != GPIO_NUM_NC)
    digitalWrite(_yellowPin, LOW);
  if (_redPin != GPIO_NUM_NC)
    digitalWrite(_redPin, LOW);
  _greenPin = GPIO_NUM_NC;
  _yellowPin = GPIO_NUM_NC;
  _redPin = GPIO_NUM_NC;
}

void Mycila::LightsClass::set(LightState green, LightState yellow, LightState red) {
  if (green != LightState::NONE)
    _green = green == LightState::ON;
  if (yellow != LightState::NONE)
    _yellow = yellow == LightState::ON;
  if (red != LightState::NONE)
    _red = red == LightState::ON;
  if (_greenPin != GPIO_NUM_NC)
    digitalWrite(_greenPin, _green ? HIGH : LOW);
  if (_yellowPin != GPIO_NUM_NC)
    digitalWrite(_yellowPin, _yellow ? HIGH : LOW);
  if (_redPin != GPIO_NUM_NC)
    digitalWrite(_redPin, _red ? HIGH : LOW);
}

void Mycila::LightsClass::toJson(const JsonObject& root) const {
  root["code"] = toString();
  root["green"] = _green ? "on" : "off";
  root["red"] = _red ? "on" : "off";
  root["yellow"] = _yellow ? "on" : "off";
}

namespace Mycila {
  LightsClass Lights;
} // namespace Mycila
