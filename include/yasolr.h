// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <AsyncJson.h>
#include <AsyncUDP.h>
#include <CRC.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FastCRC32.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <StreamString.h>

#include <MycilaAppInfo.h>
#include <MycilaCircularBuffer.h>
#include <MycilaConfig.h>
#include <MycilaDS18.h>
#include <MycilaDimmer.h>
#include <MycilaDimmerPWM.h>
#include <MycilaDimmerZeroCross.h>
#include <MycilaESPConnect.h>
#include <MycilaEasyDisplay.h>
#include <MycilaExpiringValue.h>
#include <MycilaGrid.h>
#include <MycilaHADiscovery.h>
#include <MycilaJSY.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaNTP.h>
#include <MycilaPID.h>
#include <MycilaPZEM004Tv3.h>
#include <MycilaPulseAnalyzer.h>
#include <MycilaRelay.h>
#include <MycilaRouter.h>
#include <MycilaRouterOutput.h>
#include <MycilaRouterRelay.h>
#include <MycilaString.h>
#include <MycilaSystem.h>
#include <MycilaTaskManager.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTime.h>
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

#include <yasolr_macros.h>

// web server
extern AsyncWebServer webServer;
extern ESPDash dashboard;
extern Mycila::Task dashboardInitTask;
extern Mycila::Task dashboardUpdateTask;
extern void yasolr_init_web_server();

// Config
extern Mycila::Config config;
extern void yasolr_init_config();

// Network
extern Mycila::ESPConnect espConnect;
extern void yasolr_init_network();

// grid electricity
extern Mycila::Grid grid;
extern void yasolr_init_grid();
extern float yasolr_frequency();

// logging
extern Mycila::Logger logger;
extern void yasolr_init_logging();

// JSY
extern Mycila::JSY* jsy;
extern Mycila::TaskManager* jsyTaskManager;
extern void yasolr_init_jsy();

// JSY Remote
extern AsyncUDP* udp;
extern Mycila::CircularBuffer<float, 15>* udpMessageRateBuffer;
extern Mycila::Task* jsyRemoteTask;
extern void yasolr_init_jsy_remote();

// DS18
extern Mycila::DS18* ds18O1;
extern Mycila::DS18* ds18O2;
extern Mycila::DS18* ds18Sys;
extern void yasolr_init_ds18();

// Display
extern Mycila::EasyDisplay* display;
extern void yasolr_init_display();

// MQTT
extern Mycila::MQTT* mqtt;
extern Mycila::Task* mqttConnectTask;
extern Mycila::Task* mqttPublishConfigTask;
extern Mycila::Task* mqttPublishTask;
extern void yasolr_init_mqtt();

// PZEM
extern Mycila::PZEM* pzemO1;
extern Mycila::PZEM* pzemO2;
extern Mycila::Task* pzemO1PairingTask;
extern Mycila::Task* pzemO2PairingTask;
extern Mycila::TaskManager* pzemTaskManager;
extern void yasolr_init_pzem();

// Lights
extern Mycila::TrafficLight lights;
extern void yasolr_init_lights();

// System
extern Mycila::Task resetTask;
extern Mycila::Task restartTask;
extern Mycila::Task safeBootTask;
extern Mycila::TaskManager coreTaskManager;
extern Mycila::TaskManager unsafeTaskManager;
extern void yasolr_boot();
extern void yasolr_init_system();

// Trial
extern void yasolr_init_trial();

// Relays
extern Mycila::RouterRelay* relay1;
extern Mycila::RouterRelay* relay2;
extern void yasolr_init_relays();

// router
extern Mycila::PID pidController;
extern Mycila::PulseAnalyzer* pulseAnalyzer;
extern Mycila::Router router;
extern Mycila::RouterOutput* output1;
extern Mycila::RouterOutput* output2;
extern void yasolr_divert();
extern void yasolr_init_router();
