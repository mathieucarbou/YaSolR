// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#ifndef ESPCONNECT_NO_MDNS
  #include <ESPmDNS.h>
#endif

Mycila::Task networkManagerTask("ESPConnect", [](void* params) { ESPConnect.loop(); });

Mycila::Task networkUpTask("Network UP", Mycila::TaskType::ONCE, [](void* params) {
  logger.info("YASOLR", "Enable Network Services...");

  // Web server
  logger.info("YASOLR", "Enable Web Server...");
  webServer.begin();
  webServer.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404);
  });

  if (!config.getBool(KEY_ENABLE_AP_MODE)) {
    // NTP
    logger.info("YASOLR", "Enable NTP...");
    Mycila::NTP.sync(config.get(KEY_NTP_SERVER));

    // mDNS
#ifndef ESPCONNECT_NO_MDNS
    logger.info("YASOLR", "Enable mDNS...");
    MDNS.addService("http", "tcp", 80);
#endif

    // MQTT
    mqttConfigTask.resume();
  }

  // UDP Server
  switch (ESPConnect.getMode()) {
    case ESPConnectMode::AP:
      logger.info("YASOLR", "Enable UDP Server on AP interface...");
      udp.listenMulticast(IPAddress(YASOLR_UDP_ADDRESS), YASOLR_UDP_PORT, 1, tcpip_adapter_if_t::TCPIP_ADAPTER_IF_AP);
      break;
    case ESPConnectMode::STA:
      logger.info("YASOLR", "Enable UDP Server on STA interface...");
      udp.listenMulticast(IPAddress(YASOLR_UDP_ADDRESS), YASOLR_UDP_PORT, 1, tcpip_adapter_if_t::TCPIP_ADAPTER_IF_STA);
      break;
    case ESPConnectMode::ETH:
      logger.info("YASOLR", "Enable UDP Server on ETH interface...");
      udp.listenMulticast(IPAddress(YASOLR_UDP_ADDRESS), YASOLR_UDP_PORT, 1, tcpip_adapter_if_t::TCPIP_ADAPTER_IF_ETH);
      break;
    default:
      break;
  }
});

Mycila::Task mqttConfigTask("MQTT Config", Mycila::TaskType::ONCE, [](void* params) {
  mqtt.end();
  if (!config.getBool(KEY_ENABLE_AP_MODE) && config.getBool(KEY_ENABLE_MQTT)) {
    logger.info("YASOLR", "Enable MQTT...");

    bool secured = config.getBool(KEY_MQTT_SECURED);
    String serverCert = "";

    if (secured && LittleFS.exists("/mqtt-server.crt")) {
      logger.debug("YASOLR", "Loading MQTT server certificate...");
      File serverCertFile = LittleFS.open("/mqtt-server.crt", "r");
      serverCert = serverCertFile.readString();
      serverCertFile.close();
      logger.debug("YASOLR", "Loaded MQTT server certificate:\n%s", serverCert.c_str());
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
