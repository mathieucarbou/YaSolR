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

    logger.info(TAG, "Enable MQTT (server: %s://%s:%" PRIu16 ")", (secured ? "mqtts" : "mqtt"), mqttConfig.server.c_str(), mqttConfig.port);
    mqtt.setAsync(false);
    mqtt.begin(mqttConfig);
  }
});
