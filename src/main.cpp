// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Config config;
Mycila::DS18 ds18O1;
Mycila::DS18 ds18O2;
Mycila::DS18 ds18Sys;
Mycila::EasyDisplay display(YASOLR_DISPLAY_LINES, YASOLR_DISPLAY_LINE_SIZE, 4, u8g2_font_6x12_tf);
Mycila::Grid grid;
Mycila::HADiscovery haDiscovery;
Mycila::JSY jsy;
Mycila::Logger logger;
Mycila::MQTT mqtt;
Mycila::PID pidController;
Mycila::PZEM pzemO1;
Mycila::PZEM pzemO2;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::TrafficLight lights;
Mycila::ZCD zcd;

Mycila::TaskManager ioTaskManager("yasolr-io");
Mycila::TaskManager jsyTaskManager("yasolr-jsy");
Mycila::TaskManager core0TaskManager("yasolr-core-0");
Mycila::TaskManager core1TaskManager("yasolr-core-1");
Mycila::TaskManager pzemTaskManager("yasolr-pzem");
Mycila::TaskManager routerTaskManager("yasolr-router");

Mycila::Router router(pidController, jsy);

Mycila::Dimmer dimmerO1(zcd);
Mycila::Dimmer dimmerO2(zcd);

Mycila::RouterOutput output1("output1", dimmerO1, bypassRelayO1, ds18O1, pzemO1);
Mycila::RouterOutput output2("output2", dimmerO2, bypassRelayO2, ds18O2, pzemO2);

Mycila::RouterRelay routerRelay1(relay1);
Mycila::RouterRelay routerRelay2(relay2);

Mycila::CircularBuffer<float, 50> udpMessageRateBuffer;

AsyncUDP udp;
AsyncWebServer webServer(80);
AsyncWebSocket wsDebugPID("/ws/debug/pid");
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
  initDashboardCards.forceRun();

  assert( core0TaskManager.asyncStart(512 * 6, 1, 0, 100, true));  // NOLINT
  assert( core1TaskManager.asyncStart(512 * 6, 1, 1, 100, true));  // NOLINT
  assert(    ioTaskManager.asyncStart(512 * 9, 1, 1, 100, false)); // NOLINT
  assert(   jsyTaskManager.asyncStart(512 * 4, 5, 0, 100, true));  // NOLINT
  assert(  pzemTaskManager.asyncStart(512 * 4, 5, 0, 100, true));  // NOLINT
  assert(routerTaskManager.asyncStart(512 * 6, 5, 1, 100, true));  // NOLINT

  // STARTUP READY!
  logger.info(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }
