// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

void setup() {
  yasolr_init_logging(); // init logging
  yasolr_init_system();  // init system (safeboot, restart, reset, etc)

  ESP_LOGI(TAG, "Booting %s", Mycila::AppInfo.nameModelVersion.c_str());

  yasolr_init_config();             // load configuration from NVS
  yasolr_init_persistent_logging(); // configure persistent logging if enabled
  yasolr_configure_logging();       // configure logging

  ESP_LOGI(TAG, "Starting %s", Mycila::AppInfo.nameModelVersion.c_str());

  yasolr_init_lights();
  yasolr_init_trial();
  yasolr_init_ds18();
  yasolr_init_relays();
  yasolr_init_router();
  yasolr_init_web_server();
  yasolr_init_version_check();
  yasolr_init_network();

  // DS18 first since we need to wait and loop
  yasolr_configure_output1_ds18();
  yasolr_configure_output2_ds18();
  yasolr_configure_router_ds18();

  // Dimmer second - need speed and not a lot of disk writes for ISR during their init
  yasolr_configure_output1_dimmer();
  yasolr_configure_output2_dimmer();

  // PZEM and JSY are long to initialize and do not cause a lot of logs so they will not impact dimmer ISR
  yasolr_configure_output1_pzem();
  yasolr_configure_output2_pzem();
  yasolr_configure_jsy();

  yasolr_configure_output1_bypass_relay();
  yasolr_configure_output2_bypass_relay();

  yasolr_configure_display();
  yasolr_configure_jsy_remote();
  yasolr_configure_lights();
  yasolr_configure_mqtt();
  yasolr_configure_pid();
  yasolr_configure_relay1();
  yasolr_configure_relay2();
  yasolr_configure_victron();

  // STARTUP READY!
  assert(unsafeTaskManager.asyncStart(4608, 1, 1, 100, true));
  ESP_LOGI(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());

  // startup finished: do not save logs at next boot
  config.set(KEY_ENABLE_DEBUG_BOOT, YASOLR_FALSE);
}

// Destroy default Arduino async task
void loop() { coreTaskManager.loop(); }
