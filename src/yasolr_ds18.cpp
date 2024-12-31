// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::DS18* ds18O1;
Mycila::DS18* ds18O2;
Mycila::DS18* ds18Sys;
Mycila::Task* ds18Task;

void yasolr_init_ds18() {
  logger.info(TAG, "Initialize DS18 probes...");
  uint8_t count = 0;

  if (config.getBool(KEY_ENABLE_DS18_SYSTEM)) {
    assert(!ds18Sys);

    ds18Sys = new Mycila::DS18();
    ds18Sys->begin(config.getLong(KEY_PIN_ROUTER_DS18));

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
    ds18O1->begin(config.getLong(KEY_PIN_OUTPUT1_DS18));

    if (ds18O1->isEnabled()) {
      count++;
      ds18O1->listen([](float temperature, bool changed) {
        if (output1)
          output1->temperature().update(temperature);
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
    ds18O2->begin(config.getLong(KEY_PIN_OUTPUT2_DS18));

    if (ds18O2->isEnabled()) {
      count++;
      ds18O2->listen([](float temperature, bool changed) {
        if (output2)
          output2->temperature().update(temperature);
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
    ds18Task->setInterval(10 * Mycila::TaskDuration::SECONDS);
    ds18Task->setManager(coreTaskManager);
    if (config.getBool(KEY_ENABLE_DEBUG))
      ds18Task->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  }
}
