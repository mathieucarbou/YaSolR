// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

Mycila::Config config;
Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;
Mycila::DS18 ds18O1;
Mycila::DS18 ds18O2;
Mycila::DS18 ds18Sys;
Mycila::EasyDisplay display(YASOLR_DISPLAY_LINES, YASOLR_DISPLAY_LINE_SIZE, 4, u8g2_font_6x12_tf);
Mycila::HADiscovery haDiscovery;
Mycila::JSY jsy;
Mycila::Logger logger;
Mycila::MQTT mqtt;
Mycila::PZEM pzemO1;
Mycila::PZEM pzemO2;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::TrafficLight lights;

Mycila::TaskManager ioTaskManager("I/O");
Mycila::TaskManager jsyTaskManager("JSY");
Mycila::TaskManager coreTaskManager("Core");
Mycila::TaskManager pzemTaskManager("PZEM");
Mycila::TaskManager routerTaskManager("Router");

Mycila::RouterOutput output1("output1", dimmerO1, ds18O1, bypassRelayO1, pzemO1);
Mycila::RouterOutput output2("output2", dimmerO2, ds18O2, bypassRelayO2, pzemO2);

Mycila::RouterRelay routerRelay1(relay1);
Mycila::RouterRelay routerRelay2(relay2);

Mycila::Grid grid(jsy);
Mycila::Router router(jsy);

AsyncWebServer webServer(80);
ESPDash dashboard = ESPDash(&webServer, "/dashboard", false);

void setup() {
  bootTask.forceRun();
  initLoggingTask.forceRun();
  initCoreTask.forceRun();
  initConfigTask.forceRun();
  initEventsTask.forceRun();
  initWebTask.forceRun();
  initRestApiTask.forceRun();
  initMqttSubscribersTask.forceRun();
  initDashboardTask.forceRun();

  assert(  coreTaskManager.asyncStart(1024 * 4, 1, 1, 100, true)); // NOLINT
  assert(    ioTaskManager.asyncStart(1024 * 5, 1, 1, 100, true)); // NOLINT
  assert(   jsyTaskManager.asyncStart(1024 * 3, 5, 0, 100, true)); // NOLINT
  assert(  pzemTaskManager.asyncStart(1024 * 3, 5, 0, 100, true)); // NOLINT
  assert(routerTaskManager.asyncStart(1024 * 3, 5, 1, 100, true)); // NOLINT

  // STARTUP READY!
  logger.info(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());

#ifdef YASOLR_DEBUG
  Mycila::Dimmer::generateLUT(Serial);
#endif
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }
