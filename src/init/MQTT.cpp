// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

Mycila::Task initMqttSubscribersTask("Init MQTT Subscribers", [](void* params) {
  logger.info(TAG, "Initializing MQTT Subscribers...");

  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  // config

  mqtt.subscribe(baseTopic + "/config/+/set", [](const String& topic, const String& payload) {
    const int end = topic.lastIndexOf("/");
    const int start = topic.lastIndexOf("/", end - 1);
    const String key = topic.substring(start + 1, end);
    const char* keyRef = config.keyRef(key.c_str());
    if (keyRef)
      config.set(keyRef, payload);
  });

  // relays

  mqtt.subscribe(baseTopic + "/router/relay1/state/set", [](const String& topic, const String& payload) {
    if (relay1.isEnabled()) {
      int start = payload.indexOf("=");
      String state = start >= 0 ? payload.substring(0, start) : payload;
      uint32_t duration = start >= 0 ? payload.substring(start + 1).toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState("relay1", true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState("relay1", false, duration);
    }
  });
  mqtt.subscribe(baseTopic + "/router/relay2/state/set", [](const String& topic, const String& payload) {
    if (relay2.isEnabled()) {
      int start = payload.indexOf("=");
      String state = start >= 0 ? payload.substring(0, start) : payload;
      uint32_t duration = start >= 0 ? payload.substring(start + 1).toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState("relay2", true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState("relay2", false, duration);
    }
  });

  // router

  mqtt.subscribe(baseTopic + "/router/output1/dimmer/level/set", [](const String& topic, const String& payload) {
    output1.tryDimmerLevel(payload.toInt());
  });

  mqtt.subscribe(baseTopic + "/router/output2/dimmer/level/set", [](const String& topic, const String& payload) {
    output2.tryDimmerLevel(payload.toInt());
  });

  mqtt.subscribe(baseTopic + "/router/output1/bypass/set", [](const String& topic, const String& payload) {
    if (output1.isBypassRelayEnabled()) {
      if (payload == YASOLR_ON)
        output1.tryBypassRelayState(true);
      else if (payload == YASOLR_OFF)
        output1.tryBypassRelayState(false);
    }
  });

  mqtt.subscribe(baseTopic + "/router/output2/bypass/set", [](const String& topic, const String& payload) {
    if (output2.isBypassRelayEnabled()) {
      if (payload == YASOLR_ON)
        output2.tryBypassRelayState(true);
      else if (payload == YASOLR_OFF)
        output2.tryBypassRelayState(false);
    }
  });

  // device

  mqtt.subscribe(baseTopic + "/system/device/restart", [](const String& topic, const String& payload) {
    restartTask.resume();
  });

  // grid power
  String gridPowerMQTTTopic = config.get(KEY_GRID_POWER_MQTT_TOPIC);
  if (!gridPowerMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Grid Power from MQTT topic: %s", gridPowerMQTTTopic.c_str());
    mqtt.subscribe(gridPowerMQTTTopic.c_str(), [](const String& topic, const String& payload) {
      float p = payload.toFloat();
      logger.debug(TAG, "MQTT Grid Power: %f", p);
      grid.updatePower(p);
      Mycila::Router.adjustRouting();
    });
  }

  // grid voltage
  String gridVoltageMQTTTopic = config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  if (!gridVoltageMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Grid Voltage from MQTT topic: %s", gridVoltageMQTTTopic.c_str());
    mqtt.subscribe(gridVoltageMQTTTopic.c_str(), [](const String& topic, const String& payload) {
      float v = payload.toFloat();
      logger.debug(TAG, "MQTT Grid Voltage: %f", v);
      grid.updateVoltage(v);
    });
  }
});
