// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::RouterRelay* relay1 = nullptr;
Mycila::RouterRelay* relay2 = nullptr;

void yasolr_init_relays() {
  logger.info(TAG, "Initialize relays");
  uint8_t count = 0;

  if (config.getBool(KEY_ENABLE_RELAY1)) {
    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      count++;

      relay1 = new Mycila::RouterRelay(*relay);
      relay1->setNominalLoad(config.getLong(KEY_RELAY1_LOAD));
      relay1->setTolerance(config.getFloat(KEY_RELAY1_TOLERANCE) / 100.0f);

      relay->listen([](bool state) {
        logger.info(TAG, "Relay 1 changed to %s", state ? "ON" : "OFF");
        if (mqttPublishTask)
          mqttPublishTask->requestEarlyRun();
      });

    } else {
      logger.error(TAG, "Relay 1 failed to initialize!");
      relay->end();
      delete relay1;
      relay1 = nullptr;
    }
  }

  if (config.getBool(KEY_ENABLE_RELAY2)) {
    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      count++;

      relay2 = new Mycila::RouterRelay(*relay);
      relay2->setNominalLoad(config.getLong(KEY_RELAY2_LOAD));
      relay2->setTolerance(config.getFloat(KEY_RELAY2_TOLERANCE) / 100.0f);

      relay->listen([](bool state) {
        logger.info(TAG, "Relay 2 changed to %s", state ? "ON" : "OFF");
        if (mqttPublishTask)
          mqttPublishTask->requestEarlyRun();
      });

    } else {
      logger.error(TAG, "Relay 2 failed to initialize!");
      relay->end();
      delete relay2;
      relay2 = nullptr;
    }
  }

  if (count) {
    Mycila::Task* relayTask = new Mycila::Task("Relay", [](void* params) {
      float gridPower = grid.getPower().value_or(NAN);
      float gridVoltage = grid.getVoltage().value_or(NAN);
      float setpoint = pidController.getSetPoint();

      if (isnan(gridPower) || isnan(gridVoltage) || !gridVoltage) {
        return;
      }

      Mycila::Router::Metrics routerMetrics;
      router.getRouterMeasurements(routerMetrics);

      if (relay1 && relay1->autoSwitch(gridVoltage, gridPower, routerMetrics.power, setpoint))
        return;

      if (relay2 && relay2->autoSwitch(gridVoltage, gridPower, routerMetrics.power, setpoint))
        return;
    });

    relayTask->setEnabledWhen([]() { return !router.isCalibrationRunning() && ((relay1 && relay1->isAutoRelayEnabled()) || (relay2 && relay2->isAutoRelayEnabled())) && router.isAutoDimmerEnabled(); });
    relayTask->setInterval(10000);
    if (config.getBool(KEY_ENABLE_DEBUG))
      relayTask->enableProfiling();

    coreTaskManager.addTask(*relayTask);
  }
}
