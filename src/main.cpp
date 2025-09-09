// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

void setup() {
  yasolr_init_console_logging(); // init logging
  yasolr_init_system();          // init system (safeboot, restart, reset, etc)
  yasolr_init_startup_logging(); // save early logs to file

  LOGI(TAG, "Booting %s", Mycila::AppInfo.nameModelVersion.c_str());
  yasolr_init_config();       // load configuration from NVS
  yasolr_configure_logging(); // configure logging

  LOGI(TAG, "Starting %s", Mycila::AppInfo.nameModelVersion.c_str());
  yasolr_init_lights();
  yasolr_init_trial();
  // measurements
  yasolr_init_ds18();
  yasolr_init_jsy();
  yasolr_init_jsy_remote();
  yasolr_init_pzem();
  // router hardware
  yasolr_init_relays();
  yasolr_init_router();
  yasolr_init_grid();
  // UI: display, web, mqtt, etc
  yasolr_init_display();
  yasolr_init_web_server();
  yasolr_init_mqtt();
  yasolr_init_victron();
  // network
  yasolr_init_network();
  // start tasks
  yasolr_init_tasks();

  // STARTUP READY!
  LOGI(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }

// #include "esp_heap_caps.h"
// void esp_heap_trace_alloc_hook(void* ptr, size_t size, uint32_t caps) {
//   if (size >= 1024 && caps == 6144) {
//     ets_printf("alloc: %p %d\n", ptr, size);
//     if (size > 2308) {
//       assert(false);
//     }
//   }
// }
// void esp_heap_trace_free_hook(void* ptr) {
//   // ets_printf("free: %p\n", ptr);
// }
