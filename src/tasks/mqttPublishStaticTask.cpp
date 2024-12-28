// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

Mycila::Task mqttPublishStaticTask("MQTT Static", Mycila::TaskType::ONCE, [](void* params) {
  logger.debug(TAG, "Publishing static data to MQTT");
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  mqtt.publish(baseTopic + "/system/app/manufacturer", Mycila::AppInfo.manufacturer, true);
  mqtt.publish(baseTopic + "/system/app/model", Mycila::AppInfo.model, true);
  mqtt.publish(baseTopic + "/system/app/name", Mycila::AppInfo.name, true);
  mqtt.publish(baseTopic + "/system/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial), true);
  mqtt.publish(baseTopic + "/system/app/version", Mycila::AppInfo.version, true);
  yield();

  mqtt.publish(baseTopic + "/system/device/boots", std::to_string(Mycila::System::getBootCount()), true);
  mqtt.publish(baseTopic + "/system/device/cores", std::to_string(ESP.getChipCores()), true);
  mqtt.publish(baseTopic + "/system/device/cpu_freq", std::to_string(ESP.getCpuFreqMHz()), true);
  mqtt.publish(baseTopic + "/system/device/id", Mycila::AppInfo.id, true);
  mqtt.publish(baseTopic + "/system/device/model", ESP.getChipModel(), true);
  yield();

  mqtt.publish(baseTopic + "/system/firmware/build/branch", Mycila::AppInfo.buildBranch, true);
  mqtt.publish(baseTopic + "/system/firmware/build/hash", Mycila::AppInfo.buildHash, true);
  mqtt.publish(baseTopic + "/system/firmware/build/timestamp", Mycila::AppInfo.buildDate, true);
  mqtt.publish(baseTopic + "/system/firmware/debug", YASOLR_BOOL(Mycila::AppInfo.debug), true);
  mqtt.publish(baseTopic + "/system/firmware/filename", Mycila::AppInfo.firmware, true);
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/mac_address", espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH), true);
  mqtt.publish(baseTopic + "/system/network/hostname", espConnect.getHostname(), true);
  mqtt.publish(baseTopic + "/system/network/wifi/mac_address", espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA), true);
  yield();
});
