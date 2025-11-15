// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::JSY* jsy = nullptr;
Mycila::TaskManager* jsyTaskManager = nullptr;

static Mycila::Task* jsyTask;
static Mycila::JSY::Data* jsyData;

static void jsy_callback(const Mycila::JSY::EventType eventType, const Mycila::JSY::Data& data) {
  if (*jsyData != data) {
    *jsyData = data;

    switch (data.model) {
      case MYCILA_JSY_MK_1031:
        // JSY1030 has no sign: it cannot be used to measure the grid
        break;

      case MYCILA_JSY_MK_163:
      case MYCILA_JSY_MK_227:
      case MYCILA_JSY_MK_229:
        grid.updateMetrics({
          .source = Mycila::Grid::Source::JSY,
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
        grid.updateMetrics({
          .source = Mycila::Grid::Source::JSY,
          .apparentPower = data.channel2().apparentPower,
          .current = data.channel2().current,
          .energy = data.channel2().activeEnergyImported,
          .energyReturned = data.channel2().activeEnergyReturned,
          .frequency = data.aggregate.frequency,
          .power = data.channel2().activePower,
          .powerFactor = data.channel2().powerFactor,
          .voltage = data.channel2().voltage,
        });
        router.updateMetrics({
          .source = Mycila::Router::Source::JSY,
          .apparentPower = data.channel1().apparentPower,
          .current = data.channel1().current,
          .energy = data.channel1().activeEnergy + data.channel1().activeEnergyReturned, // if the clamp is installed reversed
          .power = std::abs(data.channel1().activePower),                                // if the clamp is installed reversed
          .powerFactor = data.channel1().powerFactor,
          .resistance = data.channel1().resistance(),
          .thdi = data.channel1().thdi(),
          .voltage = data.channel1().dimmedVoltage(),
        });
        break;

      case MYCILA_JSY_MK_333:
        grid.updateMetrics({
          .source = Mycila::Grid::Source::JSY,
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

    if (grid.isUsing(Mycila::Grid::Source::JSY)) {
      yasolr_divert();
    }
  }
}

void yasolr_configure_jsy() {
  if (config.getBool(KEY_ENABLE_JSY)) {
    // setup JSY if not done yet
    if (jsy == nullptr) {
      if (strcmp(config.get(KEY_JSY_UART), YASOLR_UART_NONE) == 0) {
        ESP_LOGE(TAG, "No UART selected for JSY");
        return;
      }

      ESP_LOGI(TAG, "Enable JSY with UART %s", config.get(KEY_JSY_UART));

      jsy = new Mycila::JSY();

      if (strcmp(config.get(KEY_JSY_UART), YASOLR_UART_1_NAME) == 0)
        jsy->begin(Serial1, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));

#if SOC_UART_NUM > 2
      if (strcmp(config.get(KEY_JSY_UART), YASOLR_UART_2_NAME) == 0)
        jsy->begin(Serial2, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));
#endif

      if (!jsy->isEnabled()) {
        ESP_LOGE(TAG, "JSY failed to initialize!");
        jsy->end();
        delete jsy;
        jsy = nullptr;
        return;
      }

      if (jsy->getBaudRate() != jsy->getMaxAvailableBaudRate())
        jsy->setBaudRate(jsy->getMaxAvailableBaudRate());

      jsyData = new Mycila::JSY::Data();
      jsy->setCallback(jsy_callback);

      // setup JSY task
      jsyTask = new Mycila::Task("JSY", []() {
        if (jsy != nullptr)
          jsy->read();
      });
      if (config.getBool(KEY_ENABLE_DEBUG)) {
        jsyTask->enableProfiling();
      }

      // setup JSY task manager
      jsyTaskManager = new Mycila::TaskManager("jsyTask");
      jsyTaskManager->addTask(*jsyTask);
      assert(jsyTaskManager->asyncStart(2048, 1, 0, 100, true));
      Mycila::TaskMonitor.addTask(jsyTaskManager->name());
    }
  } else {
    // disable JSY if enabled but leave the task manager in case we re-enable it later
    // stopping the whole task manager is supported but not deleting it to free memory, so we can lave it as-is
    if (jsy != nullptr) {
      ESP_LOGI(TAG, "Disable JSY");

      Mycila::TaskMonitor.removeTask(jsyTaskManager->name());
      jsyTaskManager->asyncStop();
      jsyTask->setEnabled(false);
      jsy->end();
      jsyTaskManager->waitForAllTasksToComplete();

      delete jsyTaskManager;
      jsyTaskManager = nullptr;

      delete jsyTask;
      jsyTask = nullptr;

      delete jsy;
      jsy = nullptr;

      delete jsyData;
      jsyData = nullptr;

      grid.deleteMetrics(Mycila::Grid::Source::JSY);
    }
  }
}
