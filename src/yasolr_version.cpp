// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <HTTPClient.h>
#include <NetworkClientSecure.h>

Mycila::Task versionCheckTask("Version Check", [](void* params) {
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
      if (https->getStream().find("\"tag_name\":")) {
        https->getStream().readStringUntil('v');
        std::string v = https->getStream().readStringUntil('"').c_str();
        if (v.length()) {
          Mycila::AppInfo.latestVersion = std::move(v);
          ESP_LOGI(TAG, "Latest YaSolR version: %s", Mycila::AppInfo.latestVersion.c_str());
          dashboardInitTask.resume();
        } else {
          ESP_LOGE(TAG, "Failed to find latest version in response");
        }
      } else {
        ESP_LOGE(TAG, "Failed to parse HTTP response");
      }
    } else {
      ESP_LOGE(TAG, "Failed HTTP GET %d %s", status, https->errorToString(status).c_str());
    }
  } else {
    ESP_LOGE(TAG, "Failed to begin HTTPS connection");
  }

  https->end();
  delete https;
  delete client;
});

void yasolr_init_version_check() {
  LOGI(TAG, "Initialize version check");
  versionCheckTask.setEnabled(false);
  versionCheckTask.setInterval(3600000);
  unsafeTaskManager.addTask(versionCheckTask);
}
