// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

Mycila::Task initMqttSubscribersTask("Init MQTT Subscribers", [](void* params) {
  logger.info(TAG, "Initializing MQTT Subscribers");

  const std::string baseTopic = config.get(KEY_MQTT_TOPIC);

  // config

  mqtt.subscribe((baseTopic + "/config/+/set").c_str(), [](const std::string& topic, const std::string& payload) {
    const std::size_t end = topic.rfind("/set");
    if (end == std::string::npos)
      return;
    const std::size_t start = topic.rfind("/", end - 1);
    const char* key = config.keyRef(topic.substr(start + 1, end - start - 1).c_str());
    if (key)
      config.set(key, payload.c_str());
  });

  // relays

  mqtt.subscribe((baseTopic + "/router/relay1/set").c_str(), [](const std::string& topic, const std::string& payload) {
    if (relay1.isEnabled()) {
      const std::size_t eq = payload.find("=");
      std::string state = payload.substr(0, eq);
      if (state.empty())
        return;
      uint32_t duration = strtoul(payload.c_str() + eq + 1, nullptr, 10);

      if (state == YASOLR_ON)
        routerRelay1.tryRelayState(true, duration);
      else if (state == YASOLR_OFF)
        routerRelay1.tryRelayState(false, duration);
    }
  });

  mqtt.subscribe((baseTopic + "/router/relay2/set").c_str(), [](const std::string& topic, const std::string& payload) {
    if (relay2.isEnabled()) {
      const std::size_t eq = payload.find("=");
      std::string state = payload.substr(0, eq);
      if (state.empty())
        return;
      uint32_t duration = strtoul(payload.c_str() + eq + 1, nullptr, 10);

      if (state == YASOLR_ON)
        routerRelay2.tryRelayState(true, duration);
      else if (state == YASOLR_OFF)
        routerRelay2.tryRelayState(false, duration);
    }
  });

  // router

  mqtt.subscribe((baseTopic + "/router/output1/duty_cycle/set").c_str(), [](const std::string& topic, const std::string& payload) {
    output1.setDimmerDutyCycle(std::stof(payload) / 100);
  });

  mqtt.subscribe((baseTopic + "/router/output2/duty_cycle/set").c_str(), [](const std::string& topic, const std::string& payload) {
    output2.setDimmerDutyCycle(std::stof(payload) / 100);
  });

  mqtt.subscribe((baseTopic + "/router/output1/bypass/set").c_str(), [](const std::string& topic, const std::string& payload) {
    if (output1.isBypassEnabled()) {
      if (payload == YASOLR_ON)
        output1.setBypassOn();
      else if (payload == YASOLR_OFF)
        output1.setBypassOff();
    }
  });

  mqtt.subscribe((baseTopic + "/router/output2/bypass/set").c_str(), [](const std::string& topic, const std::string& payload) {
    if (output2.isBypassEnabled()) {
      if (payload == YASOLR_ON)
        output2.setBypassOn();
      else if (payload == YASOLR_OFF)
        output2.setBypassOff();
    }
  });

  // device

  mqtt.subscribe((baseTopic + "/system/device/restart").c_str(), [](const std::string& topic, const std::string& payload) {
    restartTask.resume();
  });

  // grid power
  String gridPowerMQTTTopic = config.get(KEY_GRID_POWER_MQTT_TOPIC);
  if (!gridPowerMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Grid Power from MQTT topic: %s", gridPowerMQTTTopic.c_str());
    mqtt.subscribe(gridPowerMQTTTopic.c_str(), [](const std::string& topic, const std::string& payload) {
      float p = std::stof(payload);
      logger.debug(TAG, "Grid Power from MQTT: %f", p);
      grid.mqttPower().update(p);
      if (grid.updatePower())
        routingTask.resume();
    });
  }

  // grid voltage
  String gridVoltageMQTTTopic = config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  if (!gridVoltageMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Grid Voltage from MQTT topic: %s", gridVoltageMQTTTopic.c_str());
    mqtt.subscribe(gridVoltageMQTTTopic.c_str(), [](const std::string& topic, const std::string& payload) {
      float v = std::stof(payload);
      logger.debug(TAG, "Grid Voltage from MQTT: %f", v);
      grid.mqttVoltage().update(v);
    });
  }

  // output 1 temperature
  String output1TemperatureMQTTTopic = config.get(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  if (!output1TemperatureMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Output 1 Temperature from MQTT topic: %s", output1TemperatureMQTTTopic.c_str());
    mqtt.subscribe(output1TemperatureMQTTTopic.c_str(), [](const std::string& topic, const std::string& payload) {
      float t = std::stof(payload);
      logger.debug(TAG, "Output 1 Temperature from MQTT: %f", t);
      output1.temperature().update(t);
    });
  }

  // output 2 temperature
  String output2TemperatureMQTTTopic = config.get(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  if (!output2TemperatureMQTTTopic.isEmpty()) {
    logger.info(TAG, "Reading Output 2 Temperature from MQTT topic: %s", output2TemperatureMQTTTopic.c_str());
    mqtt.subscribe(output2TemperatureMQTTTopic.c_str(), [](const std::string& topic, const std::string& payload) {
      float t = std::stof(payload);
      logger.debug(TAG, "Output 2 Temperature from MQTT: %f", t);
      output2.temperature().update(t);
    });
  }
});
