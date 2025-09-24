// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

void setup() {
  yasolr_init_logging(); // init logging
  yasolr_init_system();  // init system (safeboot, restart, reset, etc)

  LOGI(TAG, "Booting %s", Mycila::AppInfo.nameModelVersion.c_str());

  yasolr_init_config();       // load configuration from NVS
  yasolr_configure_logging(); // configure logging

  LOGI(TAG, "Starting %s", Mycila::AppInfo.nameModelVersion.c_str());

  yasolr_init_lights();
  yasolr_init_trial();
  yasolr_init_ds18();
  yasolr_init_grid();
  yasolr_init_relays();
  yasolr_init_router();
  yasolr_init_web_server();
  yasolr_init_network();

  yasolr_configure_display();
  yasolr_configure_jsy_remote();
  yasolr_configure_jsy();
  yasolr_configure_leds();
  yasolr_configure_mqtt();
  yasolr_configure_pid();
  yasolr_configure_relay1();
  yasolr_configure_relay2();
  yasolr_configure_router_ds18();
  yasolr_configure_victron();

  yasolr_configure_output1_bypass_relay();
  yasolr_configure_output2_bypass_relay();
  yasolr_configure_output1_pzem();
  yasolr_configure_output2_pzem();
  yasolr_configure_output1_ds18(); // must be before dimmer otherwise ISR will cause crash since ds18 initialization is time sensitive
  yasolr_configure_output2_ds18();
  yasolr_configure_output1_dimmer();
  yasolr_configure_output2_dimmer();

  // STARTUP READY!
  yasolr_start_task_manager();
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
