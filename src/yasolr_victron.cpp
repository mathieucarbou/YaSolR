// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Victron* victron = nullptr;
Mycila::Task* victronConnectTask = nullptr;

static Mycila::Task* victronReadTask = nullptr;

void yasolr_configure_victron() {
  if (config.getBool(KEY_ENABLE_VICTRON_MODBUS)) {
    if (victron == nullptr) {
      if (!config.getString(KEY_VICTRON_MODBUS_SERVER).length()) {
        LOGE(TAG, "Victron Modbus TCP server is not set");
        return;
      }

      LOGI(TAG, "Enable Victron Modbus TCP");

      // Victron class handling Modbus TCP connection
      victron = new Mycila::Victron();

      // when receiving data from Victron, update grid metrics
      victron->setCallback([](Mycila::Victron::EventType eventType) {
        if (eventType == Mycila::Victron::EventType::EVT_READ) {
          grid.metrics(Mycila::Grid::Source::REMOTE).update({
            .current = victron->getCurrent(),
            .frequency = victron->getFrequency(),
            .power = victron->getPower(),
            .voltage = victron->getVoltage(),
          });

          if (grid.getDataSource(Mycila::Grid::DataType::POWER) == Mycila::Grid::Source::REMOTE) {
            yasolr_divert();
          }
        }
      });

      // task called once network is up to connect
      victronConnectTask = new Mycila::Task("Victron Connect", Mycila::Task::Type::ONCE, [](void* params) {
        victron->end();
        const char* server = config.get(KEY_VICTRON_MODBUS_SERVER);
        uint16_t port = static_cast<uint16_t>(config.getLong(KEY_VICTRON_MODBUS_PORT));
        victron->begin(server, port);
      });

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
  } else {
    if (victron != nullptr) {
      LOGI(TAG, "Disable Victron Modbus TCP");

      if (victronConnectTask) {
        unsafeTaskManager.removeTask(*victronConnectTask);
        delete victronConnectTask;
        victronConnectTask = nullptr;
      }

      if (victronReadTask) {
        unsafeTaskManager.removeTask(*victronReadTask);
        delete victronReadTask;
        victronReadTask = nullptr;
      }

      victron->end();
      delete victron;
      victron = nullptr;
    }
  }
}
