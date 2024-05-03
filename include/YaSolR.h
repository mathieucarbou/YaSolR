// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <HardwareSerial.h>

#include <MycilaAppInfo.h>
#include <MycilaButton.h>
#include <MycilaBuzzer.h>
#include <MycilaConfig.h>
#include <MycilaDimmer.h>
#include <MycilaEasyDisplay.h>
#include <MycilaESPConnect.h>
#include <MycilaGrid.h>
#include <MycilaHADiscovery.h>
#include <MycilaHTTPd.h>
#include <MycilaJSY.h>
#include <MycilaJSY.h>
#include <MycilaLights.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaNTP.h>
#include <MycilaPZEM004Tv3.h>
#include <MycilaRelay.h>
#include <MycilaRelayManager.h>
#include <MycilaRouter.h>
#include <MycilaRouterOutput.h>
#include <MycilaRouterOutput.h>
#include <MycilaSystem.h>
#include <MycilaTaskManager.h>
#include <MycilaTaskManager.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTemperatureSensor.h>
#include <MycilaTemperatureSensor.h>
#include <MycilaTime.h>
#include <MycilaZeroCrossDetection.h>

#include <YaSolRClass.h>
#include <YaSolRMacros.h>

#include <ESPAsyncWebServer.h>
#include <WebSerialLite.h>

#ifdef APP_MODEL_TRIAL
#include <MycilaTrial.h>
#endif

#ifdef APP_MODEL_PRO
#include <ESPDashPro.h>
#else
#include <ESPDash.h>
#endif

extern AsyncWebServer webServer;
extern ESPDash dashboard;

extern Mycila::JSY jsy;

extern Mycila::Dimmer output1Dimmer;
extern Mycila::Dimmer output2Dimmer;

extern Mycila::PZEM output1PZEM;
extern Mycila::PZEM output2PZEM;

extern Mycila::Relay relay1;
extern Mycila::Relay relay2;
extern Mycila::Relay output1BypassRelay;
extern Mycila::Relay output2BypassRelay;

extern Mycila::RouterOutput output1;
extern Mycila::RouterOutput output2;

extern Mycila::TemperatureSensor output1TemperatureSensor;
extern Mycila::TemperatureSensor output2TemperatureSensor;
extern Mycila::TemperatureSensor systemTemperatureSensor;

extern Mycila::TaskManager loopTaskManager;
// extern Mycila::TaskManager meterTaskManager;
extern Mycila::TaskManager routerTaskManager;

extern Mycila::Task assignOutput1PZEMTask;
extern Mycila::Task assignOutput2PZEMTask;
extern Mycila::Task autoRelayTask;
extern Mycila::Task displayTask;
extern Mycila::Task espConnectTask;
extern Mycila::Task haDiscoTask;
extern Mycila::Task jsyTask;
extern Mycila::Task lightsTask;
extern Mycila::Task otaPrepareTask;
extern Mycila::Task output1AutoBypassTask;
extern Mycila::Task output1DimmerLimitTask;
extern Mycila::Task output1UpdateStats;
extern Mycila::Task output1PZEMTask;
extern Mycila::Task output1TemperatureTask;
extern Mycila::Task output2AutoBypassTask;
extern Mycila::Task output2DimmerLimitTask;
extern Mycila::Task output2UpdateStats;
extern Mycila::Task output2PZEMTask;
extern Mycila::Task output2TemperatureTask;
extern Mycila::Task profileTask;
extern Mycila::Task publisherTask;
extern Mycila::Task resetTask;
extern Mycila::Task restartTask;
extern Mycila::Task stackMonitorTask;
extern Mycila::Task startNetworkServicesTask;
extern Mycila::Task stopNetworkServicesTask;
extern Mycila::Task systemTemperatureTask;
extern Mycila::Task websiteTask;

extern Mycila::Task configureButtonTask;
extern Mycila::Task configureBuzzerTask;
extern Mycila::Task configureDebugTask;
extern Mycila::Task configureDisplayTask;
extern Mycila::Task configureGridTask;
extern Mycila::Task configureJSYTask;
extern Mycila::Task configureLightsTask;
extern Mycila::Task configureMQTTTask;
extern Mycila::Task configureNetworkTask;
extern Mycila::Task configureNTPTask;
extern Mycila::Task configureOutput1BypassRelayTask;
extern Mycila::Task configureOutput1DimmerTask;
extern Mycila::Task configureOutput1PZEMTask;
extern Mycila::Task configureOutput1TemperatureSensorTask;
extern Mycila::Task configureOutput2BypassRelayTask;
extern Mycila::Task configureOutput2DimmerTask;
extern Mycila::Task configureOutput2PZEMTask;
extern Mycila::Task configureOutput2TemperatureSensorTask;
extern Mycila::Task configureRelay1Task;
extern Mycila::Task configureRelay2Task;
extern Mycila::Task configureSystemTemperatureSensorTask;
extern Mycila::Task configureTaskMonitorTask;
extern Mycila::Task configureZCDTask;

#ifdef APP_MODEL_TRIAL
extern Mycila::Task trialTask;
#endif
