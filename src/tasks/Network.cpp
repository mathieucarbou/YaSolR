// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#ifndef ESPCONNECT_NO_MDNS
  #include <ESPmDNS.h>
#endif

Mycila::Task networkManagerTask("ESPConnect", [](void* params) { espConnect.loop(); });

Mycila::Task networkConfigTask("Network UP", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Enable Network Services");

  // Web server
  logger.info(TAG, "Enable Web Server");
  webServer.begin();
  webServer.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404);
  });

  if (!config.getBool(KEY_ENABLE_AP_MODE)) {
    // NTP
    logger.info(TAG, "Enable NTP");
    Mycila::NTP.sync(config.get(KEY_NTP_SERVER));

    // mDNS
#ifndef ESPCONNECT_NO_MDNS
    logger.info(TAG, "Enable mDNS");
    MDNS.addService("http", "tcp", 80);
#endif

    // MQTT
    mqttConfigTask.resume();
  }

  // UDP Server
  const uint16_t udpPort = config.getLong(KEY_UDP_PORT);
  logger.info(TAG, "Enable UDP Server on port %" PRIu16, udpPort);
  udp.listen(udpPort);
});
