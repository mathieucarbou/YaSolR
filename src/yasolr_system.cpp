// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <esp_core_dump.h>

Mycila::TaskManager coreTaskManager("loopTask");
Mycila::TaskManager unsafeTaskManager("unsafeTask");

Mycila::Task resetTask("Reset", Mycila::Task::Type::ONCE, [](void* params) {
  LOGW("YaSolR", "Resetting %s", Mycila::AppInfo.nameModelVersion.c_str());
  config.clear();
  Mycila::System::restart(500);
});

Mycila::Task restartTask("Restart", Mycila::Task::Type::ONCE, [](void* params) {
  LOGW("YaSolR", "Restarting %s", Mycila::AppInfo.nameModelVersion.c_str());
  Mycila::System::restart(500);
});

Mycila::Task safeBootTask("SafeBoot", Mycila::Task::Type::ONCE, [](void* params) {
  LOGI(TAG, "Restarting %s in SafeBoot mode", Mycila::AppInfo.nameModelVersion.c_str());
  // save current network configuration so that it can be restored and used by safeboot
  espConnect.saveConfiguration();
  Mycila::System::restartFactory(YASOLR_SAFEBOOT_PARTITION_NAME);
});

void yasolr_init_system() {
  LOGI(TAG, "Initialize system");

  Mycila::TaskManager::configureWDT(60, true);

  Mycila::System::init(true, "fs");

  esp_reset_reason_t reason = esp_reset_reason();
  if (reason == esp_reset_reason_t::ESP_RST_POWERON || reason == esp_reset_reason_t::ESP_RST_SW || reason == esp_reset_reason_t::ESP_RST_DEEPSLEEP) {
    LOGI(TAG, "Erasing core dump");
    esp_core_dump_image_erase();
  } else {
    LOGE(TAG, "ESP32 resumed from a crash: please look at the core dump info!");
  }

  coreTaskManager.addTask(resetTask);
  coreTaskManager.addTask(restartTask);
  coreTaskManager.addTask(safeBootTask);

  Mycila::TaskMonitor.addTask(coreTaskManager.name());   // YaSolR
  Mycila::TaskMonitor.addTask(unsafeTaskManager.name()); // YaSolR
}
