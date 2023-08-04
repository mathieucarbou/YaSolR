// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRouterOutput.h>
#include <MycilaTaskManager.h>
#include <MycilaTemperatureSensor.h>

#include <ESPAsyncWebServer.h>

#ifdef APP_VERSION_PRO
#include <ESPDashPro.h>
#else
#include <ESPDash.h>
#endif

extern AsyncWebServer webServer;
extern ESPDash dashboard;

extern Mycila::Relay relay1;
extern Mycila::Relay relay2;
extern Mycila::Relay output1BypassRelay;
extern Mycila::Relay output2BypassRelay;

extern Mycila::Dimmer output1Dimmer;
extern Mycila::Dimmer output2Dimmer;

extern Mycila::RouterOutput output1;
extern Mycila::RouterOutput output2;

extern Mycila::TemperatureSensor output1TemperatureSensor;
extern Mycila::TemperatureSensor output2TemperatureSensor;
extern Mycila::TemperatureSensor systemTemperatureSensor;

extern Mycila::Task autoRelayTask;
extern Mycila::Task buttonTask;
extern Mycila::Task configureButtonTask;
extern Mycila::Task configureBuzzerTask;
extern Mycila::Task configureDisplayTask;
extern Mycila::Task configureGridTask;
extern Mycila::Task configureJSYTask;
extern Mycila::Task configureLightsTask;
extern Mycila::Task configureMQTTTask;
extern Mycila::Task configureNetworkTask;
extern Mycila::Task configureNTPTask;
extern Mycila::Task configureOutput1BypassRelayTask;
extern Mycila::Task configureOutput1DimmerTask;
extern Mycila::Task configureOutput1TemperatureSensorTask;
extern Mycila::Task configureOutput2BypassRelayTask;
extern Mycila::Task configureOutput2DimmerTask;
extern Mycila::Task configureOutput2TemperatureSensorTask;
extern Mycila::Task configureRelay1Task;
extern Mycila::Task configureRelay2Task;
extern Mycila::Task configureSystemTemperatureSensorTask;
extern Mycila::Task configureTaskMonitorTask;
extern Mycila::Task configureZCDTask;
extern Mycila::Task displayTask;
extern Mycila::Task espConnectTask;
extern Mycila::Task haDiscoTask;
extern Mycila::Task lightsTask;
extern Mycila::Task mqttTask;
extern Mycila::Task otaPrepareTask;
extern Mycila::Task output1AutoBypassTask;
extern Mycila::Task output1DimmerLimitTask;
extern Mycila::Task output1TemperatureTask;
extern Mycila::Task output2AutoBypassTask;
extern Mycila::Task output2DimmerLimitTask;
extern Mycila::Task output2TemperatureTask;
extern Mycila::Task publisherTask;
extern Mycila::Task resetTask;
extern Mycila::Task restartTask;
extern Mycila::Task stackMonitorTask;
extern Mycila::Task startNetworkServicesTask;
extern Mycila::Task stopNetworkServicesTask;
extern Mycila::Task systemTemperatureTask;
extern Mycila::Task websiteTask;

#ifdef APP_VERSION_TRIAL
extern Mycila::Task trialTask;
#endif
