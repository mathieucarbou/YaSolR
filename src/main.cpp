// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Config config;
Mycila::DS18 ds18O1;
Mycila::DS18 ds18O2;
Mycila::DS18 ds18Sys;
Mycila::EasyDisplay display(YASOLR_DISPLAY_LINES, YASOLR_DISPLAY_LINE_SIZE, 4, u8g2_font_6x12_tf);
Mycila::Grid grid;
Mycila::HA::Discovery haDiscovery;
Mycila::JSY jsy;
Mycila::Logger logger;
Mycila::MQTT mqtt;
Mycila::PID pidController;
Mycila::PulseAnalyzer pulseAnalyzer;
Mycila::PZEM pzemO1;
Mycila::PZEM pzemO2;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::TrafficLight lights;

Mycila::TaskManager coreTaskManager("y-core");
Mycila::TaskManager jsyTaskManager("y-jsy");
Mycila::TaskManager mqttTaskManager("y-mqtt");
Mycila::TaskManager pioTaskManager("y-pio");
Mycila::TaskManager pzemTaskManager("y-pzem");
Mycila::TaskManager routingTaskManager("y-router");

Mycila::Router router(pidController, jsy);

Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;

Mycila::RouterOutput output1("output1", dimmerO1, bypassRelayO1, pzemO1);
Mycila::RouterOutput output2("output2", dimmerO2, bypassRelayO2, pzemO2);

Mycila::RouterRelay routerRelay1(relay1);
Mycila::RouterRelay routerRelay2(relay2);

AsyncUDP udp;
Mycila::CircularBuffer<float, 50> udpMessageRateBuffer;

AsyncWebServer webServer(80);
AsyncWebSocket wsDebugPID("/ws/pid/csv");
AuthenticationMiddleware authMiddleware;
LoggingMiddleware loggingMiddleware;
ESPDash dashboard = ESPDash(&webServer, "/dashboard", false);
Mycila::ESPConnect espConnect(webServer);

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

  assert(    jsyTaskManager.asyncStart(512 * 6, 5, 0, 100, true));  // NOLINT
  assert(    pioTaskManager.asyncStart(512 * 7, 1, 1, 100, true));  // NOLINT
  assert(   coreTaskManager.asyncStart(512 * 8, 1, 1, 100, true));  // NOLINT
  assert(   mqttTaskManager.asyncStart(512 * 7, 1, 1, 100, false)); // NOLINT
  assert(   pzemTaskManager.asyncStart(512 * 6, 5, 0, 100, true));  // NOLINT
  assert(routingTaskManager.asyncStart(512 * 4, 5, 0, 100, true));  // NOLINT

  // STARTUP READY!
  logger.info(TAG, "Started %s", Mycila::AppInfo.nameModelVersion.c_str());
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }
