// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task mqttPublishStaticTask("MQTT Static", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Publishing static data to MQTT...");
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  mqtt.publish(baseTopic + "/system/app/name", Mycila::AppInfo.name, true);
  mqtt.publish(baseTopic + "/system/app/model", Mycila::AppInfo.model, true);
  mqtt.publish(baseTopic + "/system/app/version", Mycila::AppInfo.version, true);
  mqtt.publish(baseTopic + "/system/app/manufacturer", Mycila::AppInfo.manufacturer, true);
  mqtt.publish(baseTopic + "/system/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial), true);
  yield();

  mqtt.publish(baseTopic + "/system/device/id", Mycila::AppInfo.id, true);
  mqtt.publish(baseTopic + "/system/device/model", ESP.getChipModel(), true);
  mqtt.publish(baseTopic + "/system/device/cores", String(ESP.getChipCores()), true);
  mqtt.publish(baseTopic + "/system/device/cpu_freq", String(ESP.getCpuFreqMHz()), true);
  mqtt.publish(baseTopic + "/system/device/boots", String(Mycila::System.getBootCount()), true);
  mqtt.publish(baseTopic + "/system/device/heap/total", String(ESP.getHeapSize()), true);
  yield();

  mqtt.publish(baseTopic + "/system/firmware/filename", Mycila::AppInfo.firmware, true);
  mqtt.publish(baseTopic + "/system/firmware/build/hash", Mycila::AppInfo.buildHash, true);
  mqtt.publish(baseTopic + "/system/firmware/build/branch", Mycila::AppInfo.buildBranch, true);
  mqtt.publish(baseTopic + "/system/firmware/build/timestamp", Mycila::AppInfo.buildDate, true);
  mqtt.publish(baseTopic + "/system/firmware/debug", YASOLR_BOOL(Mycila::AppInfo.debug), true);
  yield();

  mqtt.publish(baseTopic + "/system/network/hostname", ESPConnect.getHostname(), true);
  mqtt.publish(baseTopic + "/system/network/eth/mac_address", ESPConnect.getMACAddress(ESPConnectMode::ETH), true);
  mqtt.publish(baseTopic + "/system/network/wifi/mac_address", ESPConnect.getMACAddress(ESPConnectMode::STA), true);
  yield();

  mqtt.publish(baseTopic + "/router/relay1/state", YASOLR_STATE(relay1.isOn()), true);
  mqtt.publish(baseTopic + "/router/relay1/switch_count", String(relay1.getSwitchCount()), true);
  yield();

  if (!relay2.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay2/state", YASOLR_STATE(relay2.isOn()), true);
    mqtt.publish(baseTopic + "/router/relay2/switch_count", String(relay2.getSwitchCount()), true);
    yield();
  }
});

Mycila::Task mqttPublishConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Publishing config to MQTT...");
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
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/ip_address", ESPConnect.getIPAddress(ESPConnectMode::ETH).toString());
  mqtt.publish(baseTopic + "/system/network/ip_address", ESPConnect.getIPAddress().toString());
  mqtt.publish(baseTopic + "/system/network/mac_address", ESPConnect.getMACAddress());
  mqtt.publish(baseTopic + "/system/network/ntp", YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt.publish(baseTopic + "/system/network/wifi/bssid", ESPConnect.getWiFiBSSID());
  mqtt.publish(baseTopic + "/system/network/wifi/ip_address", ESPConnect.getIPAddress(ESPConnectMode::STA).toString());
  mqtt.publish(baseTopic + "/system/network/wifi/quality", String(ESPConnect.getWiFiSignalQuality()));
  mqtt.publish(baseTopic + "/system/network/wifi/rssi", String(ESPConnect.getWiFiRSSI()));
  mqtt.publish(baseTopic + "/system/network/wifi/ssid", ESPConnect.getWiFiSSID());
  yield();

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

  Mycila::GridMetrics gridMetrics;
  grid.getMetrics(gridMetrics);
  mqtt.publish(baseTopic + "/grid/apparent_power", String(gridMetrics.apparentPower, 3));
  mqtt.publish(baseTopic + "/grid/current", String(gridMetrics.current, 3));
  mqtt.publish(baseTopic + "/grid/energy", String(gridMetrics.energy, 3));
  mqtt.publish(baseTopic + "/grid/energy_returned", String(gridMetrics.energyReturned, 3));
  mqtt.publish(baseTopic + "/grid/frequency", String(gridMetrics.frequency, 3));
  mqtt.publish(baseTopic + "/grid/online", YASOLR_BOOL(grid.isConnected()));
  mqtt.publish(baseTopic + "/grid/power", String(gridMetrics.power, 3));
  mqtt.publish(baseTopic + "/grid/power_factor", String(gridMetrics.powerFactor, 3));
  mqtt.publish(baseTopic + "/grid/voltage", String(gridMetrics.voltage, 3));
  yield();

  Mycila::RouterMetrics routerMetrics;
  router.getMetrics(routerMetrics);
  mqtt.publish(baseTopic + "/router/apparent_power", String(routerMetrics.apparentPower, 3));
  mqtt.publish(baseTopic + "/router/current", String(routerMetrics.current, 3));
  mqtt.publish(baseTopic + "/router/energy", String(routerMetrics.energy, 3));
  mqtt.publish(baseTopic + "/router/lights", lights.toString());
  mqtt.publish(baseTopic + "/router/power", String(routerMetrics.power, 3));
  mqtt.publish(baseTopic + "/router/power_factor", String(routerMetrics.powerFactor, 3));
  mqtt.publish(baseTopic + "/router/temperature", String(ds18Sys.getValidTemperature()));
  mqtt.publish(baseTopic + "/router/thdi", String(routerMetrics.thdi, 3));
  mqtt.publish(baseTopic + "/router/virtual_grid_power", String(gridMetrics.power - routerMetrics.power, 3));
  yield();

  mqtt.publish(baseTopic + "/router/output1/bypass", YASOLR_STATE(output1.isBypassOn()));
  mqtt.publish(baseTopic + "/router/output1/state", output1.getStateName());
  mqtt.publish(baseTopic + "/router/output1/temperature", String(ds18O1.getValidTemperature(), 1));
  mqtt.publish(baseTopic + "/router/output1/dimmer/duty", String(dimmerO1.getPowerDuty()));
  mqtt.publish(baseTopic + "/router/output1/dimmer/duty_cycle", String(dimmerO1.getPowerDutyCycle()));
  mqtt.publish(baseTopic + "/router/output1/dimmer/state", YASOLR_STATE(dimmerO1.isOn()));
  mqtt.publish(baseTopic + "/router/output1/relay/state", YASOLR_STATE(bypassRelayO1.isOn()));
  mqtt.publish(baseTopic + "/router/output1/relay/switch_count", String(bypassRelayO1.getSwitchCount()));
  yield();

  mqtt.publish(baseTopic + "/router/output2/bypass", YASOLR_STATE(output2.isBypassOn()));
  mqtt.publish(baseTopic + "/router/output2/state", output2.getStateName());
  mqtt.publish(baseTopic + "/router/output2/temperature", String(ds18O2.getValidTemperature(), 1));
  mqtt.publish(baseTopic + "/router/output2/dimmer/duty", String(dimmerO2.getPowerDuty()));
  mqtt.publish(baseTopic + "/router/output2/dimmer/duty_cycle", String(dimmerO2.getPowerDutyCycle()));
  mqtt.publish(baseTopic + "/router/output2/dimmer/state", YASOLR_STATE(dimmerO2.isOn()));
  mqtt.publish(baseTopic + "/router/output2/relay/state", YASOLR_STATE(bypassRelayO2.isOn()));
  mqtt.publish(baseTopic + "/router/output2/relay/switch_count", String(bypassRelayO2.getSwitchCount()));
  yield();

  if (relay1.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay1/state", YASOLR_STATE(relay1.isOn()));
    mqtt.publish(baseTopic + "/router/relay1/switch_count", String(relay1.getSwitchCount()));
    yield();
  }

  if (relay2.isEnabled()) {
    mqtt.publish(baseTopic + "/router/relay2/state", YASOLR_STATE(relay2.isOn()));
    mqtt.publish(baseTopic + "/router/relay2/switch_count", String(relay2.getSwitchCount()));
    yield();
  }
});
