// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <AsyncJson.h>
#include <AsyncUDP.h>
#include <CRC.h>
#include <ESPAsyncWebServer.h>
#include <FastCRC32.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <StreamString.h>

#include <MycilaAppInfo.h>
#include <MycilaConfig.h>
#include <MycilaDS18.h>
#include <MycilaDimmer.h>
#include <MycilaESPConnect.h>
#include <MycilaEasyDisplay.h>
#include <MycilaGrid.h>
#include <MycilaHADiscovery.h>
#include <MycilaJSY.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaNTP.h>
#include <MycilaPulseAnalyzer.h>
#include <MycilaPZEM004Tv3.h>
#include <MycilaRelay.h>
#include <MycilaRouter.h>
#include <MycilaRouterOutput.h>
#include <MycilaRouterRelay.h>
#include <MycilaSystem.h>
#include <MycilaTaskManager.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTrafficLight.h>
#include <MycilaUtilities.h>

#ifdef APP_MODEL_TRIAL
  #include <MycilaTrial.h>
#endif

#ifdef APP_MODEL_PRO
  #include <ESPDashPro.h>
  #include <WebSerialPro.h>
#else
  #include <ESPDash.h>
  #include <MycilaWebSerial.h>
#endif

#include <YaSolRDefines.h>

extern AsyncWebServer webServer;
extern AuthenticationMiddleware authMiddleware;
extern LoggingMiddleware loggingMiddleware;
extern ESPDash dashboard;

extern Mycila::CircularBuffer<float, 50> udpMessageRateBuffer;
extern Mycila::Config config;
extern Mycila::ESPConnect espConnect;
extern Mycila::Grid grid;
extern Mycila::Logger logger;
extern Mycila::PID pidController;
extern Mycila::Router router;
extern Mycila::TrafficLight lights;

// hardware
extern AsyncUDP udp;
extern Mycila::Dimmer dimmerO1;
extern Mycila::Dimmer dimmerO2;
extern Mycila::DS18 ds18O1;
extern Mycila::DS18 ds18O2;
extern Mycila::DS18 ds18Sys;
extern Mycila::EasyDisplay display;
extern Mycila::HA::Discovery haDiscovery;
extern Mycila::JSY jsy;
extern Mycila::MQTT mqtt;
extern Mycila::PulseAnalyzer pulseAnalyzer;
extern Mycila::PZEM pzemO1;
extern Mycila::PZEM pzemO2;
extern Mycila::Relay bypassRelayO1;
extern Mycila::Relay bypassRelayO2;
extern Mycila::Relay relay1;
extern Mycila::Relay relay2;
extern Mycila::RouterRelay routerRelay1;
extern Mycila::RouterRelay routerRelay2;
extern Mycila::RouterOutput output1;
extern Mycila::RouterOutput output2;

extern Mycila::TaskManager coreTaskManager;
extern Mycila::Task calibrationTask;
extern Mycila::Task dashboardInitTask;
extern Mycila::Task dashboardUpdateTask;
extern Mycila::Task debugTask;
extern Mycila::Task displayCarouselTask;
extern Mycila::Task displayTask;
extern Mycila::Task ds18Task;
extern Mycila::Task lightsTask;
extern Mycila::Task loggingTask;
extern Mycila::Task networkStartTask;
extern Mycila::Task networkManagerTask;
extern Mycila::Task relayTask;
extern Mycila::Task resetTask;
extern Mycila::Task restartTask;
extern Mycila::Task routerTask;
extern Mycila::Task safeBootTask;
extern Mycila::Task zcdTask;
#ifdef APP_MODEL_TRIAL
extern Mycila::Task trialTask;
#endif

extern Mycila::TaskManager unsafeTaskManager;
extern Mycila::Task haDiscoveryTask;
extern Mycila::Task mqttConfigTask;
extern Mycila::Task mqttPublishConfigTask;
extern Mycila::Task mqttPublishStaticTask;
extern Mycila::Task mqttPublishTask;
extern Mycila::Task pzemO1PairingTask;
extern Mycila::Task pzemO2PairingTask;

extern Mycila::TaskManager jsyTaskManager;
extern Mycila::Task jsyTask;

extern Mycila::TaskManager pzemTaskManager;
extern Mycila::Task pzemTask;

// fn
extern float yasolr_frequency();
extern void yasolr_boot();
extern void yasolr_configure();
extern void yasolr_divert();
extern void yasolr_http();
extern void yasolr_event_listeners();
extern void yasolr_rest_api();
extern void yasolr_mqtt_subscribers();

enum class DisplayKind {
  DISPLAY_HOME = 1,
  DISPLAY_NETWORK,
  DISPLAY_ROUTER,
  DISPLAY_OUTPUT1,
  DISPLAY_OUTPUT2,
};
