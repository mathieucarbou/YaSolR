// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::RouterRelay* relay1 = nullptr;
Mycila::RouterRelay* relay2 = nullptr;
Mycila::Task* relayTask = nullptr;

void yasolr_init_relays() {
  logger.info(TAG, "Initialize relays...");
  uint8_t count = 0;

  if (config.getBool(KEY_ENABLE_RELAY1)) {
    assert(!relay1);

    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      count++;

      relay1 = new Mycila::RouterRelay(*relay);
      relay1->setLoad(config.getLong(KEY_RELAY1_LOAD));

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
    assert(!relay2);

    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      count++;

      relay2 = new Mycila::RouterRelay(*relay);
      relay2->setLoad(config.getLong(KEY_RELAY2_LOAD));

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
    relayTask = new Mycila::Task("Relay", [](void* params) {
      if (grid.getPower().isAbsent())
        return;

      Mycila::Router::Metrics routerMetrics;
      router.getRouterMeasurements(routerMetrics);

      float virtualGridPower = grid.getPower().get() - routerMetrics.power;

      if (relay1 && relay1->autoSwitch(virtualGridPower))
        return;

      if (relay2 && relay2->autoSwitch(virtualGridPower))
        return;
    });

    relayTask->setEnabledWhen([]() { return !router.isCalibrationRunning() && ((relay1 && relay1->isAutoRelayEnabled()) || (relay2 && relay2->isAutoRelayEnabled())); });
    relayTask->setInterval(7 * Mycila::TaskDuration::SECONDS);
    relayTask->setManager(coreTaskManager);
    if (config.getBool(KEY_ENABLE_DEBUG))
      relayTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  }
}
