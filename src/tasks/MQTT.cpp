// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

extern const uint8_t ca_certs_bundle_start[] asm("_binary__pio_data_cacerts_bin_start");
extern const uint8_t ca_certs_bundle_end[] asm("_binary__pio_data_cacerts_bin_end");

Mycila::Task mqttConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  mqtt.end();
  if (!config.getBool(KEY_ENABLE_AP_MODE) && config.getBool(KEY_ENABLE_MQTT)) {
    bool secured = config.getBool(KEY_MQTT_SECURED);

    Mycila::MQTT::Config mqttConfig;
    mqttConfig.server = config.getString(KEY_MQTT_SERVER);
    mqttConfig.port = static_cast<uint16_t>(config.getLong(KEY_MQTT_PORT));
    mqttConfig.secured = secured;
    mqttConfig.username = config.getString(KEY_MQTT_USERNAME);
    mqttConfig.password = config.getString(KEY_MQTT_PASSWORD);
    mqttConfig.clientId = Mycila::AppInfo.defaultMqttClientId;
    mqttConfig.willTopic = config.getString(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC;
    mqttConfig.keepAlive = YASOLR_MQTT_KEEPALIVE;

    if (secured) {
      // if a server certificate has been used, set it
      if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE)) {
        logger.debug(TAG, "Loading MQTT PEM server certificate");
        File serverCertFile = LittleFS.open(YASOLR_MQTT_SERVER_CERT_FILE, "r");
        mqttConfig.serverCert = serverCertFile.readString().c_str();
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

Mycila::Task mqttPublishTask("MQTT", [](void* params) {
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);

  mqtt.publish(baseTopic + "/system/device/heap/total", std::to_string(memory.total));
  mqtt.publish(baseTopic + "/system/device/heap/usage", std::to_string(memory.usage));
  mqtt.publish(baseTopic + "/system/device/heap/used", std::to_string(memory.used));
  mqtt.publish(baseTopic + "/system/device/uptime", std::to_string(Mycila::System::getUptime()));
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  mqtt.publish(baseTopic + "/system/network/ip_address", espConnect.getIPAddress().toString().c_str());
  mqtt.publish(baseTopic + "/system/network/mac_address", espConnect.getMACAddress());
  mqtt.publish(baseTopic + "/system/network/ntp", YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt.publish(baseTopic + "/system/network/wifi/bssid", espConnect.getWiFiBSSID());
  mqtt.publish(baseTopic + "/system/network/wifi/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  mqtt.publish(baseTopic + "/system/network/wifi/quality", std::to_string(espConnect.getWiFiSignalQuality()));
  mqtt.publish(baseTopic + "/system/network/wifi/rssi", std::to_string(espConnect.getWiFiRSSI()));
  mqtt.publish(baseTopic + "/system/network/wifi/ssid", espConnect.getWiFiSSID());
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
  grid.getGridMeasurements(gridMetrics);
  mqtt.publish(baseTopic + "/grid/apparent_power", std::to_string(gridMetrics.apparentPower));
  mqtt.publish(baseTopic + "/grid/current", std::to_string(gridMetrics.current));
  mqtt.publish(baseTopic + "/grid/energy", std::to_string(gridMetrics.energy));
  mqtt.publish(baseTopic + "/grid/energy_returned", std::to_string(gridMetrics.energyReturned));
  mqtt.publish(baseTopic + "/grid/frequency", std::to_string(gridMetrics.frequency));
  mqtt.publish(baseTopic + "/grid/online", YASOLR_BOOL(grid.isConnected()));
  mqtt.publish(baseTopic + "/grid/power", std::to_string(gridMetrics.power));
  mqtt.publish(baseTopic + "/grid/power_factor", std::to_string(gridMetrics.powerFactor));
  mqtt.publish(baseTopic + "/grid/voltage", std::to_string(gridMetrics.voltage));
  yield();

  Mycila::Router::Metrics routerMeasurements;
  router.getRouterMeasurements(routerMeasurements);
  mqtt.publish(baseTopic + "/router/apparent_power", std::to_string(routerMeasurements.apparentPower));
  mqtt.publish(baseTopic + "/router/current", std::to_string(routerMeasurements.current));
  mqtt.publish(baseTopic + "/router/energy", std::to_string(routerMeasurements.energy));
  mqtt.publish(baseTopic + "/router/lights", lights.toString());
  mqtt.publish(baseTopic + "/router/power_factor", isnan(routerMeasurements.powerFactor) ? "0" : std::to_string(routerMeasurements.powerFactor));
  mqtt.publish(baseTopic + "/router/power", std::to_string(routerMeasurements.power));
  mqtt.publish(baseTopic + "/router/relay1", YASOLR_STATE(relay1.isOn()));
  mqtt.publish(baseTopic + "/router/relay2", YASOLR_STATE(relay2.isOn()));
  mqtt.publish(baseTopic + "/router/temperature", std::to_string(ds18Sys.getTemperature().value_or(0)));
  mqtt.publish(baseTopic + "/router/thdi", isnan(routerMeasurements.thdi) ? "0" : std::to_string(routerMeasurements.thdi));
  mqtt.publish(baseTopic + "/router/virtual_grid_power", std::to_string(gridMetrics.power - routerMeasurements.power));
  yield();

  for (const auto& output : router.getOutputs()) {
    const std::string outputTopic = baseTopic + "/router/" + output->getName();
    mqtt.publish(outputTopic + "/state", output->getStateName());
    mqtt.publish(outputTopic + "/bypass", YASOLR_STATE(output->isBypassOn()));
    mqtt.publish(outputTopic + "/dimmer", YASOLR_STATE(output->isDimmerOn()));
    mqtt.publish(outputTopic + "/duty_cycle", std::to_string(output->getDimmerDutyCycle() * 100));
    mqtt.publish(outputTopic + "/temperature", std::to_string(output->temperature().orElse(0)));
    yield();
  }
});
