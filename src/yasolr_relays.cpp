// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::Router::Relay* relay1 = nullptr;
Mycila::Router::Relay* relay2 = nullptr;

void yasolr_configure_relay1() {
  if (!config.isEmpty(KEY_RELAY1)) {
    if (relay1 == nullptr) {
      ESP_LOGI(TAG, "Enable Relay 1");
      relay1 = new Mycila::Router::Relay();
      relay1->relay().begin(config.get<int8_t>(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1, YASOLR_RELAY_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
      if (relay1->relay().isEnabled()) {
        relay1->setNominalLoad(config.get<uint16_t>(KEY_RELAY1_LOAD));
        relay1->setTolerance(config.get<uint8_t>(KEY_RELAY1_TOLERANCE) / 100.0f);
        if (config.isEqual(KEY_RELAY1_AUTO, "Output 1")) relay1->setAutoMode(Mycila::Router::Relay::AutoMode::OUTPUT_1);
        else if (config.isEqual(KEY_RELAY1_AUTO, "Output 2")) relay1->setAutoMode(Mycila::Router::Relay::AutoMode::OUTPUT_2);
        else if (config.isEqual(KEY_RELAY1_AUTO, "Any Output")) relay1->setAutoMode(Mycila::Router::Relay::AutoMode::OUTPUT_ANY);
        else relay1->setAutoMode(Mycila::Router::Relay::AutoMode::OFF);
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
  if (!config.isEmpty(KEY_RELAY2)) {
    if (relay2 == nullptr) {
      ESP_LOGI(TAG, "Enable Relay 2");
      relay2 = new Mycila::Router::Relay();
      relay2->relay().begin(config.get<int8_t>(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2, YASOLR_RELAY_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
      if (relay2->relay().isEnabled()) {
        relay2->setNominalLoad(config.get<uint16_t>(KEY_RELAY2_LOAD));
        relay2->setTolerance(config.get<uint8_t>(KEY_RELAY2_TOLERANCE) / 100.0f);
        if (config.isEqual(KEY_RELAY2_AUTO, "Output 1")) relay2->setAutoMode(Mycila::Router::Relay::AutoMode::OUTPUT_1);
        else if (config.isEqual(KEY_RELAY2_AUTO, "Output 2")) relay2->setAutoMode(Mycila::Router::Relay::AutoMode::OUTPUT_2);
        else if (config.isEqual(KEY_RELAY2_AUTO, "Any Output")) relay2->setAutoMode(Mycila::Router::Relay::AutoMode::OUTPUT_ANY);
        else relay2->setAutoMode(Mycila::Router::Relay::AutoMode::OFF);
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

    if (!gridPower.has_value() || !gridVoltage.has_value()) {
      ESP_LOGW(TAG, "Cannot auto switch relays: missing grid power or grid voltage");
      return;
    }

    std::optional<float> totalRoutedPower = router.getTotalRoutedPower(gridVoltage.value());

    // room is the power that is available for a relay to switch on, and depends on the relay's auto mode:
    // * Output 1 / Output 2: room is the power currently routed to that specific output, compared against the relay's own nominal load
    // * Any Output: room is the excess power available on the grid, considering the PID setpoint and the total routed power
    auto room = [&](Mycila::Router::Relay::AutoMode mode) -> std::optional<float> {
      switch (mode) {
        case Mycila::Router::Relay::AutoMode::OUTPUT_1:
          if (!output1.isAutoDimmerEnabled())
            return std::nullopt;
          return output1.getRoutedPower(gridVoltage.value());
        case Mycila::Router::Relay::AutoMode::OUTPUT_2:
          if (!output2.isAutoDimmerEnabled())
            return std::nullopt;
          return output2.getRoutedPower(gridVoltage.value());
        case Mycila::Router::Relay::AutoMode::OUTPUT_ANY:
          if (!totalRoutedPower.has_value())
            return std::nullopt;
          return pidController.getSetpoint() + totalRoutedPower.value() - gridPower.value();
        default:
          return std::nullopt;
      }
    };

    // only switch one relay per round, to leave time for measurement devices to pick up the change
    if (relay1) {
      std::optional<float> relay1Room = room(relay1->getAutoMode());
      if (relay1Room.has_value() && relay1->autoSwitch(gridVoltage.value(), relay1Room.value()))
        return;
    }

    if (relay2) {
      std::optional<float> relay2Room = room(relay2->getAutoMode());
      if (relay2Room.has_value() && relay2->autoSwitch(gridVoltage.value(), relay2Room.value()))
        return;
    }
  });

  relayTask->setEnabledWhen([]() {
    return ((relay1 && relay1->isAutoRelayEnabled()) || (relay2 && relay2->isAutoRelayEnabled())) && !router.isCalibrationRunning() && router.isAutoDimmerEnabled();
  });
  relayTask->setInterval(config.get<uint16_t>(KEY_RELAY_CHECK_INTERVAL) * 1000);

  if (config.get<bool>(KEY_ENABLE_DEBUG))
    relayTask->enableProfiling();

  coreTaskManager.addTask(*relayTask);
}
