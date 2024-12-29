// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::ESPConnect espConnect(webServer);
Mycila::Grid grid;
Mycila::PID pidController;
Mycila::Router router(pidController);

// hardware
Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;
Mycila::HA::Discovery haDiscovery;
Mycila::MQTT mqtt;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::RouterRelay routerRelay1(relay1);
Mycila::RouterRelay routerRelay2(relay2);
Mycila::RouterOutput output1("output1", dimmerO1, bypassRelayO1);
Mycila::RouterOutput output2("output2", dimmerO2, bypassRelayO2);
Mycila::TaskManager coreTaskManager("y-core");
Mycila::TaskManager unsafeTaskManager("y-unsafe");

void setup() {
  yasolr_boot();
  yasolr_start_config();

  yasolr_configure();
  yasolr_event_listeners();
  yasolr_start_rest_api();
  yasolr_mqtt_subscribers();

  yasolr_start_display();
  yasolr_start_ds18();
  yasolr_start_jsy();
  yasolr_start_lights();
  yasolr_start_logging();
  yasolr_start_pzem();
  yasolr_start_website();
  yasolr_start_zcd();

  yasolr_configure_logging();

  assert(coreTaskManager.asyncStart(512 * 8, 1, 1, 100, true));
  assert(unsafeTaskManager.asyncStart(512 * 8, 1, 1, 100, false));

  // STARTUP READY!
  logger.info(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }
