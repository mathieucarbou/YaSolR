// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::RouterRelay* relay1 = nullptr;
Mycila::RouterRelay* relay2 = nullptr;

void yasolr_configure_relay1() {
  if (config.getBool(KEY_ENABLE_RELAY1)) {
    if (relay1 == nullptr) {
      LOGI(TAG, "Enable Relay 1");
      relay1 = new Mycila::RouterRelay();
      relay1->relay().begin(config.getLong(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
      if (relay1->relay().isEnabled()) {
        relay1->setNominalLoad(config.getLong(KEY_RELAY1_LOAD));
        relay1->setTolerance(config.getFloat(KEY_RELAY1_TOLERANCE) / 100.0f);
        relay1->relay().listen([](bool state) {
          LOGI(TAG, "Relay 1 changed to %s", state ? "ON" : "OFF");
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        });
      } else {
        LOGE(TAG, "Relay 1 failed to initialize!");
        relay1->relay().end();
        delete relay1;
        relay1 = nullptr;
      }
    }
  } else {
    if (relay1 != nullptr) {
      LOGI(TAG, "Disable Relay 1");
      relay1->relay().end();
      delete relay1;
      relay1 = nullptr;
    }
  }
}

void yasolr_configure_relay2() {
  if (config.getBool(KEY_ENABLE_RELAY2)) {
    if (relay2 == nullptr) {
      LOGI(TAG, "Enable Relay 2");
      relay2 = new Mycila::RouterRelay();
      relay2->relay().begin(config.getLong(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
      if (relay2->relay().isEnabled()) {
        relay2->setNominalLoad(config.getLong(KEY_RELAY2_LOAD));
        relay2->setTolerance(config.getFloat(KEY_RELAY2_TOLERANCE) / 100.0f);
        relay2->relay().listen([](bool state) {
          LOGI(TAG, "Relay 2 changed to %s", state ? "ON" : "OFF");
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        });
      } else {
        LOGE(TAG, "Relay 2 failed to initialize!");
        relay2->relay().end();
        delete relay2;
        relay2 = nullptr;
      }
    }
  } else {
    if (relay2 != nullptr) {
      LOGI(TAG, "Disable Relay 2");
      relay2->relay().end();
      delete relay2;
      relay2 = nullptr;
    }
  }
}

void yasolr_init_relays() {
  LOGI(TAG, "Initialize relays");

  Mycila::Task* relayTask = new Mycila::Task("Relay", [](void* params) {
    float gridPower = grid.getPower().value_or(NAN);
    float gridVoltage = grid.getVoltage().value_or(NAN);
    float setpoint = pidController.getSetPoint();

    if (isnan(gridPower) || isnan(gridVoltage) || !gridVoltage) {
      LOGW(TAG, "Cannot auto switch relays: missing grid power/voltage");
      return;
    }

    Mycila::Router::Metrics routerMetrics;
    router.getRouterMeasurements(routerMetrics);

    if (relay1 && relay1->autoSwitch(gridVoltage, gridPower, routerMetrics.power, setpoint))
      return;

    if (relay2 && relay2->autoSwitch(gridVoltage, gridPower, routerMetrics.power, setpoint))
      return;
  });

  relayTask->setEnabledWhen([]() { return ((relay1 && relay1->isAutoRelayEnabled()) || (relay2 && relay2->isAutoRelayEnabled())) && !router.isCalibrationRunning() && router.isAutoDimmerEnabled(); });
  relayTask->setInterval(10000);

  if (config.getBool(KEY_ENABLE_DEBUG))
    relayTask->enableProfiling();

  coreTaskManager.addTask(*relayTask);
}
