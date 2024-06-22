// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include "MycilaJSYClientUDP.h"

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

#define TAG "JSY-UDP"

bool Mycila::JSYClientUDP::begin(JSY& jsy, const String& address, uint16_t port) {
  LOGI(TAG, "Connecting JSY UDP Client to %s:%d...", address.c_str(), port);

  if (!_udp.connect(IPAddress(address.c_str()), port)) {
    LOGE(TAG, "Failed to connect to %s:%d", address.c_str(), port);
    return false;
  }

  LOGI(TAG, "Connected!");

  jsy.setCallback([](const JSYEventType type) {
    if (type == JSYEventType::EVT_READ) {
    }
  });

  _enabled = true;
  return true;
}

void Mycila::JSYClientUDP::end() {
  LOGI(TAG, "Disconnecting JSY UDP Client...");
  _udp.close();
}
