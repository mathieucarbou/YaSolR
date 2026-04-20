// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#include <memory>
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
    assert(jsyTaskManager->asyncStart(YASOLR_TASK_JSY_STACK_SIZE, 1, 0, 0, true));
    Mycila::TaskMonitor.addTask(jsyTaskManager->name());
  }
}

static void updateGrid(const Mycila::JSY::Metrics& channel) {
  std::unique_ptr<Mycila::metric::Metrics> metrics = std::make_unique<Mycila::metric::Metrics>();
  metrics->apparentPower = channel.apparentPower;
  metrics->current = channel.current;
  metrics->energy = channel.activeEnergyImported;
  metrics->energyReturned = channel.activeEnergyReturned;
  metrics->frequency = channel.frequency;
  metrics->power = channel.activePower;
  metrics->powerFactor = channel.powerFactor;
  metrics->voltage = channel.voltage;
  grid.updateMetrics(std::move(metrics));
  pidTask.requestEarlyRun();
}

static void updateOutput(Mycila::Router::Output* output, const Mycila::JSY::Metrics& channel) {
  std::unique_ptr<Mycila::metric::Metrics> metrics = std::make_unique<Mycila::metric::Metrics>();
  metrics->apparentPower = channel.apparentPower;
  metrics->current = channel.current;
  metrics->energy = (channel.activeEnergyImported + channel.activeEnergyReturned); // if the clamp is installed reversed
  metrics->frequency = channel.frequency;
  metrics->power = std::abs(channel.activePower); // if the clamp is installed reversed
  metrics->powerFactor = channel.powerFactor;
  metrics->resistance = channel.resistance();
  metrics->thdi = channel.thdi();
  metrics->voltage = channel.voltage;
  metrics->zeroNaN();
  output->updateMetrics(std::move(metrics));
}

static void jsy_callback(const uint8_t index, Mycila::metric::Kind serialKind, const Mycila::JSY::EventType eventType, const Mycila::JSY::Data& data) {
  if (*jsyData[index] != data) {
    *jsyData[index] = data;

    switch (data.model) {
      case MYCILA_JSY_MK_163:
      case MYCILA_JSY_MK_227:
      case MYCILA_JSY_MK_229: {
        if (grid.isUsing(serialKind) && (grid.isUsing(Mycila::metric::Kind::JSY_MK_163) ||
                                         grid.isUsing(Mycila::metric::Kind::JSY_MK_227) ||
                                         grid.isUsing(Mycila::metric::Kind::JSY_MK_229))) {
          updateGrid(data.single());
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && (output->isUsing(Mycila::metric::Kind::JSY_MK_163) ||
                                                output->isUsing(Mycila::metric::Kind::JSY_MK_227) ||
                                                output->isUsing(Mycila::metric::Kind::JSY_MK_229))) {
              updateOutput(output, data.single());
            }
          }
        }
        break;
      }
      case MYCILA_JSY_MK_193:
      case MYCILA_JSY_MK_194: {
        // Channel 1
        if (grid.isUsing(serialKind) && (grid.isUsing(Mycila::metric::Kind::JSY_MK_193_CH1) ||
                                         grid.isUsing(Mycila::metric::Kind::JSY_MK_194_CH1))) {
          updateGrid(data.channel1());
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && (output->isUsing(Mycila::metric::Kind::JSY_MK_193_CH1) ||
                                                output->isUsing(Mycila::metric::Kind::JSY_MK_194_CH1))) {
              updateOutput(output, data.channel1());
            }
          }
        }
        // Channel 2
        if (grid.isUsing(serialKind) && (grid.isUsing(Mycila::metric::Kind::JSY_MK_193_CH2) ||
                                         grid.isUsing(Mycila::metric::Kind::JSY_MK_194_CH2))) {
          updateGrid(data.channel2());
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && (output->isUsing(Mycila::metric::Kind::JSY_MK_193_CH2) ||
                                                output->isUsing(Mycila::metric::Kind::JSY_MK_194_CH2))) {
              updateOutput(output, data.channel2());
            }
          }
        }
        break;
      }
      case MYCILA_JSY_MK_333: {
        // Aggregate (only supported by grid since it does not make sense for outputs)
        if (grid.isUsing(serialKind) && grid.isUsing(Mycila::metric::Kind::JSY_MK_333_AGGREGATE)) {
          updateGrid(data.aggregate);
        }
        // Phase A
        if (grid.isUsing(serialKind) && grid.isUsing(Mycila::metric::Kind::JSY_MK_333_PHASE_A)) {
          updateGrid(data.phaseA());
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && output->isUsing(Mycila::metric::Kind::JSY_MK_333_PHASE_A)) {
              updateOutput(output, data.phaseA());
            }
          }
        }
        // Phase B
        if (grid.isUsing(serialKind) && grid.isUsing(Mycila::metric::Kind::JSY_MK_333_PHASE_B)) {
          updateGrid(data.phaseB());
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && output->isUsing(Mycila::metric::Kind::JSY_MK_333_PHASE_B)) {
              updateOutput(output, data.phaseB());
            }
          }
        }
        // Phase C
        if (grid.isUsing(serialKind) && grid.isUsing(Mycila::metric::Kind::JSY_MK_333_PHASE_C)) {
          updateGrid(data.phaseC());
        } else {
          for (Mycila::Router::Output* output : {&output1, &output2}) {
            if (output->isUsing(serialKind) && output->isUsing(Mycila::metric::Kind::JSY_MK_333_PHASE_C)) {
              updateOutput(output, data.phaseC());
            }
          }
        }
        break;
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

      jsy[index]->setCallback(nullptr); // detach callback to avoid any potential call after end() and before delete

      Mycila::JSY* ptr = jsy[index];
      Mycila::JSY::Data* dataPtr = jsyData[index];

      jsy[index] = nullptr;
      jsyData[index] = nullptr;

      ptr->end();

      delete ptr;
      delete dataPtr;
    }
  }
}

void yasolr_configure_jsy() {
  yasolr_configure_jsy(0, Mycila::metric::Kind::JSY_SERIAL1, Serial1, config.get<int8_t>(KEY_PIN_SERIAL1_RX), config.get<int8_t>(KEY_PIN_SERIAL1_TX));
  yasolr_configure_jsy(1, Mycila::metric::Kind::JSY_SERIAL2, Serial2, config.get<int8_t>(KEY_PIN_SERIAL2_RX), config.get<int8_t>(KEY_PIN_SERIAL2_TX));
}
