// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::JSY* jsy = nullptr;
Mycila::TaskManager* jsyTaskManager = nullptr;

static Mycila::Task* jsyTask = nullptr;

void yasolr_init_jsy() {
  if (config.getBool(KEY_ENABLE_JSY)) {
    assert(!jsy);
    assert(!jsyTask);
    assert(!jsyTaskManager);

    logger.info(TAG, "Initialize JSY...");

    jsy = new Mycila::JSY();
    jsy->begin(YASOLR_JSY_SERIAL, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));

    if (!jsy->isEnabled()) {
      logger.error(TAG, "JSY failed to initialize!");
      jsy->end();
      delete jsy;
      jsy = nullptr;
      return;
    }

    if (jsy->getBaudRate() != jsy->getMaxAvailableBaudRate())
      jsy->setBaudRate(jsy->getMaxAvailableBaudRate());

    jsy->setCallback([](const Mycila::JSY::EventType eventType) {
      if (eventType == Mycila::JSY::EventType::EVT_CHANGE) {
        switch (jsy->data.model) {
          case MYCILA_JSY_MK_1031:
            // JSY1030 has no sign: it cannot be used to measure the grid
            break;

          case MYCILA_JSY_MK_163:
          case MYCILA_JSY_MK_227:
          case MYCILA_JSY_MK_229:
            grid.localMetrics().update({
              .apparentPower = jsy->data.single().apparentPower,
              .current = jsy->data.single().current,
              .energy = jsy->data.single().activeEnergyImported,
              .energyReturned = jsy->data.single().activeEnergyReturned,
              .frequency = jsy->data.single().frequency,
              .power = jsy->data.single().activePower,
              .powerFactor = jsy->data.single().powerFactor,
              .voltage = jsy->data.single().voltage,
            });
            break;

          case MYCILA_JSY_MK_193:
          case MYCILA_JSY_MK_194:
            grid.localMetrics().update({
              .apparentPower = jsy->data.channel2().apparentPower,
              .current = jsy->data.channel2().current,
              .energy = jsy->data.channel2().activeEnergyImported,
              .energyReturned = jsy->data.channel2().activeEnergyReturned,
              .frequency = jsy->data.aggregate.frequency,
              .power = jsy->data.channel2().activePower,
              .powerFactor = jsy->data.channel2().powerFactor,
              .voltage = jsy->data.channel2().voltage,
            });
            router.localMetrics().update({
              .apparentPower = jsy->data.channel1().apparentPower,
              .current = jsy->data.channel1().current,
              .energy = jsy->data.channel1().activeEnergy,
              .power = jsy->data.channel1().activePower,
              .powerFactor = jsy->data.channel1().powerFactor,
              .resistance = jsy->data.channel1().resistance(),
              .thdi = jsy->data.channel1().thdi(),
              .voltage = jsy->data.channel1().voltage,
            });
            break;

          case MYCILA_JSY_MK_333:
            grid.localMetrics().update({
              .apparentPower = jsy->data.aggregate.apparentPower,
              .current = jsy->data.aggregate.current,
              .energy = jsy->data.aggregate.activeEnergyImported,
              .energyReturned = jsy->data.aggregate.activeEnergyReturned,
              .frequency = jsy->data.aggregate.frequency,
              .power = jsy->data.aggregate.activePower,
              .powerFactor = jsy->data.aggregate.powerFactor,
              .voltage = jsy->data.aggregate.voltage,
            });
            break;

          default:
            break;
        }
        if (grid.updatePower()) {
          yasolr_divert();
        }
      }
    });

    // async task

    jsyTask = new Mycila::Task("JSY", [](void* params) { jsy->read(); });

    jsyTaskManager = new Mycila::TaskManager("y-jsy");
    jsyTaskManager->addTask(*jsyTask);

    if (config.getBool(KEY_ENABLE_DEBUG)) {
      jsyTaskManager->enableProfiling();
    }

    assert(jsyTaskManager->asyncStart(512 * 6, 5, 0, 100, true));

    Mycila::TaskMonitor.addTask(jsyTaskManager->name());
  }
}
