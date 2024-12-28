// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

Mycila::Task mqttPublishConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  logger.debug(TAG, "Publishing config to MQTT");
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  for (auto& key : config.keys()) {
    const char* value = config.get(key);
    if (value[0] != '\0' && config.isPasswordKey(key))
      value = "********";
    mqtt.publish(baseTopic + "/config/" + key, value, true);
  }
});
