// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

extern const uint8_t ca_certs_bundle_start[] asm("_binary__pio_data_cacerts_bin_start");
extern const uint8_t ca_certs_bundle_end[] asm("_binary__pio_data_cacerts_bin_end");

Mycila::Task mqttConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  mqtt.end();
  if (!config.getBool(KEY_ENABLE_AP_MODE) && config.getBool(KEY_ENABLE_MQTT)) {
    bool secured = config.getBool(KEY_MQTT_SECURED);

    Mycila::MQTT::Config mqttConfig;
    mqttConfig.server = config.get(KEY_MQTT_SERVER);
    mqttConfig.port = static_cast<uint16_t>(config.getLong(KEY_MQTT_PORT));
    mqttConfig.secured = secured;
    mqttConfig.username = config.get(KEY_MQTT_USERNAME);
    mqttConfig.password = config.get(KEY_MQTT_PASSWORD);
    mqttConfig.clientId = Mycila::AppInfo.defaultMqttClientId;
    mqttConfig.willTopic = String(config.get(KEY_MQTT_TOPIC)) + YASOLR_MQTT_WILL_TOPIC;
    mqttConfig.keepAlive = YASOLR_MQTT_KEEPALIVE;

    if (secured) {
      // if a server certificate has been used, set it
      if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE)) {
        logger.debug(TAG, "Loading MQTT PEM server certificate");
        File serverCertFile = LittleFS.open(YASOLR_MQTT_SERVER_CERT_FILE, "r");
        mqttConfig.serverCert = serverCertFile.readString();
        serverCertFile.close();
        logger.debug(TAG, "Loaded MQTT server certificate:\n%s", mqttConfig.serverCert.c_str());
      } else {
        logger.debug(TAG, "Using cacert bundle for MQTT");
        // if no server certificate has been used, set default CA certs bundle
        mqttConfig.certBundle = ca_certs_bundle_start;
        mqttConfig.certBundleSize = static_cast<size_t>(ca_certs_bundle_end - ca_certs_bundle_start);
      }
    }

    logger.info(TAG, "Enable MQTT (server: ://%s:%" PRIu16 ")", (secured ? "mqtts" : "mqtt"), mqttConfig.server.c_str(), mqttConfig.port);
    mqtt.setAsync(false);
    mqtt.begin(mqttConfig);
  }
});

Mycila::Task mqttPublishStaticTask("MQTT Static", Mycila::TaskType::ONCE, [](void* params) {
  logger.debug(TAG, "Publishing static data to MQTT");
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  mqtt.publish(baseTopic + "/system/app/manufacturer", Mycila::AppInfo.manufacturer, true);
  mqtt.publish(baseTopic + "/system/app/model", Mycila::AppInfo.model, true);
  mqtt.publish(baseTopic + "/system/app/name", Mycila::AppInfo.name, true);
  mqtt.publish(baseTopic + "/system/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial), true);
  mqtt.publish(baseTopic + "/system/app/version", Mycila::AppInfo.version, true);
  yield();

  mqtt.publish(baseTopic + "/system/device/boots", String(Mycila::System::getBootCount()), true);
  mqtt.publish(baseTopic + "/system/device/cores", String(ESP.getChipCores()), true);
  mqtt.publish(baseTopic + "/system/device/cpu_freq", String(ESP.getCpuFreqMHz()), true);
  mqtt.publish(baseTopic + "/system/device/id", Mycila::AppInfo.id, true);
  mqtt.publish(baseTopic + "/system/device/model", ESP.getChipModel(), true);
  yield();

  mqtt.publish(baseTopic + "/system/firmware/build/branch", Mycila::AppInfo.buildBranch, true);
  mqtt.publish(baseTopic + "/system/firmware/build/hash", Mycila::AppInfo.buildHash, true);
  mqtt.publish(baseTopic + "/system/firmware/build/timestamp", Mycila::AppInfo.buildDate, true);
  mqtt.publish(baseTopic + "/system/firmware/debug", YASOLR_BOOL(Mycila::AppInfo.debug), true);
  mqtt.publish(baseTopic + "/system/firmware/filename", Mycila::AppInfo.firmware, true);
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/mac_address", espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH).c_str(), true);
  mqtt.publish(baseTopic + "/system/network/hostname", espConnect.getHostname().c_str(), true);
  mqtt.publish(baseTopic + "/system/network/wifi/mac_address", espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA).c_str(), true);
  yield();
});

Mycila::Task mqttPublishConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  logger.debug(TAG, "Publishing config to MQTT");
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  for (auto& key : config.keys()) {
    String value = config.get(key);
    if (!value.isEmpty() && config.isPasswordKey(key))
      value = "********";
    mqtt.publish(baseTopic + "/config/" + key, value, true);
  }
});

Mycila::Task mqttPublishTask("MQTT", [](void* params) {
  const String baseTopic = config.get(KEY_MQTT_TOPIC);

  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);

  mqtt.publish(baseTopic + "/system/device/heap/total", String(memory.total));
  mqtt.publish(baseTopic + "/system/device/heap/usage", String(memory.usage));
  mqtt.publish(baseTopic + "/system/device/heap/used", String(memory.used));
  mqtt.publish(baseTopic + "/system/device/uptime", String(Mycila::System::getUptime()));
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  mqtt.publish(baseTopic + "/system/network/ip_address", espConnect.getIPAddress().toString().c_str());
  mqtt.publish(baseTopic + "/system/network/mac_address", espConnect.getMACAddress().c_str());
  mqtt.publish(baseTopic + "/system/network/ntp", YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt.publish(baseTopic + "/system/network/wifi/bssid", espConnect.getWiFiBSSID().c_str());
  mqtt.publish(baseTopic + "/system/network/wifi/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  mqtt.publish(baseTopic + "/system/network/wifi/quality", String(espConnect.getWiFiSignalQuality()).c_str());
  mqtt.publish(baseTopic + "/system/network/wifi/rssi", String(espConnect.getWiFiRSSI()).c_str());
  mqtt.publish(baseTopic + "/system/network/wifi/ssid", espConnect.getWiFiSSID().c_str());
  yield();

  switch (espConnect.getMode()) {
    case Mycila::ESPConnect::Mode::ETH:
      mqtt.publish(baseTopic + "/system/network/mode", "eth");
      break;
    case Mycila::ESPConnect::Mode::STA:
      mqtt.publish(baseTopic + "/system/network/mode", "wifi");
      break;
    case Mycila::ESPConnect::Mode::AP:
      mqtt.publish(baseTopic + "/system/network/mode", "ap");
      break;
    default:
      mqtt.publish(baseTopic + "/system/network/mode", "");
      break;
  }

  Mycila::Grid::Metrics gridMetrics;
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

  Mycila::Router::Metrics routerMeasurements;
  router.getMeasurements(routerMeasurements);
  mqtt.publish(baseTopic + "/router/apparent_power", String(routerMeasurements.apparentPower, 3));
  mqtt.publish(baseTopic + "/router/current", String(routerMeasurements.current, 3));
  mqtt.publish(baseTopic + "/router/energy", String(routerMeasurements.energy, 3));
  mqtt.publish(baseTopic + "/router/lights", lights.toString().c_str());
  mqtt.publish(baseTopic + "/router/power_factor", String(routerMeasurements.powerFactor, 3));
  mqtt.publish(baseTopic + "/router/power", String(routerMeasurements.power, 3));
  mqtt.publish(baseTopic + "/router/relay1", YASOLR_STATE(relay1.isOn()));
  mqtt.publish(baseTopic + "/router/relay2", YASOLR_STATE(relay2.isOn()));
  mqtt.publish(baseTopic + "/router/temperature", String(ds18Sys.getTemperature().value_or(0)));
  mqtt.publish(baseTopic + "/router/thdi", String(routerMeasurements.thdi, 3));
  mqtt.publish(baseTopic + "/router/virtual_grid_power", String(gridMetrics.power - routerMeasurements.power, 3));
  yield();

  for (const auto& output : router.getOutputs()) {
    const String outputTopic = baseTopic + "/router/" + output->getName();
    mqtt.publish(outputTopic + "/state", output->getStateName());
    mqtt.publish(outputTopic + "/bypass", YASOLR_STATE(output->isBypassOn()));
    mqtt.publish(outputTopic + "/dimmer", YASOLR_STATE(output->isDimmerOn()));
    mqtt.publish(outputTopic + "/duty_cycle", String(output->getDimmerDutyCycle() * 100));
    mqtt.publish(outputTopic + "/temperature", String(output->temperature().orElse(0), 1));
    yield();
  }
});
