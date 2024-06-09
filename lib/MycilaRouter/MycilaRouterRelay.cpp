// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRouterRelay.h>

#ifdef MYCILA_LOGGER_SUPPORT
  #include <MycilaLogger.h>
extern Mycila::Logger logger;
  #define LOGD(tag, format, ...) logger.debug(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) logger.info(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) logger.warn(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) logger.error(tag, format, ##__VA_ARGS__)
#else
  #define LOGD(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

#ifndef MYCILA_RELAY_TOLERANCE
  // in percentage
  // => 30W for a tri-phase 3000W resistance
  // => 21W for a tri-phase 2100W resistance
  #define MYCILA_RELAY_TOLERANCE 0.03
#endif

#define TAG "RELAY"

bool Mycila::RouterRelay::tryRelayState(bool state, uint32_t duration) {
  if (!_relay->isEnabled())
    return false;

  if (isAutoRelayEnabled()) {
    LOGW(TAG, "Relay on pin %u cannot be activated because it is connected to a load of %" PRIu16 "W", _relay->getPin(), _load);
    return false;
  }

  if (duration)
    LOGI(TAG, "Switching relay on pin %u %s for %u ms", _relay->getPin(), state ? "ON" : "OFF", duration);
  else
    LOGI(TAG, "Switching relay on pin %u %s", _relay->getPin(), state ? "ON" : "OFF");
  _relay->setState(state, duration);
  return true;
}

bool Mycila::RouterRelay::tryRelayStateAuto(bool state, float virtualGridPower) {
  if (!isAutoRelayEnabled())
    return false;

  if (state && _relay->isOff() && virtualGridPower + _load <= -_load * MYCILA_RELAY_TOLERANCE) {
    LOGI(TAG, "Auto-Switching relay on pin %u ON: virtual grid power is %.2f W", _relay->getPin(), virtualGridPower);
    _relay->setState(true);
    return true;
  }

  if (!state && _relay->isOn() && virtualGridPower >= _load * MYCILA_RELAY_TOLERANCE) {
    LOGI(TAG, "Auto-Switching relay on pin %u OFF: virtual grid power is %.2f W", _relay->getPin(), virtualGridPower);
    _relay->setState(false);
    return true;
  }

  return false;
}
