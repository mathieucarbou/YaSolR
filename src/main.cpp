// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::PID pidController;
Mycila::Router router(pidController);
Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::RouterRelay routerRelay1(relay1);
Mycila::RouterRelay routerRelay2(relay2);
Mycila::RouterOutput output1("output1", dimmerO1, bypassRelayO1);
Mycila::RouterOutput output2("output2", dimmerO2, bypassRelayO2);

void setup() {
  // boot
  yasolr_boot();
  // config
  yasolr_init_config();
  // logging
  yasolr_init_logging();
  // system
  yasolr_init_system();
  // hardware
  yasolr_init_display();
  yasolr_init_ds18();
  yasolr_init_grid();
  yasolr_init_jsy();
  yasolr_init_jsy_remote();
  yasolr_init_lights();
  yasolr_init_mqtt();
  yasolr_init_network();
  yasolr_init_pzem();
  yasolr_init_trial();
  yasolr_init_web_server();
  yasolr_init_zcd();
  // logging configuration
  yasolr_configure_logging();

  yasolr_configure(); // TODO

  // core task manager
  assert(coreTaskManager.asyncStart(512 * 8, 1, 1, 100, true));

  // task manager for long running tasks like mqtt / pzem
  if (unsafeTaskManager.getSize())
    assert(unsafeTaskManager.asyncStart(512 * 8, 1, 1, 100, false));

  // STARTUP READY!
  logger.info(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }
