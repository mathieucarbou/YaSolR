// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::DS18* ds18O1 = nullptr;
Mycila::DS18* ds18O2 = nullptr;
Mycila::DS18* ds18Sys = nullptr;

void yasolr_configure_router_ds18() {
  if (config.get<bool>(KEY_ENABLE_SYSTEM_DS18)) {
    if (ds18Sys == nullptr) {
      ESP_LOGI(TAG, "Enable system DS18");

      ds18Sys = new Mycila::DS18();
      ds18Sys->begin(config.get<int8_t>(KEY_PIN_ROUTER_DS18), static_cast<uint8_t>(YASOLR_DS18_SEARCH_MAX_RETRY));

      if (ds18Sys->isEnabled()) {
        ds18Sys->listen([](float temperature, bool changed) {
          if (changed) {
            ESP_LOGI(TAG, "Router Temperature changed to %.02f °C", temperature);
            if (mqttPublishTask)
              mqttPublishTask->requestEarlyRun();
          }
        });
      } else {
        ESP_LOGE(TAG, "System DS18 failed to initialize!");
        delete ds18Sys;
        ds18Sys = nullptr;
      }
    }
  } else {
    if (ds18Sys != nullptr) {
      ESP_LOGI(TAG, "Disable system DS18");
      delete ds18Sys;
      ds18Sys = nullptr;
    }
  }
}

void yasolr_configure_output1_ds18() {
  if (config.get<bool>(KEY_ENABLE_OUTPUT1_DS18)) {
    if (ds18O1 == nullptr) {
      ESP_LOGI(TAG, "Enable output 1 DS18");

      ds18O1 = new Mycila::DS18();
      ds18O1->begin(config.get<int8_t>(KEY_PIN_OUTPUT1_DS18), static_cast<uint8_t>(YASOLR_DS18_SEARCH_MAX_RETRY));

      if (ds18O1->isEnabled()) {
        ds18O1->listen([](float temperature, bool changed) {
          if (temperature > 0) {
            // update the temperature in the output
            if (!output1.temperature().update(temperature).has_value()) {
              // if this is the first time we get the temperature, we can trigger the dashboard init task
              dashboardInitTask.resume();
            }

            if (changed) {
              ESP_LOGI(TAG, "Output 1 Temperature changed to %.02f °C", temperature);
              if (mqttPublishTask)
                mqttPublishTask->requestEarlyRun();
            }
          }
        });
      } else {
        ESP_LOGE(TAG, "Output 1 DS18 failed to initialize!");
        delete ds18O1;
        ds18O1 = nullptr;
      }
    }
  } else {
    if (ds18O1 != nullptr) {
      ESP_LOGI(TAG, "Disable output 1 DS18");
      delete ds18O1;
      ds18O1 = nullptr;
    }
  }
}

void yasolr_configure_output2_ds18() {
  if (config.get<bool>(KEY_ENABLE_OUTPUT2_DS18)) {
    if (ds18O2 == nullptr) {
      ESP_LOGI(TAG, "Enable output 2 DS18");

      ds18O2 = new Mycila::DS18();
      ds18O2->begin(config.get<int8_t>(KEY_PIN_OUTPUT2_DS18), static_cast<uint8_t>(YASOLR_DS18_SEARCH_MAX_RETRY));

      if (ds18O2->isEnabled()) {
        ds18O2->listen([](float temperature, bool changed) {
          if (temperature > 0) {
            // update the temperature in the output
            if (!output2.temperature().update(temperature).has_value()) {
              // if this is the first time we get the temperature, we can trigger the dashboard init task
              dashboardInitTask.resume();
            }

            if (changed) {
              ESP_LOGI(TAG, "Output 2 Temperature changed to %.02f °C", temperature);
              if (mqttPublishTask)
                mqttPublishTask->requestEarlyRun();
            }
          }
        });
      } else {
        ESP_LOGE(TAG, "Output 2 DS18 failed to initialize!");
        delete ds18O2;
        ds18O2 = nullptr;
      }
    }
  } else {
    if (ds18O2 != nullptr) {
      ESP_LOGI(TAG, "Disable output 2 DS18");
      delete ds18O2;
      ds18O2 = nullptr;
    }
  }
}

void yasolr_init_ds18() {
  ESP_LOGI(TAG, "Initialize DS18 reading task");

  Mycila::Task* ds18Task = new Mycila::Task("DS18", []() {
    if (ds18Sys) {
      ds18Sys->read();
      yield();
    }
    if (ds18O1) {
      ds18O1->read();
      yield();
    }
    if (ds18O2) {
      ds18O2->read();
      yield();
    }
  });

  ds18Task->setInterval(10000);

  if (config.get<bool>(KEY_ENABLE_DEBUG))
    ds18Task->enableProfiling();

  unsafeTaskManager.addTask(*ds18Task);
}
