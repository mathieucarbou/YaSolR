// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

static Mycila::Task networkManagerTask("ESPConnect", [](void* params) { espConnect.loop(); });

static Mycila::Task networkStartTask("Network Start", Mycila::Task::Type::ONCE, [](void* params) {
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

    if (jsyRemoteTask)
      jsyRemoteTask->resume();

    if (mqttConnectTask)
      mqttConnectTask->resume();

    if (victronConnectTask) {
      victronConnectTask->resume();
    }
  }
});

void yasolr_init_network() {
  logger.info(TAG, "Initialize networking");

  // NTP
  Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

  // Network Manager
  Mycila::ESPConnect::Config espConnectConfig;
  espConnectConfig.hostname = config.getString(KEY_HOSTNAME);
  espConnectConfig.apMode = config.getBool(KEY_ENABLE_AP_MODE);
  espConnectConfig.wifiBSSID = config.getString(KEY_WIFI_BSSID);
  espConnectConfig.wifiSSID = config.getString(KEY_WIFI_SSID);
  espConnectConfig.wifiPassword = config.getString(KEY_WIFI_PASSWORD);
  espConnectConfig.ipConfig.ip.fromString(config.get(KEY_NET_IP));
  espConnectConfig.ipConfig.gateway.fromString(config.get(KEY_NET_GATEWAY));
  espConnectConfig.ipConfig.subnet.fromString(config.get(KEY_NET_SUBNET));
  espConnectConfig.ipConfig.dns.fromString(config.get(KEY_NET_DNS));
  espConnect.setAutoRestart(true);
  espConnect.setBlocking(false);
  espConnect.begin(Mycila::AppInfo.defaultHostname.c_str(), config.get(KEY_ADMIN_PASSWORD), espConnectConfig);

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
        logger.info(TAG, "Connected to network!");
        logger.info(TAG, "IP Address: %s", espConnect.getIPAddress().toString().c_str());
        logger.info(TAG, "Hostname: %s", espConnect.getConfig().hostname.c_str());
        if (espConnect.getWiFiSSID().length()) {
          logger.info(TAG, "WiFi SSID: %s", espConnect.getWiFiSSID().c_str());
        }
        networkStartTask.resume();
        break;
      case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
        logger.warn(TAG, "Unable to connect to any network!");
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
        if (espConnect.getConfig().apMode) {
          logger.info(TAG, "Captive Portal: Access Point configured");
          config.setBool(KEY_ENABLE_AP_MODE, true);
        } else {
          logger.info(TAG, "Captive Portal: WiFi configured");
          logger.info(TAG, "WiFi SSID: %s", espConnect.getConfig().wifiSSID.c_str());
          logger.info(TAG, "WiFi BSSID: %s", espConnect.getConfig().wifiBSSID.c_str());
          config.setBool(KEY_ENABLE_AP_MODE, false);
          config.set(KEY_WIFI_BSSID, espConnect.getConfig().wifiBSSID);
          config.set(KEY_WIFI_SSID, espConnect.getConfig().wifiSSID);
          config.set(KEY_WIFI_PASSWORD, espConnect.getConfig().wifiPassword);
        }
        break;
      }
      case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
        logger.warn(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
  });

  if (config.getBool(KEY_ENABLE_DEBUG))
    networkStartTask.enableProfiling();

  networkManagerTask.setInterval(200);

  coreTaskManager.addTask(networkStartTask);
  coreTaskManager.addTask(networkManagerTask);
}
