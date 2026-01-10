// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::Router::Relay* relay1 = nullptr;
Mycila::Router::Relay* relay2 = nullptr;

void yasolr_configure_relay1() {
  if (config.get<bool>(KEY_ENABLE_RELAY1)) {
    if (relay1 == nullptr) {
      ESP_LOGI(TAG, "Enable Relay 1");
      relay1 = new Mycila::Router::Relay();
      relay1->relay().begin(config.get<int8_t>(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
      if (relay1->relay().isEnabled()) {
        relay1->setNominalLoad(config.get<uint16_t>(KEY_RELAY1_LOAD));
        relay1->setTolerance(config.get<uint8_t>(KEY_RELAY1_TOLERANCE) / 100.0f);
        relay1->relay().listen([](bool state) {
          ESP_LOGI(TAG, "Relay 1 changed to %s", state ? "ON" : "OFF");
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        });
      } else {
        ESP_LOGE(TAG, "Relay 1 failed to initialize!");
        relay1->relay().end();
        delete relay1;
        relay1 = nullptr;
      }
    }
  } else {
    if (relay1 != nullptr) {
      ESP_LOGI(TAG, "Disable Relay 1");
      relay1->relay().end();
      delete relay1;
      relay1 = nullptr;
    }
  }
}

void yasolr_configure_relay2() {
  if (config.get<bool>(KEY_ENABLE_RELAY2)) {
    if (relay2 == nullptr) {
      ESP_LOGI(TAG, "Enable Relay 2");
      relay2 = new Mycila::Router::Relay();
      relay2->relay().begin(config.get<int8_t>(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
      if (relay2->relay().isEnabled()) {
        relay2->setNominalLoad(config.get<uint16_t>(KEY_RELAY2_LOAD));
        relay2->setTolerance(config.get<uint8_t>(KEY_RELAY2_TOLERANCE) / 100.0f);
        relay2->relay().listen([](bool state) {
          ESP_LOGI(TAG, "Relay 2 changed to %s", state ? "ON" : "OFF");
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        });
      } else {
        ESP_LOGE(TAG, "Relay 2 failed to initialize!");
        relay2->relay().end();
        delete relay2;
        relay2 = nullptr;
      }
    }
  } else {
    if (relay2 != nullptr) {
      ESP_LOGI(TAG, "Disable Relay 2");
      relay2->relay().end();
      delete relay2;
      relay2 = nullptr;
    }
  }
}

void yasolr_init_relays() {
  ESP_LOGI(TAG, "Initialize relays");

  Mycila::Task* relayTask = new Mycila::Task("Relay", []() {
    std::optional<float> gridPower = grid.getPower();
    std::optional<float> gridVoltage = grid.getVoltage();
    std::optional<float> routedPower = router.readTotalRoutedPower();
    float setpoint = pidController.getSetpoint();

    if (!routedPower.has_value() && gridVoltage.has_value()) {
      routedPower = router.computeTotalRoutedPower(gridVoltage.value());
    }

    if (!gridPower.has_value() || !gridVoltage.has_value() || !routedPower.has_value()) {
      ESP_LOGW(TAG, "Cannot auto switch relays: missing grid power, grid voltage or routed power");
      return;
    }

    if (relay1 && relay1->autoSwitch(gridVoltage.value(), gridPower.value(), routedPower.value(), setpoint))
      return;

    if (relay2 && relay2->autoSwitch(gridVoltage.value(), gridPower.value(), routedPower.value(), setpoint))
      return;
  });

  relayTask->setEnabledWhen([]() { return ((relay1 && relay1->isAutoRelayEnabled()) || (relay2 && relay2->isAutoRelayEnabled())) && !router.isCalibrationRunning() && router.isAutoDimmerEnabled(); });
  relayTask->setInterval(10000);

  if (config.get<bool>(KEY_ENABLE_DEBUG))
    relayTask->enableProfiling();

  coreTaskManager.addTask(*relayTask);
}
