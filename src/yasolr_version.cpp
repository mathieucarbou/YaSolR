// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <HTTPClient.h>
#include <NetworkClientSecure.h>

#include <string>
#include <utility>

static Mycila::Task versionCheckTask("Version Check", []() {
  ESP_LOGI(TAG, "Checking latest YaSolR version...");

  NetworkClientSecure* client = new NetworkClientSecure();
  HTTPClient* https = new HTTPClient();

  // Skip SSL certificate validation
  client->setInsecure();

  if (https->begin(*client, "https://api.github.com/repos/mathieucarbou/YaSolR/releases/latest")) {
    https->addHeader("Accept", "application/vnd.github+json");
    https->setConnectTimeout(5000);
    https->setTimeout(5000);
    const int status = https->GET();

    if (status == HTTP_CODE_OK) {
      // OK
      if (https->getStream().find("\"tag_name\":")) {
        // found tag_name
        https->getStream().readStringUntil('v');
        std::string latestVersion = https->getStream().readStringUntil('"').c_str();
        if (latestVersion.length()) {
          // found latest version
          if (Mycila::AppInfo.latestVersion != latestVersion) {
            Mycila::AppInfo.latestVersion = std::move(latestVersion);
            ESP_LOGD(TAG, "Successfully read latest version from GitHub: %s", Mycila::AppInfo.latestVersion.c_str());
            dashboardInitTask.resume();
          }
          if (Mycila::AppInfo.isOutdated()) {
            ESP_LOGW(TAG, "A new version of YaSolR is available: %s (current: %s)", Mycila::AppInfo.latestVersion.c_str(), Mycila::AppInfo.version.c_str());
          } else {
            ESP_LOGI(TAG, "YaSolR is up to date (version: %s)", Mycila::AppInfo.version.c_str());
          }
        } else {
          // cannot find version
          ESP_LOGE(TAG, "Failed to find latest version in response");
        }
      } else {
        // cannot find tag_name
        ESP_LOGE(TAG, "Failed to parse HTTP response");
      }
    } else {
      // HTTP error
      ESP_LOGE(TAG, "Failed HTTP GET %d %s", status, https->errorToString(status).c_str());
    }
  } else {
    // cannot begin HTTPS
    ESP_LOGE(TAG, "Failed to begin HTTPS connection");
  }

  https->end();
  delete https;
  delete client;
});

void yasolr_init_version_check() {
  ESP_LOGI(TAG, "Initialize version check");

  versionCheckTask.setEnabledWhen([]() { return espConnect.getState() == Mycila::ESPConnect::State::NETWORK_CONNECTED; });
  versionCheckTask.setInterval(3600000); // check every hour

  if (config.get<bool>(KEY_ENABLE_DEBUG))
    versionCheckTask.enableProfiling();

  unsafeTaskManager.addTask(versionCheckTask);
}
