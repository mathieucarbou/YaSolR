// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Victron* victron = nullptr;
Mycila::Task* victronConnectTask = nullptr;

static Mycila::Task* victronReadTask = nullptr;

static void connect() {
  victron->end();
  const char* server = config.get(KEY_VICTRON_MODBUS_SERVER);
  uint16_t port = static_cast<uint16_t>(config.getLong(KEY_VICTRON_MODBUS_PORT));
  victron->begin(server, port);
}

void yasolr_init_victron() {
  if (config.getBool(KEY_ENABLE_VICTRON_MODBUS)) {
    LOGI(TAG, "Initialize Victron Modbus TCP");

    if (!config.getString(KEY_VICTRON_MODBUS_SERVER).length()) {
      LOGE(TAG, "Victron Modbus TCP server is not set");
      return;
    }

    // Victron class handling Modbus TCP connection
    victron = new Mycila::Victron();

    // when receiving data from Victron, update grid metrics
    victron->setCallback([](Mycila::Victron::EventType eventType) {
      if (eventType == Mycila::Victron::EventType::EVT_READ) {
        grid.remoteMetrics().update({
          .current = victron->getCurrent(),
          .frequency = victron->getFrequency(),
          .power = victron->getPower(),
          .voltage = victron->getVoltage(),
        });

        if (grid.updatePower()) {
          yasolr_divert();
        }
      }
    });

    // task called once network is up to connect
    victronConnectTask = new Mycila::Task("Victron Connect", Mycila::Task::Type::ONCE, [](void* params) { connect(); });

    // reader
    victronReadTask = new Mycila::Task("Victron Read", [](void* params) { victron->read(); });
    victronReadTask->setInterval(500);

    // I/O tasks pinned to unsafe task manager
    unsafeTaskManager.addTask(*victronConnectTask);
    unsafeTaskManager.addTask(*victronReadTask);

    if (config.getBool(KEY_ENABLE_DEBUG)) {
      victronConnectTask->enableProfiling();
      victronReadTask->enableProfiling();
    }
  }
}
