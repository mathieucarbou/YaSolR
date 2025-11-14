// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

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

    if (config.getBool(KEY_ENABLE_DEBUG)) {
      pzemTask->enableProfiling();
    }
  }
}

void yasolr_configure_output1_pzem() {
  if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
    init_read_task();

    if (pzemO1 == nullptr) {
      ESP_LOGI(TAG, "Enable Output 1 PZEM with UART %s", config.get(KEY_PZEM_UART));

      pzemO1 = new Mycila::PZEM();
      pzemO1->setSharedSerial(true);

      if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_1_NAME) == 0)
        pzemO1->begin(Serial1, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT1);

#if SOC_UART_NUM > 2
      if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_2_NAME) == 0)
        pzemO1->begin(Serial2, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT1);
#endif

      if (pzemO1->isEnabled()) {
        pzemO1->setCallback([](const Mycila::PZEM::EventType eventType, const Mycila::PZEM::Data& data) {
          if (eventType == Mycila::PZEM::EventType::EVT_READ) {
            grid.updateMetrics({
              .source = Mycila::Grid::Source::PZEM,
              .apparentPower = NAN,
              .current = NAN,
              .energy = 0,
              .energyReturned = 0,
              .frequency = data.frequency,
              .power = NAN,
              .powerFactor = NAN,
              .voltage = data.voltage,
            });
            output1.updateMetrics({
              .source = Mycila::Router::Source::PZEM,
              .apparentPower = data.apparentPower,
              .current = data.current,
              .energy = data.activeEnergy,
              .power = data.activePower,
              .powerFactor = data.powerFactor,
              .resistance = data.resistance(),
              .thdi = data.thdi(),
              .voltage = data.dimmedVoltage(),
            });
          }
        });

        pzemO1PairingTask = new Mycila::Task("PZEM Pairing 0x01", Mycila::Task::Type::ONCE, []() {
          ESP_LOGI(TAG, "Pairing connected PZEM to Output 1");
          pzemO1->end();

          if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_1_NAME) == 0)
            pzemO1->begin(Serial1, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);

#if SOC_UART_NUM > 2
          if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_2_NAME) == 0)
            pzemO1->begin(Serial2, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);
#endif

          switch (pzemO1->getDeviceAddress()) {
            case YASOLR_PZEM_ADDRESS_OUTPUT1:
              // already paired
              if (!config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
                // stop PZEM if it was not enabled
                pzemO1->end();
              }
              ESP_LOGW(TAG, "PZEM already paired to Output 1");
              break;
            case MYCILA_PZEM_ADDRESS_UNKNOWN:
              // no device found
              pzemO1->end();
              ESP_LOGE(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is powered and connected to Serial RX/TX!");
              break;
            default:
              // found a device
              if (pzemO1->setDeviceAddress(YASOLR_PZEM_ADDRESS_OUTPUT1)) {
                if (!config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
                  // stop PZEM if it was not enabled
                  pzemO1->end();
                }
                ESP_LOGI(TAG, "PZEM has been paired to Output 1");
              } else {
                pzemO1->end();
                ESP_LOGE(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is powered and connected to Serial RX/TX!");
              }
          }
        });

        unsafeTaskManager.addTask(*pzemO1PairingTask);

        if (config.getBool(KEY_ENABLE_DEBUG))
          pzemO1PairingTask->enableProfiling();

      } else {
        ESP_LOGE(TAG, "PZEM for Output 1 failed to initialize!");
        pzemO1->end();
        delete pzemO1;
        pzemO1 = nullptr;
      }
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
  if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM)) {
    init_read_task();

    if (pzemO2 == nullptr) {
      ESP_LOGI(TAG, "Enable Output 2 PZEM with UART %s", config.get(KEY_PZEM_UART));

      pzemO2 = new Mycila::PZEM();
      pzemO2->setSharedSerial(true);

      if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_1_NAME) == 0)
        pzemO2->begin(Serial1, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT2);

#if SOC_UART_NUM > 2
      if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_2_NAME) == 0)
        pzemO2->begin(Serial2, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT2);
#endif

      if (pzemO2->isEnabled()) {
        pzemO2->setCallback([](const Mycila::PZEM::EventType eventType, const Mycila::PZEM::Data& data) {
          if (eventType == Mycila::PZEM::EventType::EVT_READ) {
            grid.updateMetrics({
              .source = Mycila::Grid::Source::PZEM,
              .apparentPower = NAN,
              .current = NAN,
              .energy = 0,
              .energyReturned = 0,
              .frequency = data.frequency,
              .power = NAN,
              .powerFactor = NAN,
              .voltage = data.voltage,
            });
            output2.updateMetrics({
              .source = Mycila::Router::Source::PZEM,
              .apparentPower = data.apparentPower,
              .current = data.current,
              .energy = data.activeEnergy,
              .power = data.activePower,
              .powerFactor = data.powerFactor,
              .resistance = data.resistance(),
              .thdi = data.thdi(),
              .voltage = data.dimmedVoltage(),
            });
          }
        });

        pzemO2PairingTask = new Mycila::Task("PZEM Pairing 0x02", Mycila::Task::Type::ONCE, []() {
          ESP_LOGI(TAG, "Pairing connected PZEM to Output 2");
          pzemO2->end();

          if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_1_NAME) == 0)
            pzemO2->begin(Serial1, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);

#if SOC_UART_NUM > 2
          if (strcmp(config.get(KEY_PZEM_UART), YASOLR_UART_2_NAME) == 0)
            pzemO2->begin(Serial2, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);
#endif

          switch (pzemO2->getDeviceAddress()) {
            case YASOLR_PZEM_ADDRESS_OUTPUT2:
              // already paired
              if (!config.getBool(KEY_ENABLE_OUTPUT2_PZEM)) {
                // stop PZEM if it was not enabled
                pzemO2->end();
              }
              ESP_LOGW(TAG, "PZEM already paired to Output 2");
              break;
            case MYCILA_PZEM_ADDRESS_UNKNOWN:
              // no device found
              pzemO2->end();
              ESP_LOGE(TAG, "Failed to pair PZEM to Output 2: make sure only PZEM of Output 2 is powered and connected to Serial RX/TX!");
              break;
            default:
              // found a device
              if (pzemO2->setDeviceAddress(YASOLR_PZEM_ADDRESS_OUTPUT2)) {
                if (!config.getBool(KEY_ENABLE_OUTPUT2_PZEM)) {
                  // stop PZEM if it was not enabled
                  pzemO2->end();
                }
                ESP_LOGI(TAG, "PZEM has been paired to Output 2");
              } else {
                pzemO2->end();
                ESP_LOGE(TAG, "Failed to pair PZEM to Output 2: make sure only PZEM of Output 2 is powered and connected to Serial RX/TX!");
              }
          }
        });

        unsafeTaskManager.addTask(*pzemO2PairingTask);

        if (config.getBool(KEY_ENABLE_DEBUG))
          pzemO2PairingTask->enableProfiling();

      } else {
        ESP_LOGE(TAG, "PZEM for Output 2 failed to initialize!");
        pzemO2->end();
        delete pzemO2;
        pzemO2 = nullptr;
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
