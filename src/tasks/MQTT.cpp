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
    mqttConfig.server = config.get(KEY_MQTT_SERVER);
    mqttConfig.port = static_cast<uint16_t>(config.getLong(KEY_MQTT_PORT));
    mqttConfig.secured = secured;
    mqttConfig.username = config.get(KEY_MQTT_USERNAME);
    mqttConfig.password = config.get(KEY_MQTT_PASSWORD);
    mqttConfig.clientId = Mycila::AppInfo.defaultMqttClientId.c_str();
    mqttConfig.willTopic = std::string(config.get(KEY_MQTT_TOPIC)) + YASOLR_MQTT_WILL_TOPIC;
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
  const std::string baseTopic = config.get(KEY_MQTT_TOPIC);

  mqtt.publish((baseTopic + "/system/app/manufacturer").c_str(), Mycila::AppInfo.manufacturer.c_str(), true);
  mqtt.publish((baseTopic + "/system/app/model").c_str(), Mycila::AppInfo.model.c_str(), true);
  mqtt.publish((baseTopic + "/system/app/name").c_str(), Mycila::AppInfo.name.c_str(), true);
  mqtt.publish((baseTopic + "/system/app/trial").c_str(), YASOLR_BOOL(Mycila::AppInfo.trial), true);
  mqtt.publish((baseTopic + "/system/app/version").c_str(), Mycila::AppInfo.version.c_str(), true);
  yield();

  mqtt.publish((baseTopic + "/system/device/boots").c_str(), std::to_string(Mycila::System::getBootCount()).c_str(), true);
  mqtt.publish((baseTopic + "/system/device/cores").c_str(), std::to_string(ESP.getChipCores()).c_str(), true);
  mqtt.publish((baseTopic + "/system/device/cpu_freq").c_str(), std::to_string(ESP.getCpuFreqMHz()).c_str(), true);
  mqtt.publish((baseTopic + "/system/device/id").c_str(), Mycila::AppInfo.id.c_str(), true);
  mqtt.publish((baseTopic + "/system/device/model").c_str(), ESP.getChipModel(), true);
  yield();

  mqtt.publish((baseTopic + "/system/firmware/build/branch").c_str(), Mycila::AppInfo.buildBranch.c_str(), true);
  mqtt.publish((baseTopic + "/system/firmware/build/hash").c_str(), Mycila::AppInfo.buildHash.c_str(), true);
  mqtt.publish((baseTopic + "/system/firmware/build/timestamp").c_str(), Mycila::AppInfo.buildDate.c_str(), true);
  mqtt.publish((baseTopic + "/system/firmware/debug").c_str(), YASOLR_BOOL(Mycila::AppInfo.debug), true);
  mqtt.publish((baseTopic + "/system/firmware/filename").c_str(), Mycila::AppInfo.firmware.c_str(), true);
  yield();

  mqtt.publish((baseTopic + "/system/network/eth/mac_address").c_str(), espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH).c_str(), true);
  mqtt.publish((baseTopic + "/system/network/hostname").c_str(), espConnect.getHostname().c_str(), true);
  mqtt.publish((baseTopic + "/system/network/wifi/mac_address").c_str(), espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA).c_str(), true);
  yield();
});

Mycila::Task mqttPublishConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  logger.debug(TAG, "Publishing config to MQTT");
  const std::string baseTopic = config.get(KEY_MQTT_TOPIC);

  for (auto& key : config.keys()) {
    const char* value = config.get(key);
    if (value[0] != '\0' && config.isPasswordKey(key))
      value = "********";
    mqtt.publish((baseTopic + "/config/" + key).c_str(), value, true);
  }
});

Mycila::Task mqttPublishTask("MQTT", [](void* params) {
  const std::string baseTopic = config.get(KEY_MQTT_TOPIC);

  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);

  mqtt.publish((baseTopic + "/system/device/heap/total").c_str(), std::to_string(memory.total).c_str());
  mqtt.publish((baseTopic + "/system/device/heap/usage").c_str(), std::to_string(memory.usage).c_str());
  mqtt.publish((baseTopic + "/system/device/heap/used").c_str(), std::to_string(memory.used).c_str());
  mqtt.publish((baseTopic + "/system/device/uptime").c_str(), std::to_string(Mycila::System::getUptime()).c_str());
  yield();

  mqtt.publish((baseTopic + "/system/network/eth/ip_address").c_str(), espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  mqtt.publish((baseTopic + "/system/network/ip_address").c_str(), espConnect.getIPAddress().toString().c_str());
  mqtt.publish((baseTopic + "/system/network/mac_address").c_str(), espConnect.getMACAddress().c_str());
  mqtt.publish((baseTopic + "/system/network/ntp").c_str(), YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt.publish((baseTopic + "/system/network/wifi/bssid").c_str(), espConnect.getWiFiBSSID().c_str());
  mqtt.publish((baseTopic + "/system/network/wifi/ip_address").c_str(), espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  mqtt.publish((baseTopic + "/system/network/wifi/quality").c_str(), std::to_string(espConnect.getWiFiSignalQuality()).c_str());
  mqtt.publish((baseTopic + "/system/network/wifi/rssi").c_str(), std::to_string(espConnect.getWiFiRSSI()).c_str());
  mqtt.publish((baseTopic + "/system/network/wifi/ssid").c_str(), espConnect.getWiFiSSID().c_str());
  yield();

  switch (espConnect.getMode()) {
    case Mycila::ESPConnect::Mode::ETH:
      mqtt.publish((baseTopic + "/system/network/mode").c_str(), "eth");
      break;
    case Mycila::ESPConnect::Mode::STA:
      mqtt.publish((baseTopic + "/system/network/mode").c_str(), "wifi");
      break;
    case Mycila::ESPConnect::Mode::AP:
      mqtt.publish((baseTopic + "/system/network/mode").c_str(), "ap");
      break;
    default:
      mqtt.publish((baseTopic + "/system/network/mode").c_str(), "");
      break;
  }

  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);
  mqtt.publish((baseTopic + "/grid/apparent_power").c_str(), std::to_string(gridMetrics.apparentPower).c_str());
  mqtt.publish((baseTopic + "/grid/current").c_str(), std::to_string(gridMetrics.current).c_str());
  mqtt.publish((baseTopic + "/grid/energy").c_str(), std::to_string(gridMetrics.energy).c_str());
  mqtt.publish((baseTopic + "/grid/energy_returned").c_str(), std::to_string(gridMetrics.energyReturned).c_str());
  mqtt.publish((baseTopic + "/grid/frequency").c_str(), std::to_string(gridMetrics.frequency).c_str());
  mqtt.publish((baseTopic + "/grid/online").c_str(), YASOLR_BOOL(grid.isConnected()));
  mqtt.publish((baseTopic + "/grid/power").c_str(), std::to_string(gridMetrics.power).c_str());
  mqtt.publish((baseTopic + "/grid/power_factor").c_str(), std::to_string(gridMetrics.powerFactor).c_str());
  mqtt.publish((baseTopic + "/grid/voltage").c_str(), std::to_string(gridMetrics.voltage).c_str());
  yield();

  Mycila::Router::Metrics routerMeasurements;
  router.getMeasurements(routerMeasurements);
  mqtt.publish((baseTopic + "/router/apparent_power").c_str(), std::to_string(routerMeasurements.apparentPower).c_str());
  mqtt.publish((baseTopic + "/router/current").c_str(), std::to_string(routerMeasurements.current).c_str());
  mqtt.publish((baseTopic + "/router/energy").c_str(), std::to_string(routerMeasurements.energy).c_str());
  mqtt.publish((baseTopic + "/router/lights").c_str(), lights.toString().c_str());
  mqtt.publish((baseTopic + "/router/power_factor").c_str(), std::to_string(routerMeasurements.powerFactor).c_str());
  mqtt.publish((baseTopic + "/router/power").c_str(), std::to_string(routerMeasurements.power).c_str());
  mqtt.publish((baseTopic + "/router/relay1").c_str(), YASOLR_STATE(relay1.isOn()));
  mqtt.publish((baseTopic + "/router/relay2").c_str(), YASOLR_STATE(relay2.isOn()));
  mqtt.publish((baseTopic + "/router/temperature").c_str(), std::to_string(ds18Sys.getTemperature().value_or(0)).c_str());
  mqtt.publish((baseTopic + "/router/thdi").c_str(), std::to_string(routerMeasurements.thdi).c_str());
  mqtt.publish((baseTopic + "/router/virtual_grid_power").c_str(), std::to_string(gridMetrics.power - routerMeasurements.power).c_str());
  yield();

  for (const auto& output : router.getOutputs()) {
    const std::string outputTopic = baseTopic + "/router/" + output->getName();
    mqtt.publish((outputTopic + "/state").c_str(), output->getStateName());
    mqtt.publish((outputTopic + "/bypass").c_str(), YASOLR_STATE(output->isBypassOn()));
    mqtt.publish((outputTopic + "/dimmer").c_str(), YASOLR_STATE(output->isDimmerOn()));
    mqtt.publish((outputTopic + "/duty_cycle").c_str(), std::to_string(output->getDimmerDutyCycle() * 100).c_str());
    mqtt.publish((outputTopic + "/temperature").c_str(), std::to_string(output->temperature().orElse(0)).c_str());
    yield();
  }
});
