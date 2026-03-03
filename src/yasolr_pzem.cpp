// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#include <yasolr.h>

#include <utility>

Mycila::PZEM* pzemO1 = nullptr;
Mycila::PZEM* pzemO2 = nullptr;
Mycila::Task* pzemO1PairingTask = nullptr;
Mycila::Task* pzemO2PairingTask = nullptr;

static Mycila::Task* pzemTask = nullptr;

static void init_read_task() {
  if (pzemTask == nullptr) {
    pzemTask = new Mycila::Task("PZEM", []() {
      if (pzemO1) {
        pzemO1->read();
        yield();
      }
      if (pzemO2) {
        pzemO2->read();
        yield();
      }
    });

    pzemTask->setEnabledWhen([]() { return (!pzemO1PairingTask || pzemO1PairingTask->paused()) && (!pzemO2PairingTask || pzemO2PairingTask->paused()); });
    pzemTask->setInterval(200);

    unsafeTaskManager.addTask(*pzemTask);

    if (config.get<bool>(KEY_ENABLE_DEBUG)) {
      pzemTask->enableProfiling();
    }
  }
}

static Mycila::PZEM* init_pzem(const char* label, const uint8_t address, Mycila::PZEM* existing = nullptr) {
  const bool serial1AssignedToPZEM = config.isEqual(KEY_PIN_SERIAL1_DEV, YASOLR_UART_DEVICE_PZEM);
  const bool serial2AssignedToPZEM = config.isEqual(KEY_PIN_SERIAL2_DEV, YASOLR_UART_DEVICE_PZEM);

  if (!serial1AssignedToPZEM && !serial2AssignedToPZEM) {
    ESP_LOGE(TAG, "Unable to activate %s PZEM: PZEM was not assigned to any UART (Serial1 or Serial2)", label);
    return nullptr;
  }

  // true if Serial1 or Serial2 is allocated to PZEM but not both at the same time (mutually exclusive)
  const bool onlyOneUartForPZEM = serial1AssignedToPZEM ^ serial2AssignedToPZEM;

  // true if all PZEM are enabled on only 1 UART (either Serial1 or Serial2), in which case we need to assign addresses
  const bool onlyOneUartForAllPZEM = onlyOneUartForPZEM && config.get<bool>(KEY_ENABLE_OUTPUT1_PZEM) && config.get<bool>(KEY_ENABLE_OUTPUT2_PZEM);

  Mycila::PZEM* pzem = nullptr;

  if (serial1AssignedToPZEM) {
    ESP_LOGI(TAG, "Enable %s PZEM on UART Serial1", label);
    pzem = existing == nullptr ? new Mycila::PZEM() : existing;
    pzem->setSharedSerial(onlyOneUartForPZEM);
    pzem->begin(Serial1, config.get<int8_t>(KEY_PIN_SERIAL1_RX), config.get<int8_t>(KEY_PIN_SERIAL1_TX), onlyOneUartForAllPZEM ? address : MYCILA_PZEM_ADDRESS_GENERAL);
  } else { // serial2AssignedToPZEM == true
    ESP_LOGI(TAG, "Enable %s PZEM on UART Serial2", label);
    pzem = existing == nullptr ? new Mycila::PZEM() : existing;
    pzem->setSharedSerial(onlyOneUartForPZEM);
    pzem->begin(Serial2, config.get<int8_t>(KEY_PIN_SERIAL2_RX), config.get<int8_t>(KEY_PIN_SERIAL2_TX), onlyOneUartForAllPZEM ? address : MYCILA_PZEM_ADDRESS_GENERAL);
  }

  if (!pzem->isEnabled()) {
    ESP_LOGE(TAG, "%s PZEM failed to initialize!", label);
    pzem->end();

    if (existing == nullptr) {
      delete pzem;
      pzem = nullptr;
    }
  }

  return pzem;
}

static bool pair(const char* label, const uint8_t toAddress, Mycila::PZEM* pzem) {
  ESP_LOGI(TAG, "Pairing connected PZEM to %s", label);

  pzem->end();
  init_pzem(label, MYCILA_PZEM_ADDRESS_GENERAL, pzem);
  const uint8_t deviceAddress = pzem->getDeviceAddress();

  // already paired
  if (deviceAddress == toAddress) {
    ESP_LOGW(TAG, "PZEM already paired to %s", label);
    return true;
  }

  // no device found or cannot set address
  if (deviceAddress == MYCILA_PZEM_ADDRESS_UNKNOWN || !pzem->setDeviceAddress(toAddress)) {
    ESP_LOGE(TAG, "Failed to pair PZEM to %s: make sure only PZEM of %s is powered and connected to Serial RX/TX!", label, label);
    pzem->end();
    return false;
  }

  ESP_LOGI(TAG, "PZEM has been paired to %s", label);
  return true;
}

void yasolr_configure_output1_pzem() {
  if (config.get<bool>(KEY_ENABLE_OUTPUT1_PZEM)) {
    init_read_task();

    if (pzemO1 == nullptr) {
      pzemO1 = init_pzem("Output 1", YASOLR_PZEM_ADDRESS_OUTPUT1, nullptr);

      if (pzemO1) {
        pzemO1->setCallback([](const Mycila::PZEM::EventType eventType, const Mycila::PZEM::Data& data) {
          if (eventType == Mycila::PZEM::EventType::EVT_READ) {
            Mycila::Grid::Metrics metrics;
            metrics.source = Mycila::Grid::Source::PZEM;
            metrics.frequency = data.frequency;
            metrics.voltage = data.voltage;
            grid.updateMetrics(std::move(metrics));

            Mycila::Router::Metrics routerMetrics;
            routerMetrics.source = Mycila::Router::Source::PZEM;
            routerMetrics.apparentPower = data.apparentPower;
            routerMetrics.current = data.current;
            routerMetrics.energy = data.activeEnergy;
            routerMetrics.power = data.activePower;
            routerMetrics.powerFactor = data.powerFactor;
            routerMetrics.resistance = data.resistance();
            routerMetrics.thdi = data.thdi();
            routerMetrics.voltage = data.dimmedVoltage();
            output1.updateMetrics(std::move(routerMetrics));
          }
        });

        pzemO1PairingTask = new Mycila::Task("PZEM Pairing 0x01", Mycila::Task::Type::ONCE, []() {
          ESP_LOGI(TAG, "Pairing connected PZEM to Output 1");
          pair("Output 1", YASOLR_PZEM_ADDRESS_OUTPUT1, pzemO1);
        });

        unsafeTaskManager.addTask(*pzemO1PairingTask);

        if (config.get<bool>(KEY_ENABLE_DEBUG))
          pzemO1PairingTask->enableProfiling();
      };
    }
  } else {
    if (pzemO1 != nullptr) {
      ESP_LOGI(TAG, "Disable Output 1 PZEM");

      unsafeTaskManager.removeTask(*pzemO1PairingTask);
      delete pzemO1PairingTask;
      pzemO1PairingTask = nullptr;

      pzemO1->end();
      delete pzemO1;
      pzemO1 = nullptr;
    }
  }
}

void yasolr_configure_output2_pzem() {
  if (config.get<bool>(KEY_ENABLE_OUTPUT2_PZEM)) {
    init_read_task();

    if (pzemO2 == nullptr) {
      pzemO2 = init_pzem("Output 2", YASOLR_PZEM_ADDRESS_OUTPUT2, nullptr);

      if (pzemO2) {
        pzemO2->setCallback([](const Mycila::PZEM::EventType eventType, const Mycila::PZEM::Data& data) {
          if (eventType == Mycila::PZEM::EventType::EVT_READ) {
            Mycila::Grid::Metrics metrics;
            metrics.source = Mycila::Grid::Source::PZEM;
            metrics.frequency = data.frequency;
            metrics.voltage = data.voltage;
            grid.updateMetrics(std::move(metrics));

            Mycila::Router::Metrics routerMetrics;
            routerMetrics.source = Mycila::Router::Source::PZEM;
            routerMetrics.apparentPower = data.apparentPower;
            routerMetrics.current = data.current;
            routerMetrics.energy = data.activeEnergy;
            routerMetrics.power = data.activePower;
            routerMetrics.powerFactor = data.powerFactor;
            routerMetrics.resistance = data.resistance();
            routerMetrics.thdi = data.thdi();
            routerMetrics.voltage = data.dimmedVoltage();
            output2.updateMetrics(std::move(routerMetrics));
          }
        });

        pzemO2PairingTask = new Mycila::Task("PZEM Pairing 0x02", Mycila::Task::Type::ONCE, []() {
          ESP_LOGI(TAG, "Pairing connected PZEM to Output 2");
          pair("Output 2", YASOLR_PZEM_ADDRESS_OUTPUT2, pzemO2);
        });

        unsafeTaskManager.addTask(*pzemO2PairingTask);

        if (config.get<bool>(KEY_ENABLE_DEBUG))
          pzemO2PairingTask->enableProfiling();
      }
    }
  } else {
    if (pzemO2 != nullptr) {
      ESP_LOGI(TAG, "Disable Output 2 PZEM");

      unsafeTaskManager.removeTask(*pzemO2PairingTask);
      delete pzemO2PairingTask;
      pzemO2PairingTask = nullptr;

      pzemO2->end();
      delete pzemO2;
      pzemO2 = nullptr;
    }
  }
}
