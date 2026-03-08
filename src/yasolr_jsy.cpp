// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#include <utility>

Mycila::JSY* jsy[2] = {nullptr, nullptr}; // array of 2 pointers: jsy[0] for Serial1, jsy[1] for Serial2
Mycila::TaskManager* jsyTaskManager = nullptr;

static Mycila::Task* jsyTask;
static Mycila::JSY::Data* jsyData[2] = {nullptr, nullptr}; // array of 2 pointers: jsyData[0] for Serial1, jsyData[1] for Serial2

static void init_read_task() {
  if (jsyTask == nullptr) {
    jsyTask = new Mycila::Task("JSY", []() {
      // delay to yield back to core0
      if (jsy[0] == nullptr && jsy[1] == nullptr) {
        delay(100);
        return;
      }

      if (jsy[0]) {
        if (jsy[0]->read()) yield();
        else delay(40);
      }

      if (jsy[1]) {
        if (jsy[1]->read()) yield();
        else delay(40);
      }
    });

    if (config.get<bool>(KEY_ENABLE_DEBUG)) {
      jsyTask->enableProfiling();
    }

    // setup JSY task manager
    jsyTaskManager = new Mycila::TaskManager("jsyTask");
    jsyTaskManager->addTask(*jsyTask);
    assert(jsyTaskManager->asyncStart(3072, 1, 0, 0, true));
    Mycila::TaskMonitor.addTask(jsyTaskManager->name());
  }
}

static void jsy_callback(const uint8_t index, Mycila::metric::Kind serialKind, const Mycila::JSY::EventType eventType, const Mycila::JSY::Data& data) {
  if (*jsyData[index] != data) {
    *jsyData[index] = data;

    switch (data.model) {
      case MYCILA_JSY_MK_163:
      case MYCILA_JSY_MK_227:
      case MYCILA_JSY_MK_229: {
        if (grid.isUsing(serialKind) && (grid.isUsing(Mycila::metric::Kind::JSY_MK_163) || grid.isUsing(Mycila::metric::Kind::JSY_MK_227) || grid.isUsing(Mycila::metric::Kind::JSY_MK_229))) {
          Mycila::metric::Metrics metrics;
          metrics.apparentPower = data.single().apparentPower;
          metrics.current = data.single().current;
          metrics.energy = data.single().activeEnergyImported;
          metrics.energyReturned = data.single().activeEnergyReturned;
          metrics.frequency = data.single().frequency;
          metrics.power = data.single().activePower;
          metrics.powerFactor = data.single().powerFactor;
          metrics.voltage = data.single().voltage;
          grid.updateMetrics(std::move(metrics));
          pidTask.requestEarlyRun();
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && (output->isUsing(Mycila::metric::Kind::JSY_MK_163) || output->isUsing(Mycila::metric::Kind::JSY_MK_227) || output->isUsing(Mycila::metric::Kind::JSY_MK_229))) {
              Mycila::metric::Metrics metrics;
              metrics.apparentPower = data.single().apparentPower;
              metrics.current = data.single().current;
              metrics.energy = (data.single().activeEnergyImported + data.single().activeEnergyReturned); // if the clamp is installed reversed
              metrics.frequency = data.single().frequency;
              metrics.power = std::abs(data.single().activePower); // if the clamp is installed reversed
              metrics.powerFactor = data.single().powerFactor;
              metrics.resistance = data.single().resistance();
              metrics.thdi = data.single().thdi();
              metrics.voltage = data.single().voltage;
              metrics.zeroNaN();
              output->updateMetrics(std::move(metrics));
              break;
            }
          }
        }
        break;
      }
      case MYCILA_JSY_MK_193:
      case MYCILA_JSY_MK_194: {
        // Channel 1
        if (grid.isUsing(serialKind) && (grid.isUsing(Mycila::metric::Kind::JSY_MK_193_CH1) || grid.isUsing(Mycila::metric::Kind::JSY_MK_194_CH1))) {
          Mycila::metric::Metrics metrics;
          metrics.apparentPower = data.channel1().apparentPower;
          metrics.current = data.channel1().current;
          metrics.energy = data.channel1().activeEnergyImported;
          metrics.energyReturned = data.channel1().activeEnergyReturned;
          metrics.frequency = data.channel1().frequency;
          metrics.power = data.channel1().activePower;
          metrics.powerFactor = data.channel1().powerFactor;
          metrics.voltage = data.channel1().voltage;
          grid.updateMetrics(std::move(metrics));
          pidTask.requestEarlyRun();
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && (output->isUsing(Mycila::metric::Kind::JSY_MK_193_CH1) || output->isUsing(Mycila::metric::Kind::JSY_MK_194_CH1))) {
              Mycila::metric::Metrics metrics;
              metrics.apparentPower = data.channel1().apparentPower;
              metrics.current = data.channel1().current;
              metrics.energy = (data.channel1().activeEnergyImported + data.channel1().activeEnergyReturned); // if the clamp is installed reversed
              metrics.frequency = data.channel1().frequency;
              metrics.power = std::abs(data.channel1().activePower); // if the clamp is installed reversed
              metrics.powerFactor = data.channel1().powerFactor;
              metrics.resistance = data.channel1().resistance();
              metrics.thdi = data.channel1().thdi();
              metrics.voltage = data.channel1().voltage;
              metrics.zeroNaN();
              output->updateMetrics(std::move(metrics));
              break;
            }
          }
        }
        // Channel 2
        if (grid.isUsing(serialKind) && (grid.isUsing(Mycila::metric::Kind::JSY_MK_193_CH2) || grid.isUsing(Mycila::metric::Kind::JSY_MK_194_CH2))) {
          Mycila::metric::Metrics metrics;
          metrics.apparentPower = data.channel2().apparentPower;
          metrics.current = data.channel2().current;
          metrics.energy = data.channel2().activeEnergyImported;
          metrics.energyReturned = data.channel2().activeEnergyReturned;
          metrics.frequency = data.channel2().frequency;
          metrics.power = data.channel2().activePower;
          metrics.powerFactor = data.channel2().powerFactor;
          metrics.voltage = data.channel2().voltage;
          metrics.zeroNaN();
          grid.updateMetrics(std::move(metrics));
          pidTask.requestEarlyRun();
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && (output->isUsing(Mycila::metric::Kind::JSY_MK_193_CH2) || output->isUsing(Mycila::metric::Kind::JSY_MK_194_CH2))) {
              Mycila::metric::Metrics metrics;
              metrics.apparentPower = data.channel2().apparentPower;
              metrics.current = data.channel2().current;
              metrics.energy = (data.channel2().activeEnergyImported + data.channel2().activeEnergyReturned); // if the clamp is installed reversed
              metrics.frequency = data.channel2().frequency;
              metrics.power = std::abs(data.channel2().activePower); // if the clamp is installed reversed
              metrics.powerFactor = data.channel2().powerFactor;
              metrics.resistance = data.channel2().resistance();
              metrics.thdi = data.channel2().thdi();
              metrics.voltage = data.channel2().voltage;
              metrics.zeroNaN();
              output->updateMetrics(std::move(metrics));
              break;
            }
          }
        }
        break;
      }
      case MYCILA_JSY_MK_333: {
        if (grid.isUsing(serialKind) && grid.isUsing(Mycila::metric::Kind::JSY_MK_333)) {
          Mycila::metric::Metrics metrics;
          metrics.apparentPower = data.aggregate.apparentPower;
          metrics.current = data.aggregate.current;
          metrics.energy = data.aggregate.activeEnergyImported;
          metrics.energyReturned = data.aggregate.activeEnergyReturned;
          metrics.frequency = data.aggregate.frequency;
          metrics.power = data.aggregate.activePower;
          metrics.powerFactor = data.aggregate.powerFactor;
          metrics.voltage = data.aggregate.voltage;
          grid.updateMetrics(std::move(metrics));
          pidTask.requestEarlyRun();
          break;
        }
      }
      default:
        break; // unknown model => do not divert
    }
  }
}
static void yasolr_configure_jsy(const uint8_t index, Mycila::metric::Kind serialKind, HardwareSerial& serial, int8_t rxPin, int8_t txPin) {
  if (grid.isUsing(serialKind) || output1.isUsing(serialKind) || output2.isUsing(serialKind)) {
    // setup JSY if not done yet
    if (jsy[index] == nullptr) {
      ESP_LOGI(TAG, "Enable JSY on UART Serial1");
      jsy[index] = new Mycila::JSY();
      jsy[index]->begin(serial, rxPin, txPin);

      if (!jsy[index]->isEnabled()) {
        ESP_LOGE(TAG, "JSY failed to initialize!");
        jsy[index]->end();
        delete jsy[0];
        jsy[index] = nullptr;
        return;
      }

      if (jsy[index]->getBaudRate() != jsy[index]->getMaxAvailableBaudRate())
        jsy[index]->setBaudRate(jsy[index]->getMaxAvailableBaudRate());

      jsyData[index] = new Mycila::JSY::Data();
      jsy[index]->setCallback([index, serialKind](const Mycila::JSY::EventType eventType, const Mycila::JSY::Data& data) {
        jsy_callback(index, serialKind, eventType, data);
      });

      init_read_task();
    }
  } else {
    // disable JSY if enabled but leave the task manager in case we re-enable it later
    // stopping the whole task manager is supported but not deleting it to free memory, so we can lave it as-is
    if (jsy[index] != nullptr) {
      ESP_LOGI(TAG, "Disable JSY on UART Serial1");

      jsyTask->setEnabled(false);
      jsy[index]->end();

      delete jsy[index];
      jsy[index] = nullptr;

      delete jsyData[index];
      jsyData[index] = nullptr;

      if (jsy[0] == nullptr && jsy[1] == nullptr) {
        Mycila::TaskMonitor.removeTask(jsyTaskManager->name());
        jsyTaskManager->asyncStop();
        jsyTaskManager->waitForAllTasksToComplete();
        delete jsyTaskManager;
        jsyTaskManager = nullptr;
      }
    }
  }
}

void yasolr_configure_jsy() {
  yasolr_configure_jsy(0, Mycila::metric::Kind::JSY_SERIAL1, Serial1, config.get<int8_t>(KEY_PIN_SERIAL1_RX), config.get<int8_t>(KEY_PIN_SERIAL1_TX));
  yasolr_configure_jsy(1, Mycila::metric::Kind::JSY_SERIAL2, Serial2, config.get<int8_t>(KEY_PIN_SERIAL2_RX), config.get<int8_t>(KEY_PIN_SERIAL2_TX));
}
