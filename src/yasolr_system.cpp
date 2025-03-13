// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <esp_core_dump.h>

Mycila::TaskManager coreTaskManager("y-core");
Mycila::TaskManager unsafeTaskManager("y-unsafe");

Mycila::Task resetTask("Reset", Mycila::Task::Type::ONCE, [](void* params) {
  logger.warn("YaSolR", "Resetting %s", Mycila::AppInfo.nameModelVersion.c_str());
  config.clear();
  Mycila::System::restart(500);
});

Mycila::Task restartTask("Restart", Mycila::Task::Type::ONCE, [](void* params) {
  logger.warn("YaSolR", "Restarting %s", Mycila::AppInfo.nameModelVersion.c_str());
  Mycila::System::restart(500);
});

Mycila::Task safeBootTask("SafeBoot", Mycila::Task::Type::ONCE, [](void* params) {
  logger.info(TAG, "Restarting %s in SafeBoot mode", Mycila::AppInfo.nameModelVersion.c_str());
  // save current network configuration so that it can be restored and used by safeboot
  espConnect.saveConfiguration();
  Mycila::System::restartFactory("safeboot");
});

void yasolr_init_system() {
  logger.info(TAG, "Initialize system");

  esp_reset_reason_t reason = esp_reset_reason();
  if (reason == esp_reset_reason_t::ESP_RST_POWERON || reason == esp_reset_reason_t::ESP_RST_SW || reason == esp_reset_reason_t::ESP_RST_DEEPSLEEP) {
    logger.info(TAG, "Erasing core dump");
    esp_core_dump_image_erase();
  } else {
    logger.error(TAG, "ESP32 resumed from a crash: please look at the core dump info!");
  }

  coreTaskManager.addTask(resetTask);
  coreTaskManager.addTask(restartTask);
  coreTaskManager.addTask(safeBootTask);

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    coreTaskManager.enableProfiling();
    unsafeTaskManager.enableProfiling();
  }

  Mycila::TaskMonitor.addTask(coreTaskManager.name());   // YaSolR
  Mycila::TaskMonitor.addTask(unsafeTaskManager.name()); // YaSolR

  // Mycila::TaskMonitor.addTask("arduino_events"); // used bt non controlable
  // Mycila::TaskMonitor.addTask("https_ota_task"); // unused
  // Mycila::TaskMonitor.addTask("i2c_slave_task");
  // Mycila::TaskMonitor.addTask("loopTask"); // killed
  // Mycila::TaskMonitor.addTask("msc_disk"); // unused
  // Mycila::TaskMonitor.addTask("spp_tx"); // unused
  // Mycila::TaskMonitor.addTask("toneTask"); // unused
  // Mycila::TaskMonitor.addTask("uart_event_task"); // unused
  // Mycila::TaskMonitor.addTask("usbd"); // unused
  // Mycila::TaskMonitor.addTask("Zigbee_main"); // unused
}
