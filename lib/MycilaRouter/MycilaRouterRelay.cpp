// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
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
  // => 50W for a tri-phase 3000W resistance (1000W per phase)
  // => 35W for a tri-phase 2100W resistance (700W per phase)
  #define MYCILA_RELAY_TOLERANCE 0.05f
#endif

#define TAG "RELAY"

bool Mycila::RouterRelay::trySwitchRelay(bool state, uint32_t duration) {
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

bool Mycila::RouterRelay::autoSwitch(float virtualGridPower) {
  if (!isAutoRelayEnabled())
    return false;

  if (_relay->isOff()) {
    if (virtualGridPower + _load <= -_load * MYCILA_RELAY_TOLERANCE) {
      LOGI(TAG, "Auto-Switching relay on pin %u ON: virtual grid power is %.2f W", _relay->getPin(), virtualGridPower);
      _relay->setState(true);
      return true;
    }
    return false;
  }

  if (_relay->isOn()) {
    if (virtualGridPower >= _load * MYCILA_RELAY_TOLERANCE) {
      LOGI(TAG, "Auto-Switching relay on pin %u OFF: virtual grid power is %.2f W", _relay->getPin(), virtualGridPower);
      _relay->setState(false);
      return true;
    }
    return false;
  }

  return false;
}
