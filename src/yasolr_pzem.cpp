// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#include <utility>

Mycila::PZEM* pzem[2] = {nullptr, nullptr};             // pzem[0] is for output1, pzem[1] is for output2
Mycila::Task* pzemPairingTasks[2] = {nullptr, nullptr}; // pzemPairingTasks[0] is for output1, pzemPairingTasks[1] is for output2. These tasks are used to pair the PZEM to the correct address after power on, and then deleted.

static Mycila::Task* pzemTask = nullptr;

static void init_read_task() {
  if (pzemTask == nullptr) {
    pzemTask = new Mycila::Task("PZEM", []() {
      if (pzem[0]) {
        pzem[0]->read();
        yield();
      }
      if (pzem[1]) {
        pzem[1]->read();
        yield();
      }
    });

    pzemTask->setEnabledWhen([]() { return (!pzemPairingTasks[0] || pzemPairingTasks[0]->paused()) && (!pzemPairingTasks[1] || pzemPairingTasks[1]->paused()); });
    pzemTask->setInterval(200);

    unsafeTaskManager.addTask(*pzemTask);

    if (config.get<bool>(KEY_ENABLE_DEBUG)) {
      pzemTask->enableProfiling();
    }
  }
}

static Mycila::PZEM* init_pzem(Mycila::Router::Output& output, const uint8_t address, Mycila::PZEM* existing = nullptr) {
  Mycila::PZEM* pzem = nullptr;

  if (output.isUsing(Mycila::metric::Source::PZEM_SERIAL1)) {
    ESP_LOGI(TAG, "Enable %s PZEM on UART Serial1", output.getName());
    pzem = existing == nullptr ? new Mycila::PZEM() : existing;
    pzem->setSharedSerial(true);
    pzem->begin(Serial1, config.get<int8_t>(KEY_PIN_SERIAL1_RX), config.get<int8_t>(KEY_PIN_SERIAL1_TX), address);

  } else if (output.isUsing(Mycila::metric::Source::PZEM_SERIAL2)) {
    ESP_LOGI(TAG, "Enable %s PZEM on UART Serial2", output.getName());
    pzem = existing == nullptr ? new Mycila::PZEM() : existing;
    pzem->setSharedSerial(true);
    pzem->begin(Serial2, config.get<int8_t>(KEY_PIN_SERIAL2_RX), config.get<int8_t>(KEY_PIN_SERIAL2_TX), address);
  }

  if (!pzem->isEnabled()) {
    ESP_LOGE(TAG, "%s PZEM failed to initialize!", output.getName());
    pzem->end();

    if (existing == nullptr) {
      delete pzem;
      pzem = nullptr;
    }
  }

  return pzem;
}

static bool pair(Mycila::Router::Output& output, const uint8_t toAddress, Mycila::PZEM* pzem) {
  ESP_LOGI(TAG, "Pairing connected PZEM to %s", output.getName());

  pzem->end();
  init_pzem(output, MYCILA_PZEM_ADDRESS_GENERAL, pzem);
  const uint8_t deviceAddress = pzem->getDeviceAddress();

  // already paired
  if (deviceAddress == toAddress) {
    ESP_LOGW(TAG, "PZEM already paired to %s", output.getName());
    return true;
  }

  // no device found or cannot set address
  if (deviceAddress == MYCILA_PZEM_ADDRESS_UNKNOWN || !pzem->setDeviceAddress(toAddress)) {
    ESP_LOGE(TAG, "Failed to pair PZEM to %s: make sure only PZEM of %s is powered and connected to Serial RX/TX!", output.getName(), output.getName());
    pzem->end();
    return false;
  }

  ESP_LOGI(TAG, "PZEM has been paired to %s", output.getName());
  return true;
}

static void configure_pzem(uint8_t index, Mycila::Router::Output& output, const uint8_t address) {
  if (output.isUsing(Mycila::metric::Kind::PZEM)) {
    init_read_task();

    if (pzem[index] == nullptr) {
      pzem[index] = init_pzem(output, address, nullptr);

      if (pzem[index]) {
        pzem[index]->setCallback([&output](const Mycila::PZEM::EventType eventType, const Mycila::PZEM::Data& data) {
          if (eventType == Mycila::PZEM::EventType::EVT_READ) {
            Mycila::metric::Metrics metrics;
            metrics.apparentPower = data.apparentPower;
            metrics.current = data.current;
            metrics.energy = data.activeEnergy;
            metrics.frequency = data.frequency;
            metrics.power = data.activePower;
            metrics.powerFactor = data.powerFactor;
            metrics.resistance = data.resistance();
            metrics.thdi = data.thdi();
            metrics.voltage = data.dimmedVoltage();
            output.updateMetrics(std::move(metrics));
          }
        });

        pzemPairingTasks[index] = new Mycila::Task("PZEM Pairing", Mycila::Task::Type::ONCE, [&output, index, address]() {
          ESP_LOGI(TAG, "Pairing connected PZEM to %s", output.getName());
          pair(output, address, pzem[index]);
        });

        unsafeTaskManager.addTask(*pzemPairingTasks[index]);

        if (config.get<bool>(KEY_ENABLE_DEBUG))
          pzemPairingTasks[index]->enableProfiling();
      };
    }
  } else {
    if (pzem[index] != nullptr) {
      ESP_LOGI(TAG, "Disable %s PZEM", output.getName());

      unsafeTaskManager.removeTask(*pzemPairingTasks[index]);
      delete pzemPairingTasks[index];
      pzemPairingTasks[index] = nullptr;

      pzem[index]->end();
      delete pzem[index];
      pzem[index] = nullptr;
    }
  }
}

void yasolr_configure_pzem() {
  configure_pzem(0, output1, YASOLR_PZEM_ADDRESS_OUTPUT1);
  configure_pzem(1, output2, YASOLR_PZEM_ADDRESS_OUTPUT2);
}
