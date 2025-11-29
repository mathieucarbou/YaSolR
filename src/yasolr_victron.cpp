// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

#include <utility>

Mycila::Victron* victron = nullptr;
Mycila::Task* victronConnectTask = nullptr;

static Mycila::Task* victronReadTask = nullptr;

void yasolr_configure_victron() {
  if (config.get<bool>(KEY_ENABLE_VICTRON_MODBUS)) {
    if (victron == nullptr) {
      if (strlen(config.getString(KEY_VICTRON_MODBUS_SERVER)) == 0) {
        ESP_LOGE(TAG, "Victron Modbus TCP server is not set");
        return;
      }

      ESP_LOGI(TAG, "Enable Victron Modbus TCP");

      // Victron class handling Modbus TCP connection
      victron = new Mycila::Victron();

      // when receiving data from Victron, update grid metrics
      victron->setCallback([](Mycila::Victron::EventType eventType) {
        if (eventType == Mycila::Victron::EventType::EVT_READ) {
          Mycila::Grid::Metrics metrics;
          metrics.source = Mycila::Grid::Source::VICTRON;
          metrics.current = victron->getCurrent();
          metrics.frequency = victron->getFrequency();
          metrics.power = victron->getPower();
          metrics.voltage = victron->getVoltage();
          grid.updateMetrics(std::move(metrics));

          if (grid.isUsing(Mycila::Grid::Source::VICTRON)) {
            yasolr_run_pid();
          }
        }
      });

      // task called once network is up to connect
      victronConnectTask = new Mycila::Task("Victron Connect", Mycila::Task::Type::ONCE, []() {
        victron->end();
        const char* server = config.getString(KEY_VICTRON_MODBUS_SERVER);
        uint16_t port = config.get<uint16_t>(KEY_VICTRON_MODBUS_PORT);
        victron->begin(server, port);
      });

      // reader
      victronReadTask = new Mycila::Task("Victron Read", []() { victron->read(); });
      victronReadTask->setInterval(500);

      // I/O tasks pinned to unsafe task manager
      unsafeTaskManager.addTask(*victronConnectTask);
      unsafeTaskManager.addTask(*victronReadTask);

      if (config.get<bool>(KEY_ENABLE_DEBUG)) {
        victronConnectTask->enableProfiling();
        victronReadTask->enableProfiling();
      }
    }
  } else {
    if (victron != nullptr) {
      ESP_LOGI(TAG, "Disable Victron Modbus TCP");

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

      grid.deleteMetrics(Mycila::Grid::Source::VICTRON);
    }
  }
}
