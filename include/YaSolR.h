// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <AsyncUDP.h>
#include <ESPAsyncWebServer.h>
#include <HardwareSerial.h>
#include <LittleFS.h>

#include <MycilaAppInfo.h>
#include <MycilaCircularBuffer.h>
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
#include <MycilaPZEM004Tv3.h>
#include <MycilaRelay.h>
#include <MycilaRouter.h>
#include <MycilaRouterOutput.h>
#include <MycilaRouterRelay.h>
#include <MycilaSystem.h>
#include <MycilaTaskManager.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTime.h>
#include <MycilaTrafficLight.h>
#include <MycilaZCD.h>

#ifdef APP_MODEL_TRIAL
#include <MycilaTrial.h>
#endif

#ifdef APP_MODEL_PRO
#include <ESPDashPro.h>
#include <ElegantOTAPro.h>
#include <WebSerialPro.h>
#else
#include <ESPDash.h>
#include <ElegantOTA.h>
#include <WebSerial.h>
#endif

#include <YaSolRDefines.h>

extern AsyncWebServer webServer;
extern AsyncUDP udp;
extern ESPDash dashboard;

extern Mycila::Config config;
extern Mycila::Dimmer dimmerO1;
extern Mycila::Dimmer dimmerO2;
extern Mycila::DS18 ds18O1;
extern Mycila::DS18 ds18O2;
extern Mycila::DS18 ds18Sys;
extern Mycila::EasyDisplay display;
extern Mycila::Grid grid;
extern Mycila::HADiscovery haDiscovery;
extern Mycila::JSY jsy;
extern Mycila::Logger logger;
extern Mycila::MQTT mqtt;
extern Mycila::PZEM pzemO1;
extern Mycila::PZEM pzemO2;
extern Mycila::Relay bypassRelayO1;
extern Mycila::Relay bypassRelayO2;
extern Mycila::Relay relay1;
extern Mycila::Relay relay2;
extern Mycila::Router router;
extern Mycila::RouterOutput output1;
extern Mycila::RouterOutput output2;
extern Mycila::RouterRelay routerRelay1;
extern Mycila::RouterRelay routerRelay2;
extern Mycila::TrafficLight lights;
extern Mycila::ZCD zcd;

extern Mycila::CircularBuffer<uint32_t, YASOLR_GRAPH_POINTS> timeHistory;
extern Mycila::CircularBuffer<int, YASOLR_GRAPH_POINTS> gridPowerHistory;
extern Mycila::CircularBuffer<int, YASOLR_GRAPH_POINTS> routedPowerHistory;
extern Mycila::CircularBuffer<int, YASOLR_GRAPH_POINTS> routerTHDiHistory;

extern Mycila::TaskManager ioTaskManager;
extern Mycila::Task haDiscoveryTask;
extern Mycila::Task mqttPublishTask;
extern Mycila::Task mqttPublishStaticTask;
extern Mycila::Task mqttPublishConfigTask;

extern Mycila::TaskManager jsyTaskManager;
extern Mycila::Task jsyTask;

extern Mycila::TaskManager coreTaskManager;
extern Mycila::Task carouselTask;
extern Mycila::Task dashboardCards;
extern Mycila::Task dashboardCharts;
extern Mycila::Task displayTask;
extern Mycila::Task ds18Task;
extern Mycila::Task lightsTask;
extern Mycila::Task mqttConfigTask;
extern Mycila::Task networkManagerTask;
extern Mycila::Task networkUpTask;
extern Mycila::Task otaTask;
extern Mycila::Task profilerTask;
extern Mycila::Task resetTask;
extern Mycila::Task restartTask;
extern Mycila::Task routerDebugTask;
#ifdef APP_MODEL_TRIAL
extern Mycila::Task trialTask;
#endif

extern Mycila::TaskManager pzemTaskManager;
extern Mycila::Task pzemO1PairingTask;
extern Mycila::Task pzemO2PairingTask;
extern Mycila::Task pzemTask;

extern Mycila::TaskManager routerTaskManager;
extern Mycila::Task relayTask;
extern Mycila::Task routerTask;
extern Mycila::Task routingTask;

// Tasks alone without a manager
extern Mycila::Task bootTask;
extern Mycila::Task initConfigTask;
extern Mycila::Task initCoreTask;
extern Mycila::Task initDashboardCards;
extern Mycila::Task initEventsTask;
extern Mycila::Task initLoggingTask;
extern Mycila::Task initMqttSubscribersTask;
extern Mycila::Task initRestApiTask;
extern Mycila::Task initWebTask;
