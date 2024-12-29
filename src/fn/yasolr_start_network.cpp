// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task networkManagerTask("ESPConnect", [](void* params) { espConnect.loop(); });

Mycila::Task networkStartTask("Network Start", Mycila::TaskType::ONCE, [](void* params) {
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
    logger.info(TAG, "Enable mDNS");
    MDNS.addService("http", "tcp", 80);

    // MQTT
    mqttConfigTask.resume();

    yasolr_start_jsy_remote_listener();
  }
});

void yasolr_start_network() {
  // NTP
  Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

  // Network Manager
  Mycila::ESPConnect::IPConfig ipConfig;
  ipConfig.ip.fromString(config.get(KEY_NET_IP));
  ipConfig.gateway.fromString(config.get(KEY_NET_GATEWAY));
  ipConfig.subnet.fromString(config.get(KEY_NET_SUBNET));
  ipConfig.dns.fromString(config.get(KEY_NET_DNS));
  espConnect.setIPConfig(ipConfig);
  espConnect.setAutoRestart(true);
  espConnect.setBlocking(false);
  espConnect.begin(Mycila::AppInfo.defaultHostname.c_str(), Mycila::AppInfo.defaultSSID.c_str(), config.get(KEY_ADMIN_PASSWORD), {config.get(KEY_WIFI_SSID), config.get(KEY_WIFI_PASSWORD), config.getBool(KEY_ENABLE_AP_MODE)});

  espConnect.listen([](Mycila::ESPConnect::State previous, Mycila::ESPConnect::State state) {
    logger.debug(TAG, "NetworkState: %s => %s", espConnect.getStateName(previous), espConnect.getStateName(state));
    switch (state) {
      case Mycila::ESPConnect::State::NETWORK_DISABLED:
        logger.warn(TAG, "Disabled Network!");
        break;
      case Mycila::ESPConnect::State::AP_STARTING:
        logger.info(TAG, "Starting Access Point %s", espConnect.getAccessPointSSID().c_str());
        break;
      case Mycila::ESPConnect::State::AP_STARTED:
        logger.info(TAG, "Access Point %s started with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        networkStartTask.resume();
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTING:
        logger.info(TAG, "Connecting to network");
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTED:
        logger.info(TAG, "Connected with IP address %s", espConnect.getIPAddress().toString().c_str());
        networkStartTask.resume();
        break;
      case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
        logger.warn(TAG, "Unable to connect!");
        break;
      case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
        logger.warn(TAG, "Disconnected!");
        break;
      case Mycila::ESPConnect::State::NETWORK_RECONNECTING:
        logger.info(TAG, "Trying to reconnect");
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTING:
        logger.info(TAG, "Starting Captive Portal %s for %" PRIu32 " seconds", espConnect.getAccessPointSSID().c_str(), espConnect.getCaptivePortalTimeout());
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTED:
        logger.info(TAG, "Captive Portal started at %s with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        break;
      case Mycila::ESPConnect::State::PORTAL_COMPLETE: {
        if (espConnect.hasConfiguredAPMode()) {
          logger.info(TAG, "Captive Portal: Access Point configured");
          config.setBool(KEY_ENABLE_AP_MODE, true);
        } else {
          logger.info(TAG, "Captive Portal: WiFi configured");
          config.setBool(KEY_ENABLE_AP_MODE, false);
          config.set(KEY_WIFI_SSID, espConnect.getConfiguredWiFiSSID());
          config.set(KEY_WIFI_PASSWORD, espConnect.getConfiguredWiFiPassword());
        }
        break;
      }
      case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
        logger.warn(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
    dashboardInitTask.resume();
  });

  networkStartTask.setManager(coreTaskManager);

  networkManagerTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  networkManagerTask.setManager(coreTaskManager);
}
