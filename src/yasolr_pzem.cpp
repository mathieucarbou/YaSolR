// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::PZEM* pzemO1 = nullptr;
Mycila::PZEM* pzemO2 = nullptr;
Mycila::Task* pzemO1PairingTask = nullptr;
Mycila::Task* pzemO2PairingTask = nullptr;
Mycila::Task* pzemTask = nullptr;
Mycila::TaskManager* pzemTaskManager = nullptr;

void yasolr_init_pzem() {
  logger.info(TAG, "Initialize PZEM...");
  uint8_t count = 0;

  if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
    pzemO1 = new Mycila::PZEM();
    pzemO1->begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT1);

    if (pzemO1->isEnabled()) {
      count++;
      pzemO1->setCallback([](const Mycila::PZEM::EventType eventType) {
        if (eventType == Mycila::PZEM::EventType::EVT_READ) {
          grid.pzemMetrics().update({
            .apparentPower = NAN,
            .current = NAN,
            .energy = NAN,
            .energyReturned = NAN,
            .frequency = pzemO1->data.frequency,
            .power = NAN,
            .powerFactor = NAN,
            .voltage = pzemO1->data.voltage,
          });
          if (output1)
            output1->localMetrics().update({
              .apparentPower = pzemO1->data.apparentPower,
              .current = pzemO1->data.current,
              .dimmedVoltage = pzemO1->data.dimmedVoltage(),
              .energy = pzemO1->data.activeEnergy,
              .power = pzemO1->data.activePower,
              .powerFactor = pzemO1->data.powerFactor,
              .resistance = pzemO1->data.resistance(),
              .thdi = pzemO1->data.thdi(),
              .voltage = pzemO1->data.voltage,
            });
        }
      });

      pzemO1PairingTask = new Mycila::Task("PZEM Pairing 0x01", Mycila::TaskType::ONCE, [](void* params) {
        logger.info(TAG, "Pairing connected PZEM to Output 1...");
        pzemO1->end();
        pzemO1->begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);
        switch (pzemO1->getDeviceAddress()) {
          case YASOLR_PZEM_ADDRESS_OUTPUT1:
            // already paired
            if (!config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
              // stop PZEM if it was not enabled
              pzemO1->end();
            }
            logger.warn(TAG, "PZEM already paired to Output 1");
            break;
          case MYCILA_PZEM_ADDRESS_UNKNOWN:
            // no device found
            pzemO1->end();
            logger.error(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is powered and connected to Serial RX/TX!");
            break;
          default:
            // found a device
            if (pzemO1->setDeviceAddress(YASOLR_PZEM_ADDRESS_OUTPUT1)) {
              if (!config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
                // stop PZEM if it was not enabled
                pzemO1->end();
              }
              logger.info(TAG, "PZEM has been paired to Output 1");
            } else {
              pzemO1->end();
              logger.error(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is powered and connected to Serial RX/TX!");
            }
        }
      });
      pzemO1PairingTask->setManager(unsafeTaskManager);
      if (config.getBool(KEY_ENABLE_DEBUG))
        pzemO1PairingTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);

    } else {
      logger.error(TAG, "PZEM for Output 1 failed to initialize!");
      pzemO1->end();
      delete pzemO1;
      pzemO1 = nullptr;
    }
  }

  if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM)) {
    pzemO2 = new Mycila::PZEM();
    pzemO2->begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT2);

    if (pzemO2->isEnabled()) {
      count++;
      pzemO2->setCallback([](const Mycila::PZEM::EventType eventType) {
        if (eventType == Mycila::PZEM::EventType::EVT_READ) {
          grid.pzemMetrics().update({
            .apparentPower = NAN,
            .current = NAN,
            .energy = NAN,
            .energyReturned = NAN,
            .frequency = pzemO2->data.frequency,
            .power = NAN,
            .powerFactor = NAN,
            .voltage = pzemO2->data.voltage,
          });
          if (output2)
            output2->localMetrics().update({
              .apparentPower = pzemO2->data.apparentPower,
              .current = pzemO2->data.current,
              .dimmedVoltage = pzemO2->data.dimmedVoltage(),
              .energy = pzemO2->data.activeEnergy,
              .power = pzemO2->data.activePower,
              .powerFactor = pzemO2->data.powerFactor,
              .resistance = pzemO2->data.resistance(),
              .thdi = pzemO2->data.thdi(),
              .voltage = pzemO2->data.voltage,
            });
        }
      });

      pzemO2PairingTask = new Mycila::Task("PZEM Pairing 0x02", Mycila::TaskType::ONCE, [](void* params) {
        logger.info(TAG, "Pairing connected PZEM to Output 2...");
        pzemO2->end();
        pzemO2->begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);
        switch (pzemO2->getDeviceAddress()) {
          case YASOLR_PZEM_ADDRESS_OUTPUT2:
            // already paired
            if (!config.getBool(KEY_ENABLE_OUTPUT2_PZEM)) {
              // stop PZEM if it was not enabled
              pzemO2->end();
            }
            logger.warn(TAG, "PZEM already paired to Output 2");
            break;
          case MYCILA_PZEM_ADDRESS_UNKNOWN:
            // no device found
            pzemO2->end();
            logger.error(TAG, "Failed to pair PZEM to Output 2: make sure only PZEM of Output 2 is powered and connected to Serial RX/TX!");
            break;
          default:
            // found a device
            if (pzemO2->setDeviceAddress(YASOLR_PZEM_ADDRESS_OUTPUT2)) {
              if (!config.getBool(KEY_ENABLE_OUTPUT2_PZEM)) {
                // stop PZEM if it was not enabled
                pzemO2->end();
              }
              logger.info(TAG, "PZEM has been paired to Output 2");
            } else {
              pzemO2->end();
              logger.error(TAG, "Failed to pair PZEM to Output 2: make sure only PZEM of Output 2 is powered and connected to Serial RX/TX!");
            }
        }
      });
      pzemO2PairingTask->setManager(unsafeTaskManager);
      if (config.getBool(KEY_ENABLE_DEBUG))
        pzemO2PairingTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);

    } else {
      logger.error(TAG, "PZEM for Output 2 failed to initialize!");
      pzemO2->end();
      delete pzemO2;
      pzemO2 = nullptr;
    }
  }

  if (count) {
    pzemTaskManager = new Mycila::TaskManager("y-pzem");

    pzemTask = new Mycila::Task("PZEM", [](void* params) {
      if (pzemO1) {
        pzemO1->read();
        yield();
      }
      if (pzemO2) {
        pzemO2->read();
        yield();
      }
    });
    pzemTask->setEnabledWhen([]() { return (!pzemO1PairingTask || pzemO1PairingTask->isPaused()) && (!pzemO2PairingTask || pzemO2PairingTask->isPaused()); });
    pzemTask->setManager(*pzemTaskManager);
    if (config.getBool(KEY_ENABLE_DEBUG))
      pzemTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);

    assert(pzemTaskManager->asyncStart(512 * 4, 5, 0, 100, true));

    Mycila::TaskMonitor.addTask(pzemTaskManager->getName());
  }
}
