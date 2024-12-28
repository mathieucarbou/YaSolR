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
Mycila::TrafficLight lights;

YaSolR::Website website;

// hardware
Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;
Mycila::DS18 ds18O1;
Mycila::DS18 ds18O2;
Mycila::DS18 ds18Sys;
Mycila::EasyDisplay display(YASOLR_DISPLAY_LINES, YASOLR_DISPLAY_LINE_SIZE, 4, u8g2_font_6x12_tf);
Mycila::HA::Discovery haDiscovery;
Mycila::MQTT mqtt;
Mycila::PulseAnalyzer pulseAnalyzer;
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
