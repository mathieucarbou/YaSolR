// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <utility>

Mycila::JSY* jsy = nullptr;
Mycila::TaskManager* jsyTaskManager = nullptr;

static Mycila::Task* jsyTask;
static Mycila::JSY::Data* jsyData;

static void jsy_callback(const Mycila::JSY::EventType eventType, const Mycila::JSY::Data& data) {
  if (*jsyData != data) {
    *jsyData = data;

    Mycila::Grid::Metrics metrics;
    metrics.source = Mycila::Grid::Source::JSY;

    switch (data.model) {
      case MYCILA_JSY_MK_163:
      case MYCILA_JSY_MK_227:
      case MYCILA_JSY_MK_229: {
        metrics.apparentPower = data.single().apparentPower;
        metrics.current = data.single().current;
        metrics.energy = data.single().activeEnergyImported;
        metrics.energyReturned = data.single().activeEnergyReturned;
        metrics.frequency = data.single().frequency;
        metrics.power = data.single().activePower;
        metrics.powerFactor = data.single().powerFactor;
        metrics.voltage = data.single().voltage;
        break;
      }
      case MYCILA_JSY_MK_193:
      case MYCILA_JSY_MK_194: {
        Mycila::Router::Metrics routerMetrics;
        routerMetrics.source = Mycila::Router::Source::JSY;
        routerMetrics.apparentPower = data.channel1().apparentPower;
        routerMetrics.current = data.channel1().current;
        routerMetrics.energy = data.channel1().activeEnergy + data.channel1().activeEnergyReturned; // if the clamp is installed reversed
        routerMetrics.power = std::abs(data.channel1().activePower);                                // if the clamp is installed reversed
        routerMetrics.powerFactor = data.channel1().powerFactor;
        routerMetrics.resistance = data.channel1().resistance();
        routerMetrics.thdi = data.channel1().thdi();
        routerMetrics.voltage = data.channel1().dimmedVoltage();
        router.updateMetrics(std::move(routerMetrics));

        metrics.apparentPower = data.channel2().apparentPower;
        metrics.current = data.channel2().current;
        metrics.energy = data.channel2().activeEnergyImported;
        metrics.energyReturned = data.channel2().activeEnergyReturned;
        metrics.frequency = data.aggregate.frequency;
        metrics.power = data.channel2().activePower;
        metrics.powerFactor = data.channel2().powerFactor;
        metrics.voltage = data.channel2().voltage;
        break;
      }
      case MYCILA_JSY_MK_333: {
        metrics.apparentPower = data.aggregate.apparentPower;
        metrics.current = data.aggregate.current;
        metrics.energy = data.aggregate.activeEnergyImported;
        metrics.energyReturned = data.aggregate.activeEnergyReturned;
        metrics.frequency = data.aggregate.frequency;
        metrics.power = data.aggregate.activePower;
        metrics.powerFactor = data.aggregate.powerFactor;
        metrics.voltage = data.aggregate.voltage;
        break;
      }
      default:
        return; // unknown model => do not divert
    }

    grid.updateMetrics(std::move(metrics));

    if (grid.isUsing(Mycila::Grid::Source::JSY)) {
      yasolr_divert();
    }
  }
}

void yasolr_configure_jsy() {
  if (config.get<bool>(KEY_ENABLE_JSY)) {
    // setup JSY if not done yet
    if (jsy == nullptr) {
      if (strcmp(config.getString(KEY_JSY_UART), YASOLR_UART_NONE) == 0) {
        ESP_LOGE(TAG, "No UART selected for JSY");
        return;
      }

      ESP_LOGI(TAG, "Enable JSY with UART %s", config.getString(KEY_JSY_UART));

      jsy = new Mycila::JSY();

      if (strcmp(config.getString(KEY_JSY_UART), YASOLR_UART_1_NAME) == 0)
        jsy->begin(Serial1, config.get<int8_t>(KEY_PIN_JSY_RX), config.get<int8_t>(KEY_PIN_JSY_TX));

#if SOC_UART_NUM > 2
      if (strcmp(config.getString(KEY_JSY_UART), YASOLR_UART_2_NAME) == 0)
        jsy->begin(Serial2, config.get<int8_t>(KEY_PIN_JSY_RX), config.get<int8_t>(KEY_PIN_JSY_TX));
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
      if (config.get<bool>(KEY_ENABLE_DEBUG)) {
        jsyTask->enableProfiling();
      }

      // setup JSY task manager
      jsyTaskManager = new Mycila::TaskManager("jsyTask");
      jsyTaskManager->addTask(*jsyTask);
      assert(jsyTaskManager->asyncStart(3072, 1, 0, 100, true));
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
      router.deleteMetrics(Mycila::Router::Source::JSY);
    }
  }
}
