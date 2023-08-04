// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

void YaSolR::YaSolRClass::_initMQTT() {
  Mycila::Logger.info(TAG, "Initializing MQTT API...");

  const String baseTopic = Mycila::Config.get(KEY_MQTT_TOPIC);

  // config

  Mycila::MQTT.subscribe(baseTopic + "/config/+/set", [](const String& topic, const String& payload) {
    const int end = topic.lastIndexOf("/");
    const int start = topic.lastIndexOf("/", end - 1);
    const String key = topic.substring(start + 1, end);
    const char* keyRef = Mycila::Config.keyRef(key.c_str());
    if (keyRef)
      Mycila::Config.set(keyRef, payload);
  });

  // relays

  Mycila::MQTT.subscribe(baseTopic + "/relays/" NAME_RELAY1 "/state/set", [this](const String& topic, const String& payload) {
    if (Mycila::RelayManager.relays[0].isEnabled()) {
      int start = payload.indexOf("=");
      String state = start >= 0 ? payload.substring(0, start) : payload;
      uint32_t duration = start >= 0 ? payload.substring(start + 1).toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(0, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(0, false, duration);
    }
  });
  Mycila::MQTT.subscribe(baseTopic + "/relays/" NAME_RELAY2 "/state/set", [this](const String& topic, const String& payload) {
    if (Mycila::RelayManager.relays[1].isEnabled()) {
      int start = payload.indexOf("=");
      String state = start >= 0 ? payload.substring(0, start) : payload;
      uint32_t duration = start >= 0 ? payload.substring(start + 1).toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(1, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(1, false, duration);
    }
  });

  // router

  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT1 "/dimmer/level/set", [](const String& topic, const String& payload) {
    Mycila::Router.outputs[0].tryDimmerLevel(payload.toInt());
  });

  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT2 "/dimmer/level/set", [](const String& topic, const String& payload) {
    Mycila::Router.outputs[1].tryDimmerLevel(payload.toInt());
  });

  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT1 "/bypass_relay/state/set", [](const String& topic, const String& payload) {
    if (Mycila::Router.outputs[0].isBypassRelayEnabled()) {
      if (payload == YASOLR_ON)
        Mycila::Router.outputs[0].tryBypassRelayState(true);
      else if (payload == YASOLR_OFF)
        Mycila::Router.outputs[0].tryBypassRelayState(false);
    }
  });

  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT2 "/bypass_relay/state/set", [](const String& topic, const String& payload) {
    if (Mycila::Router.outputs[1].isBypassRelayEnabled()) {
      if (payload == YASOLR_ON)
        Mycila::Router.outputs[1].tryBypassRelayState(true);
      else if (payload == YASOLR_OFF)
        Mycila::Router.outputs[1].tryBypassRelayState(false);
    }
  });

  // system

  Mycila::MQTT.subscribe(baseTopic + "/system/restart", [=](const String& topic, const String& payload) {
    restartTask.resume();
  });

  Mycila::MQTT.subscribe(baseTopic + "/system/reset", [=](const String& topic, const String& payload) {
    resetTask.resume();
  });
}
