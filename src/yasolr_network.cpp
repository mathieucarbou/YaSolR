// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

static Mycila::Task networkManagerTask("ESPConnect", []() { espConnect.loop(); });

static Mycila::Task networkStartTask("Network Start", Mycila::Task::Type::ONCE, []() {
  ESP_LOGI(TAG, "Enable Network Services");

  // Web server
  ESP_LOGI(TAG, "Enable Web Server");
  webServer.begin();
  webServer.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404);
  });

  if (!config.get<bool>(KEY_ENABLE_AP_MODE)) {
    // NTP
    ESP_LOGI(TAG, "Enable NTP");
    Mycila::NTP.sync(config.getString(KEY_NTP_SERVER));

    // mDNS
    ESP_LOGI(TAG, "Enable mDNS");
    MDNS.addService("http", "tcp", 80);

    if (jsyRemoteTask)
      jsyRemoteTask->resume();

    if (victronConnectTask) {
      victronConnectTask->resume();
    }

    if (mqttConnectTask)
      mqttConnectTask->resume(10000); // delay mqtt startup by 10 seconds to let other services start first in order to avoid too much memory consumption at once
  }
});

void yasolr_init_network() {
  ESP_LOGI(TAG, "Initialize networking");

  // NTP
  Mycila::NTP.setTimeZone(config.getString(KEY_NTP_TIMEZONE));

  // Network Manager
  Mycila::ESPConnect::Config espConnectConfig;
  espConnectConfig.hostname = config.getString(KEY_HOSTNAME);
  espConnectConfig.apMode = config.get<bool>(KEY_ENABLE_AP_MODE);
  espConnectConfig.wifiBSSID = config.getString(KEY_WIFI_BSSID);
  espConnectConfig.wifiSSID = config.getString(KEY_WIFI_SSID);
  espConnectConfig.wifiPassword = config.getString(KEY_WIFI_PASSWORD);
  espConnectConfig.ipConfig.ip.fromString(config.getString(KEY_NET_IP));
  espConnectConfig.ipConfig.gateway.fromString(config.getString(KEY_NET_GATEWAY));
  espConnectConfig.ipConfig.subnet.fromString(config.getString(KEY_NET_SUBNET));
  espConnectConfig.ipConfig.dns.fromString(config.getString(KEY_NET_DNS));
  espConnect.setAutoRestart(true);
  espConnect.setBlocking(false);
  espConnect.begin(Mycila::AppInfo.defaultHostname.c_str(), config.getString(KEY_ADMIN_PASSWORD), espConnectConfig);

  espConnect.listen([](Mycila::ESPConnect::State previous, Mycila::ESPConnect::State state) {
    ESP_LOGI(TAG, "NetworkState: %s => %s", espConnect.getStateName(previous), espConnect.getStateName(state));
    switch (state) {
      case Mycila::ESPConnect::State::NETWORK_DISABLED:
        ESP_LOGW(TAG, "Disabled Network!");
        break;
      case Mycila::ESPConnect::State::AP_STARTING:
        ESP_LOGI(TAG, "Starting Access Point %s", espConnect.getAccessPointSSID().c_str());
        break;
      case Mycila::ESPConnect::State::AP_STARTED:
        ESP_LOGI(TAG, "Access Point %s started with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        networkStartTask.resume();
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTING:
        ESP_LOGI(TAG, "Connecting to network");
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTED:
        ESP_LOGI(TAG, "Connected to network!");
        ESP_LOGI(TAG, "IP Address: %s", espConnect.getIPAddress().toString().c_str());
        ESP_LOGI(TAG, "Hostname: %s", espConnect.getConfig().hostname.c_str());
        if (espConnect.getWiFiSSID().length()) {
          ESP_LOGI(TAG, "WiFi SSID: %s", espConnect.getWiFiSSID().c_str());
        }
        networkStartTask.resume();
        versionCheckTask.resume(60000); // delay version check by 1 minute after network is connected to avoid heap consumption peak at startup
        break;
      case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
        ESP_LOGW(TAG, "Unable to connect to any network!");
        break;
      case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
        ESP_LOGW(TAG, "Disconnected!");
        break;
      case Mycila::ESPConnect::State::NETWORK_RECONNECTING:
        ESP_LOGI(TAG, "Trying to reconnect");
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTING:
        ESP_LOGI(TAG, "Starting Captive Portal %s for %" PRIu32 " seconds", espConnect.getAccessPointSSID().c_str(), espConnect.getCaptivePortalTimeout());
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTED:
        ESP_LOGI(TAG, "Captive Portal started at %s", espConnect.getWiFiSSID().c_str());
        break;
      case Mycila::ESPConnect::State::PORTAL_COMPLETE: {
        if (espConnect.getConfig().apMode) {
          ESP_LOGI(TAG, "Captive Portal: Access Point configured");
          config.set<bool>(KEY_ENABLE_AP_MODE, true);
        } else {
          ESP_LOGI(TAG, "Captive Portal: WiFi configured");
          ESP_LOGI(TAG, "WiFi SSID: %s", espConnect.getConfig().wifiSSID.c_str());
          config.set<bool>(KEY_ENABLE_AP_MODE, false);
          config.setString(KEY_WIFI_SSID, espConnect.getConfig().wifiSSID);
          config.setString(KEY_WIFI_PASSWORD, espConnect.getConfig().wifiPassword);
        }
        break;
      }
      case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
        ESP_LOGW(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
  });

  if (config.get<bool>(KEY_ENABLE_DEBUG))
    networkStartTask.enableProfiling();

  networkManagerTask.setInterval(200);

  coreTaskManager.addTask(networkStartTask);
  coreTaskManager.addTask(networkManagerTask);
}
