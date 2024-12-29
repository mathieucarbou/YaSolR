// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

AsyncWebServer webServer(80);
AuthenticationMiddleware authMiddleware;
LoggingMiddleware loggingMiddleware;
ESPDash dashboard = ESPDash(webServer, "/dashboard", false);

Mycila::Config config;
Mycila::ESPConnect espConnect(webServer);
Mycila::Grid grid;
Mycila::Logger logger;
Mycila::PID pidController;
Mycila::Router router(pidController);

YaSolR::Website website;

// hardware
Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;
Mycila::HA::Discovery haDiscovery;
Mycila::MQTT mqtt;
Mycila::PZEM pzemO1;
Mycila::PZEM pzemO2;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::RouterRelay routerRelay1(relay1);
Mycila::RouterRelay routerRelay2(relay2);
Mycila::RouterOutput output1("output1", dimmerO1, bypassRelayO1, pzemO1);
Mycila::RouterOutput output2("output2", dimmerO2, bypassRelayO2, pzemO2);
Mycila::TaskManager coreTaskManager("y-core");
Mycila::TaskManager unsafeTaskManager("y-unsafe");
Mycila::TaskManager pzemTaskManager("y-pzem");

void setup() {
  yasolr_boot();
  yasolr_configure();
  yasolr_event_listeners();
  yasolr_http();
  yasolr_rest_api();
  yasolr_mqtt_subscribers();
  yasolr_start_jsy();
  yasolr_start_ds18();
  yasolr_start_display();
  yasolr_start_lights();
  yasolr_start_zcd();

  logger.info(TAG, "Initializing dashboard");
  website.initLayout();
  website.initCards();
  website.updateCards();

  assert(coreTaskManager.asyncStart(512 * 8, 1, 1, 100, true));    // NOLINT
  assert(unsafeTaskManager.asyncStart(512 * 8, 1, 1, 100, false)); // NOLINT
  assert(pzemTaskManager.asyncStart(512 * 6, 5, 0, 100, true));    // NOLINT

  // STARTUP READY!
  logger.info(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }
