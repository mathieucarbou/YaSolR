// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task mqttConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  mqtt.end();
  if (!config.getBool(KEY_ENABLE_AP_MODE) && config.getBool(KEY_ENABLE_MQTT)) {
    logger.info(TAG, "Enable MQTT...");

    bool secured = config.getBool(KEY_MQTT_SECURED);
    String serverCert = "";

    if (secured && LittleFS.exists("/mqtt-server.crt")) {
      logger.info(TAG, "Loading MQTT server certificate...");
      File serverCertFile = LittleFS.open("/mqtt-server.crt", "r");
      serverCert = serverCertFile.readString();
      serverCertFile.close();
      logger.info(TAG, "Loaded MQTT server certificate:\n%s", serverCert.c_str());
    }

    mqtt.begin({
      .server = config.get(KEY_MQTT_SERVER),
      .port = static_cast<uint16_t>(config.get(KEY_MQTT_PORT).toInt()),
      .secured = secured,
      .serverCert = serverCert,
      .username = config.get(KEY_MQTT_USERNAME),
      .password = config.get(KEY_MQTT_PASSWORD),
      .clientId = Mycila::AppInfo.defaultMqttClientId,
      .willTopic = config.get(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC,
      .keepAlive = YASOLR_MQTT_KEEPALIVE,
    });
  }
});

Mycila::Task mqttPublishStaticTask("MQTT Static", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Publishing static data to MQTT...");
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  mqtt.publish(baseTopic + "/system/app/manufacturer", Mycila::AppInfo.manufacturer, true);
  mqtt.publish(baseTopic + "/system/app/model", Mycila::AppInfo.model, true);
  mqtt.publish(baseTopic + "/system/app/name", Mycila::AppInfo.name, true);
  mqtt.publish(baseTopic + "/system/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial), true);
  mqtt.publish(baseTopic + "/system/app/version", Mycila::AppInfo.version, true);
  yield();

  mqtt.publish(baseTopic + "/system/device/boots", String(Mycila::System.getBootCount()), true);
  mqtt.publish(baseTopic + "/system/device/cores", String(ESP.getChipCores()), true);
  mqtt.publish(baseTopic + "/system/device/cpu_freq", String(ESP.getCpuFreqMHz()), true);
  mqtt.publish(baseTopic + "/system/device/heap/total", String(ESP.getHeapSize()), true);
  mqtt.publish(baseTopic + "/system/device/id", Mycila::AppInfo.id, true);
  mqtt.publish(baseTopic + "/system/device/model", ESP.getChipModel(), true);
  yield();

  mqtt.publish(baseTopic + "/system/firmware/build/branch", Mycila::AppInfo.buildBranch, true);
  mqtt.publish(baseTopic + "/system/firmware/build/hash", Mycila::AppInfo.buildHash, true);
  mqtt.publish(baseTopic + "/system/firmware/build/timestamp", Mycila::AppInfo.buildDate, true);
  mqtt.publish(baseTopic + "/system/firmware/debug", YASOLR_BOOL(Mycila::AppInfo.debug), true);
  mqtt.publish(baseTopic + "/system/firmware/filename", Mycila::AppInfo.firmware, true);
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/mac_address", ESPConnect.getMACAddress(ESPConnectMode::ETH), true);
  mqtt.publish(baseTopic + "/system/network/hostname", ESPConnect.getHostname(), true);
  mqtt.publish(baseTopic + "/system/network/wifi/mac_address", ESPConnect.getMACAddress(ESPConnectMode::STA), true);
  yield();
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
  grid.getMeasurements(gridMetrics);
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

  Mycila::RouterMetrics routerMeasurements;
  router.getMeasurements(routerMeasurements);
  mqtt.publish(baseTopic + "/router/apparent_power", String(routerMeasurements.apparentPower, 3));
  mqtt.publish(baseTopic + "/router/current", String(routerMeasurements.current, 3));
  mqtt.publish(baseTopic + "/router/energy", String(routerMeasurements.energy, 3));
  mqtt.publish(baseTopic + "/router/lights", lights.toString());
  mqtt.publish(baseTopic + "/router/power", String(routerMeasurements.power, 3));
  mqtt.publish(baseTopic + "/router/power_factor", String(routerMeasurements.powerFactor, 3));
  mqtt.publish(baseTopic + "/router/temperature", String(ds18Sys.getTemperature().value_or(0)));
  mqtt.publish(baseTopic + "/router/thdi", String(routerMeasurements.thdi, 3));
  mqtt.publish(baseTopic + "/router/virtual_grid_power", String(gridMetrics.power - routerMeasurements.power, 3));
  yield();

  mqtt.publish(baseTopic + "/router/output1/bypass", YASOLR_STATE(output1.isBypassOn()));
  mqtt.publish(baseTopic + "/router/output1/state", output1.getStateName());
  mqtt.publish(baseTopic + "/router/output1/temperature", String(output1.temperature().orElse(0), 1));
  mqtt.publish(baseTopic + "/router/output1/dimmer/duty", String(dimmerO1.getDuty()));
  mqtt.publish(baseTopic + "/router/output1/dimmer/duty_cycle", String(dimmerO1.getDutyCycle()));
  mqtt.publish(baseTopic + "/router/output1/dimmer/state", YASOLR_STATE(dimmerO1.isOn()));
  yield();

  mqtt.publish(baseTopic + "/router/output2/bypass", YASOLR_STATE(output2.isBypassOn()));
  mqtt.publish(baseTopic + "/router/output2/state", output2.getStateName());
  mqtt.publish(baseTopic + "/router/output2/temperature", String(output2.temperature().orElse(0), 1));
  mqtt.publish(baseTopic + "/router/output2/dimmer/duty", String(dimmerO2.getDuty()));
  mqtt.publish(baseTopic + "/router/output2/dimmer/duty_cycle", String(dimmerO2.getDutyCycle()));
  mqtt.publish(baseTopic + "/router/output2/dimmer/state", YASOLR_STATE(dimmerO2.isOn()));
  yield();

  mqtt.publish(baseTopic + "/router/relay1/state", YASOLR_STATE(relay1.isOn()));
  mqtt.publish(baseTopic + "/router/relay2/state", YASOLR_STATE(relay2.isOn()));
  yield();
});
