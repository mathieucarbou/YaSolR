// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
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

    switch (data.model) {
        // clang-format off
      case MYCILA_JSY_MK_163: if (!grid.isUsing(Mycila::Grid::Source::JSY_MK_163_REMOTE)) return; else break;
      case MYCILA_JSY_MK_227: if (!grid.isUsing(Mycila::Grid::Source::JSY_MK_227_REMOTE)) return; else break;
      case MYCILA_JSY_MK_229: if (!grid.isUsing(Mycila::Grid::Source::JSY_MK_229_REMOTE)) return; else break;
      case MYCILA_JSY_MK_193: if (!grid.isUsing(Mycila::Grid::Source::JSY_MK_193_CH1_REMOTE) && !grid.isUsing(Mycila::Grid::Source::JSY_MK_193_CH2_REMOTE)) return; else break;
      case MYCILA_JSY_MK_194: if (!grid.isUsing(Mycila::Grid::Source::JSY_MK_194_CH1_REMOTE) && !grid.isUsing(Mycila::Grid::Source::JSY_MK_194_CH2_REMOTE)) return; else break;
      case MYCILA_JSY_MK_333: if (!grid.isUsing(Mycila::Grid::Source::JSY_MK_333_REMOTE)) return; else break;
      default: return; // unknown model => do not divert
        // clang-format on
    }

    Mycila::Grid::Metrics metrics;

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
        if (grid.isUsing(Mycila::Grid::Source::JSY_MK_193_CH1_SERIAL1) ||
            grid.isUsing(Mycila::Grid::Source::JSY_MK_193_CH1_SERIAL2) ||
            grid.isUsing(Mycila::Grid::Source::JSY_MK_194_CH1_SERIAL1) ||
            grid.isUsing(Mycila::Grid::Source::JSY_MK_194_CH1_SERIAL2)) {
          metrics.apparentPower = data.channel1().apparentPower;
          metrics.current = data.channel1().current;
          metrics.energy = data.channel1().activeEnergyImported;
          metrics.energyReturned = data.channel1().activeEnergyReturned;
          metrics.frequency = data.channel1().frequency;
          metrics.power = data.channel1().activePower;
          metrics.powerFactor = data.channel1().powerFactor;
          metrics.voltage = data.channel1().voltage;

          Mycila::Router::Metrics routerMetrics;
          routerMetrics.source = Mycila::Router::Source::JSY;
          routerMetrics.apparentPower = data.channel2().apparentPower;
          routerMetrics.current = data.channel2().current;
          routerMetrics.energy = data.channel2().activeEnergy + data.channel2().activeEnergyReturned; // if the clamp is installed reversed
          routerMetrics.power = std::abs(data.channel2().activePower);                                // if the clamp is installed reversed
          routerMetrics.powerFactor = data.channel2().powerFactor;
          routerMetrics.resistance = data.channel2().resistance();
          routerMetrics.thdi = data.channel2().thdi();
          routerMetrics.voltage = data.channel2().dimmedVoltage();
          router.updateMetrics(std::move(routerMetrics));

        } else if (grid.isUsing(Mycila::Grid::Source::JSY_MK_193_CH2_SERIAL1) ||
                   grid.isUsing(Mycila::Grid::Source::JSY_MK_193_CH2_SERIAL2) ||
                   grid.isUsing(Mycila::Grid::Source::JSY_MK_194_CH2_SERIAL1) ||
                   grid.isUsing(Mycila::Grid::Source::JSY_MK_194_CH2_SERIAL2)) {
          metrics.apparentPower = data.channel2().apparentPower;
          metrics.current = data.channel2().current;
          metrics.energy = data.channel2().activeEnergyImported;
          metrics.energyReturned = data.channel2().activeEnergyReturned;
          metrics.frequency = data.channel2().frequency;
          metrics.power = data.channel2().activePower;
          metrics.powerFactor = data.channel2().powerFactor;
          metrics.voltage = data.channel2().voltage;

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
        }
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
    pidTask.requestEarlyRun();
  }
}

void yasolr_configure_jsy() {
  if (grid.isUsing(Mycila::Grid::SourceKind::JSY_SERIAL1) || grid.isUsing(Mycila::Grid::SourceKind::JSY_SERIAL2)) {
    // setup JSY if not done yet
    if (jsy == nullptr) {
      const bool serial1AssignedToJSY = config.isEqual(KEY_PIN_SERIAL1_DEV, YASOLR_UART_DEVICE_JSY);
      const bool serial2AssignedToJSY = config.isEqual(KEY_PIN_SERIAL2_DEV, YASOLR_UART_DEVICE_JSY);

      if (!serial1AssignedToJSY && !serial2AssignedToJSY) {
        ESP_LOGE(TAG, "Unable to activate JSY: JSY was not assigned to any UART (Serial1 or Serial2)");
        return;
      }

      if (serial1AssignedToJSY && serial2AssignedToJSY) {
        ESP_LOGE(TAG, "Unable to activate JSY: YaSolR does not yet support running with 2 JSY");
        return;
      }

      if (serial1AssignedToJSY) {
        ESP_LOGI(TAG, "Enable JSY on UART Serial1");
        jsy = new Mycila::JSY();
        jsy->begin(Serial1, config.get<int8_t>(KEY_PIN_SERIAL1_RX), config.get<int8_t>(KEY_PIN_SERIAL1_TX));
      } else { // serial2AssignedToJSY == true
        ESP_LOGI(TAG, "Enable JSY on UART Serial2");
        jsy = new Mycila::JSY();
        jsy->begin(Serial2, config.get<int8_t>(KEY_PIN_SERIAL2_RX), config.get<int8_t>(KEY_PIN_SERIAL2_TX));
      }

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

      router.deleteMetrics(Mycila::Router::Source::JSY);
    }
  }
}
