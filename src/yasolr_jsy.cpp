// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::JSY* jsy = nullptr;
Mycila::TaskManager* jsyTaskManager = nullptr;

static Mycila::JSY::Data jsyData;

void yasolr_init_jsy() {
  if (config.getBool(KEY_ENABLE_JSY) && config.getString(KEY_JSY_UART) != YASOLR_UART_NONE) {
    logger.info(TAG, "Initialize JSY with UART %s", config.get(KEY_JSY_UART));

    jsy = new Mycila::JSY();

    if (config.getString(KEY_JSY_UART) == YASOLR_UART_1_NAME)
      jsy->begin(Serial1, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));

#if SOC_UART_NUM > 2
    if (config.getString(KEY_JSY_UART) == YASOLR_UART_2_NAME)
      jsy->begin(Serial2, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));
#endif

    if (!jsy->isEnabled()) {
      logger.error(TAG, "JSY failed to initialize!");
      jsy->end();
      delete jsy;
      jsy = nullptr;
      return;
    }

    if (jsy->getBaudRate() != jsy->getMaxAvailableBaudRate())
      jsy->setBaudRate(jsy->getMaxAvailableBaudRate());

    jsy->setCallback([](const Mycila::JSY::EventType eventType, const Mycila::JSY::Data& data) {
      if (jsyData != data) {
        jsyData = data;

        switch (data.model) {
          case MYCILA_JSY_MK_1031:
            // JSY1030 has no sign: it cannot be used to measure the grid
            break;

          case MYCILA_JSY_MK_163:
          case MYCILA_JSY_MK_227:
          case MYCILA_JSY_MK_229:
            grid.localMetrics().update({
              .apparentPower = data.single().apparentPower,
              .current = data.single().current,
              .energy = data.single().activeEnergyImported,
              .energyReturned = data.single().activeEnergyReturned,
              .frequency = data.single().frequency,
              .power = data.single().activePower,
              .powerFactor = data.single().powerFactor,
              .voltage = data.single().voltage,
            });
            break;

          case MYCILA_JSY_MK_193:
          case MYCILA_JSY_MK_194:
            grid.localMetrics().update({
              .apparentPower = data.channel2().apparentPower,
              .current = data.channel2().current,
              .energy = data.channel2().activeEnergyImported,
              .energyReturned = data.channel2().activeEnergyReturned,
              .frequency = data.aggregate.frequency,
              .power = data.channel2().activePower,
              .powerFactor = data.channel2().powerFactor,
              .voltage = data.channel2().voltage,
            });
            router.localMetrics().update({
              .apparentPower = data.channel1().apparentPower,
              .current = data.channel1().current,
              .energy = data.channel1().activeEnergy,
              .power = data.channel1().activePower,
              .powerFactor = data.channel1().powerFactor,
              .resistance = data.channel1().resistance(),
              .thdi = data.channel1().thdi(),
              .voltage = data.channel1().voltage,
            });
            break;

          case MYCILA_JSY_MK_333:
            grid.localMetrics().update({
              .apparentPower = data.aggregate.apparentPower,
              .current = data.aggregate.current,
              .energy = data.aggregate.activeEnergyImported,
              .energyReturned = data.aggregate.activeEnergyReturned,
              .frequency = data.aggregate.frequency,
              .power = data.aggregate.activePower,
              .powerFactor = data.aggregate.powerFactor,
              .voltage = data.aggregate.voltage,
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

    Mycila::Task* jsyTask = new Mycila::Task("JSY", [](void* params) { jsy->read(); });

    jsyTaskManager = new Mycila::TaskManager("y-jsy");
    jsyTaskManager->addTask(*jsyTask);

    if (config.getBool(KEY_ENABLE_DEBUG)) {
      jsyTaskManager->enableProfiling();
    }

    assert(jsyTaskManager->asyncStart(512 * 6, 5, 0, 100, true));

    Mycila::TaskMonitor.addTask(jsyTaskManager->name());
  }
}
