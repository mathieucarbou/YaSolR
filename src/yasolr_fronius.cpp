// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright © 2026, Mathieu Carbou and Remi Queyrut
 */
#include <yasolr.h>

#include <memory>
#include <utility>

Mycila::Fronius* fronius = nullptr;
Mycila::Task* froniusConnectTask = nullptr;

static Mycila::Task* froniusReadTask = nullptr;

void yasolr_configure_fronius() {
  if (grid.isUsing(Mycila::metric::Kind::FRONIUS)) {
    if (fronius == nullptr) {
      if (strlen(config.getString(KEY_FRONIUS_MODBUS_SERVER)) == 0) {
        ESP_LOGE(TAG, "Fronius Modbus TCP server is not set");
        return;
      }

      ESP_LOGI(TAG, "Enable Fronius Modbus TCP");

      // Fronius class handling Modbus TCP connection
      fronius = new Mycila::Fronius();

      // when receiving data from Fronius, update grid metrics
      fronius->setCallback([](Mycila::Fronius::EventType eventType) {
        if (eventType == Mycila::Fronius::EventType::EVT_READ) {
          std::unique_ptr<Mycila::metric::Metrics> metrics = std::make_unique<Mycila::metric::Metrics>();
          metrics->current = fronius->getCurrent();
          metrics->frequency = fronius->getFrequency();
          metrics->power = fronius->getPower();
          metrics->voltage = fronius->getVoltage();
          grid.updateMetrics(std::move(metrics));
          pidTask.requestEarlyRun();
        }
      });

      // task called once network is up to connect
      froniusConnectTask = new Mycila::Task("Fronius Connect", Mycila::Task::Type::ONCE, []() {
        fronius->end();
        const char* server = config.getString(KEY_FRONIUS_MODBUS_SERVER);
        uint16_t port = config.get<uint16_t>(KEY_FRONIUS_MODBUS_PORT);
        fronius->begin(server, port);
      });

      // reader
      froniusReadTask = new Mycila::Task("Fronius Read", []() { fronius->read(); });
      froniusReadTask->setInterval(500);

      // I/O tasks pinned to unsafe task manager
      unsafeTaskManager.addTask(*froniusConnectTask);
      unsafeTaskManager.addTask(*froniusReadTask);

      if (config.get<bool>(KEY_ENABLE_DEBUG)) {
        froniusConnectTask->enableProfiling();
        froniusReadTask->enableProfiling();
      }
    }
  } else {
    if (fronius != nullptr) {
      ESP_LOGI(TAG, "Disable Fronius Modbus TCP");

      if (froniusConnectTask) {
        unsafeTaskManager.removeTask(*froniusConnectTask);
        delete froniusConnectTask;
        froniusConnectTask = nullptr;
      }

      if (froniusReadTask) {
        unsafeTaskManager.removeTask(*froniusReadTask);
        delete froniusReadTask;
        froniusReadTask = nullptr;
      }

      fronius->end();
      delete fronius;
      fronius = nullptr;
    }
  }
}
