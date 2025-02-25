// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::DS18* ds18O1 = nullptr;
Mycila::DS18* ds18O2 = nullptr;
Mycila::DS18* ds18Sys = nullptr;

static Mycila::Task* ds18Task = nullptr;

void yasolr_init_ds18() {
  logger.info(TAG, "Initialize DS18 probes");
  uint8_t count = 0;

  if (config.getBool(KEY_ENABLE_DS18_SYSTEM)) {
    assert(!ds18Sys);

    ds18Sys = new Mycila::DS18();
    ds18Sys->begin(config.getLong(KEY_PIN_ROUTER_DS18), YASOLR_DS18_SEARCH_MAX_RETRY);

    if (ds18Sys->isEnabled()) {
      count++;
      ds18Sys->listen([](float temperature, bool changed) {
        if (changed) {
          logger.info(TAG, "Router Temperature changed to %.02f °C", temperature);
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        }
      });
    } else {
      logger.error(TAG, "DS18 system probe failed to initialize!");
      ds18Sys->end();
      delete ds18Sys;
      ds18Sys = nullptr;
    }
  }

  if (config.getBool(KEY_ENABLE_OUTPUT1_DS18)) {
    assert(!ds18O1);

    ds18O1 = new Mycila::DS18();
    ds18O1->begin(config.getLong(KEY_PIN_OUTPUT1_DS18), YASOLR_DS18_SEARCH_MAX_RETRY);

    if (ds18O1->isEnabled()) {
      count++;
      ds18O1->listen([](float temperature, bool changed) {
        if (output1) {
          // update the temperature in the output
          if (!output1->temperature().update(temperature).has_value()) {
            // if this is the first time we get the temperature, we can trigger the dashboard init task
            dashboardInitTask.resume();
          }
        }

        if (changed) {
          logger.info(TAG, "Output 1 Temperature changed to %.02f °C", temperature);
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        }
      });
    } else {
      logger.error(TAG, "DS18 output 1 probe failed to initialize!");
      ds18O1->end();
      delete ds18O1;
      ds18O1 = nullptr;
    }
  }

  if (config.getBool(KEY_ENABLE_OUTPUT2_DS18)) {
    assert(!ds18O2);

    ds18O2 = new Mycila::DS18();
    ds18O2->begin(config.getLong(KEY_PIN_OUTPUT2_DS18), YASOLR_DS18_SEARCH_MAX_RETRY);

    if (ds18O2->isEnabled()) {
      count++;
      ds18O2->listen([](float temperature, bool changed) {
        if (output2) {
          // update the temperature in the output
          if (!output2->temperature().update(temperature).has_value()) {
            // if this is the first time we get the temperature, we can trigger the dashboard init task
            dashboardInitTask.resume();
          }
        }

        if (changed) {
          logger.info(TAG, "Output 2 Temperature changed to %.02f °C", temperature);
          if (mqttPublishTask)
            mqttPublishTask->requestEarlyRun();
        }
      });
      ds18O2 = new Mycila::DS18();
    } else {
      logger.error(TAG, "DS18 output 2 probe failed to initialize!");
      ds18O2->end();
      delete ds18O2;
      ds18O2 = nullptr;
    }
  }

  if (count) {
    ds18Task = new Mycila::Task("DS18", [](void* params) {
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
    if (config.getBool(KEY_ENABLE_DEBUG))
      ds18Task->enableProfiling();

    unsafeTaskManager.addTask(*ds18Task);
  }
}
