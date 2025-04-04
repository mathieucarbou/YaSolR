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

bool Mycila::RouterRelay::autoSwitch(float gridPower, float routedPower, float setpoint) {
  if (!isAutoRelayEnabled())
    return false;

  // * setpoint is the grid power to be reached, configured in the PID section. Example 1: 1000W | Example 2: 0W
  // * routedPower is the power routed to the load through the dimmer. Example1: 800W | Example 2: 800W
  // * gridPower is the power coming from or going to the grid. Example1: 1000W | Example 2: 0W (800W routing + 200W home consumption)
  // * virtualGridPower is the power that would come from or go to the grid if the routing was off. Example1: 200W | Example 2: -800W
  // * relayRoomPower is the power that is available for a relay. Example1: 800W | Example 2: 800W

  if (_relay->isOff()) {
    const float relayRoomPower = setpoint + routedPower - gridPower;
    if (relayRoomPower >= _load * (1.0f + MYCILA_RELAY_TOLERANCE)) {
      LOGI(TAG, "Auto-Switching relay on pin %u ON. Grid power: %.2f W, routed power: %.2f W, setpoint: %.2f W", _relay->getPin(), gridPower, routedPower, setpoint);
      _relay->setState(true);
      return true;
    }
    return false;
  }

  if (_relay->isOn()) {
    const float relayRoomPower = setpoint + routedPower - gridPower + _load;
    if (relayRoomPower <= _load * (1.0f - MYCILA_RELAY_TOLERANCE)) {
      LOGI(TAG, "Auto-Switching relay on pin %u OFF. Grid power: %.2f W, routed power: %.2f W, setpoint: %.2f W", _relay->getPin(), gridPower, routedPower, setpoint);
      _relay->setState(false);
      return true;
    }
    return false;
  }

  return false;
}
