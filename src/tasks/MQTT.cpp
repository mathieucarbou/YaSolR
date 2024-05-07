// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task mqttPublishStaticTask("MQTT Static", Mycila::TaskType::ONCE, [](void* params) {
  logger.info("YASOLR", "Publishing static data to MQTT...");
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  mqtt.publish(baseTopic + "/system/app/name", Mycila::AppInfo.name, true);
  mqtt.publish(baseTopic + "/system/app/model", Mycila::AppInfo.model, true);
  mqtt.publish(baseTopic + "/system/app/version", Mycila::AppInfo.version, true);
  mqtt.publish(baseTopic + "/system/app/manufacturer", Mycila::AppInfo.manufacturer, true);
  mqtt.publish(baseTopic + "/system/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial), true);

  mqtt.publish(baseTopic + "/system/device/id", Mycila::AppInfo.id, true);
  mqtt.publish(baseTopic + "/system/device/model", ESP.getChipModel(), true);
  mqtt.publish(baseTopic + "/system/device/cores", String(chip_info.cores), true);
  mqtt.publish(baseTopic + "/system/device/cpu_freq", String(ESP.getCpuFreqMHz()), true);
  mqtt.publish(baseTopic + "/system/device/boots", String(Mycila::System.getBootCount()), true);
  mqtt.publish(baseTopic + "/system/device/heap/total", String(ESP.getHeapSize()), true);

  mqtt.publish(baseTopic + "/system/firmware/filename", Mycila::AppInfo.firmware, true);
  mqtt.publish(baseTopic + "/system/firmware/build/hash", Mycila::AppInfo.buildHash, true);
  mqtt.publish(baseTopic + "/system/firmware/build/branch", Mycila::AppInfo.buildBranch, true);
  mqtt.publish(baseTopic + "/system/firmware/build/timestamp", Mycila::AppInfo.buildDate, true);
  mqtt.publish(baseTopic + "/system/firmware/debug", YASOLR_BOOL(Mycila::AppInfo.debug), true);

  mqtt.publish(baseTopic + "/system/network/hostname", ESPConnect.getHostname(), true);
  mqtt.publish(baseTopic + "/system/network/eth/mac_address", ESPConnect.getMACAddress(ESPConnectMode::ETH), true);
  mqtt.publish(baseTopic + "/system/network/wifi/mac_address", ESPConnect.getMACAddress(ESPConnectMode::STA), true);

  if (!relay1.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay1/state", YASOLR_STATE(relay1.isOn()), true);
    mqtt.publish(baseTopic + "/router/relay1/switch_count", String(relay1.getSwitchCount()), true);
  }

  if (!relay2.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay2/state", YASOLR_STATE(relay2.isOn()), true);
    mqtt.publish(baseTopic + "/router/relay2/switch_count", String(relay2.getSwitchCount()), true);
  }

  if (!output1.isEnabled()) {
    mqtt.publish(baseTopic + "/router/output1/bypass/state", YASOLR_STATE(output1.isBypassOn()), true);
    mqtt.publish(baseTopic + "/router/output1/dimmer/level", String(dimmerO1.getLevel()), true);
    mqtt.publish(baseTopic + "/router/output1/dimmer/state", YASOLR_STATE(dimmerO1.isOn()), true);
    mqtt.publish(baseTopic + "/router/output1/relay/state", YASOLR_STATE(bypassRelayO1.isOn()), true);
    mqtt.publish(baseTopic + "/router/output1/relay/switch_count", String(bypassRelayO1.getSwitchCount()), true);
    mqtt.publish(baseTopic + "/router/output1/temperature", String(ds18O1.getLastTemperature()), true);
  }

  if (!output2.isEnabled()) {
    mqtt.publish(baseTopic + "/router/output2/bypass/state", YASOLR_STATE(output2.isBypassOn()), true);
    mqtt.publish(baseTopic + "/router/output2/dimmer/level", String(dimmerO2.getLevel()), true);
    mqtt.publish(baseTopic + "/router/output2/dimmer/state", YASOLR_STATE(dimmerO2.isOn()), true);
    mqtt.publish(baseTopic + "/router/output2/relay/state", YASOLR_STATE(bypassRelayO2.isOn()), true);
    mqtt.publish(baseTopic + "/router/output2/relay/switch_count", String(bypassRelayO2.getSwitchCount()), true);
    mqtt.publish(baseTopic + "/router/output2/temperature", String(ds18O2.getLastTemperature()), true);
  }
});

Mycila::Task mqttPublishConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  logger.info("YASOLR", "Publishing config to MQTT...");
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  for (auto& key : config.keys) {
    String value = config.get(key);
    if (!value.isEmpty() && config.isPasswordKey(key))
      value = "********";
    mqtt.publish(baseTopic + "/config/" + key, value, true);
  }
});

Mycila::Task mqttPublishTask("MQTT", [](void* params) {
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  Mycila::SystemMemory memory = Mycila::System.getMemory();
  mqtt.publish(baseTopic + "/system/device/heap/usage", String(memory.usage));
  mqtt.publish(baseTopic + "/system/device/heap/used", String(memory.used));
  mqtt.publish(baseTopic + "/system/device/uptime", String(Mycila::System.getUptime()));

  mqtt.publish(baseTopic + "/system/network/eth/ip_address", ESPConnect.getIPAddress(ESPConnectMode::ETH).toString());
  mqtt.publish(baseTopic + "/system/network/ip_address", ESPConnect.getIPAddress().toString());
  mqtt.publish(baseTopic + "/system/network/mac_address", ESPConnect.getMACAddress());
  mqtt.publish(baseTopic + "/system/network/ntp", YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt.publish(baseTopic + "/system/network/wifi/bssid", ESPConnect.getWiFiBSSID());
  mqtt.publish(baseTopic + "/system/network/wifi/ip_address", ESPConnect.getIPAddress(ESPConnectMode::STA).toString());
  mqtt.publish(baseTopic + "/system/network/wifi/quality", String(ESPConnect.getWiFiSignalQuality()));
  mqtt.publish(baseTopic + "/system/network/wifi/rssi", String(ESPConnect.getWiFiRSSI()));
  mqtt.publish(baseTopic + "/system/network/wifi/ssid", ESPConnect.getWiFiSSID());

  switch (ESPConnect.getMode()) {
    case ESPConnectMode::ETH:
      mqtt.publish(baseTopic + "/system/network/mode", "eth");
      break;
    case ESPConnectMode::STA:
      mqtt.publish(baseTopic + "/system/network/mode", "wifi");
      break;
    case ESPConnectMode::AP:
      mqtt.publish(baseTopic + "/system/network/mode", "ap");
      break;
    default:
      mqtt.publish(baseTopic + "/system/network/mode", "");
      break;
  }

  mqtt.publish(baseTopic + "/grid/energy_returned", String(Mycila::Grid.getActiveEnergyReturned(), 3));
  mqtt.publish(baseTopic + "/grid/energy", String(Mycila::Grid.getActiveEnergy(), 3));
  mqtt.publish(baseTopic + "/grid/online", YASOLR_BOOL(Mycila::Grid.isConnected()));
  mqtt.publish(baseTopic + "/grid/power_factor", String(Mycila::Grid.getPowerFactor(), 3));
  mqtt.publish(baseTopic + "/grid/power", String(Mycila::Grid.getActivePower(), 3));
  mqtt.publish(baseTopic + "/grid/voltage", String(Mycila::Grid.getVoltage(), 3));

  mqtt.publish(baseTopic + "/router/lights", lights.toString());
  mqtt.publish(baseTopic + "/router/temperature", String(ds18Sys.getLastTemperature()));
  mqtt.publish(baseTopic + "/router/energy", String(Mycila::Router.getTotalRoutedEnergy(), 3));
  mqtt.publish(baseTopic + "/router/power_factor", String(Mycila::Router.getTotalPowerFactor(), 3));
  mqtt.publish(baseTopic + "/router/power", String(Mycila::Router.getTotalRoutedPower(), 3));
  mqtt.publish(baseTopic + "/router/thdi", String(Mycila::Router.getTotalTHDi(), 3));
  mqtt.publish(baseTopic + "/router/virtual_grid_power", String(Mycila::Router.getVirtualGridPower(), 3));

  if (relay1.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay1/state", YASOLR_STATE(relay1.isOn()));
    mqtt.publish(baseTopic + "/router/relay1/switch_count", String(relay1.getSwitchCount()));
  }

  if (relay2.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay2/state", YASOLR_STATE(relay2.isOn()));
    mqtt.publish(baseTopic + "/router/relay2/switch_count", String(relay2.getSwitchCount()));
  }

  if (output1.isEnabled()) {
    mqtt.publish(baseTopic + "/router/output1/bypass/state", YASOLR_STATE(output1.isBypassOn()));
    mqtt.publish(baseTopic + "/router/output1/dimmer/level", String(dimmerO1.getLevel()));
    mqtt.publish(baseTopic + "/router/output1/dimmer/state", YASOLR_STATE(dimmerO1.isOn()));
    mqtt.publish(baseTopic + "/router/output1/relay/state", YASOLR_STATE(bypassRelayO1.isOn()));
    mqtt.publish(baseTopic + "/router/output1/relay/switch_count", String(bypassRelayO1.getSwitchCount()));
    mqtt.publish(baseTopic + "/router/output1/temperature", String(ds18O1.getLastTemperature()));
  }

  if (output2.isEnabled()) {
    mqtt.publish(baseTopic + "/router/output2/bypass/state", YASOLR_STATE(output2.isBypassOn()));
    mqtt.publish(baseTopic + "/router/output2/dimmer/level", String(dimmerO2.getLevel()));
    mqtt.publish(baseTopic + "/router/output2/dimmer/state", YASOLR_STATE(dimmerO2.isOn()));
    mqtt.publish(baseTopic + "/router/output2/relay/state", YASOLR_STATE(bypassRelayO2.isOn()));
    mqtt.publish(baseTopic + "/router/output2/relay/switch_count", String(bypassRelayO2.getSwitchCount()));
    mqtt.publish(baseTopic + "/router/output2/temperature", String(ds18O2.getLastTemperature()));
  }
});
