// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRouterOutput.h>
#include <MycilaTaskManager.h>
#include <MycilaTemperatureSensor.h>

#include <YaSolRWebsite.h>

extern Mycila::Relay relay1;
extern Mycila::Relay relay2;

extern Mycila::RouterOutput output1;
extern Mycila::RouterOutput output2;

extern Mycila::TemperatureSensor systemTemperatureSensor;

extern Mycila::Task autoRelayTask;
extern Mycila::Task buttonTask;
extern Mycila::Task displayTask;
extern Mycila::Task espConnectTask;
extern Mycila::Task haDiscoTask;
extern Mycila::Task lightsTask;
extern Mycila::Task mqttTask;
extern Mycila::Task otaPrepareTask;
extern Mycila::Task otaTask;
extern Mycila::Task publisherTask;
extern Mycila::Task reconfigureTask;
extern Mycila::Task resetTask;
extern Mycila::Task restartTask;
extern Mycila::Task stackMonitorTask;
extern Mycila::Task startNetworkServicesTask;
extern Mycila::Task stopNetworkServicesTask;
extern Mycila::Task websiteTask;

#ifdef APP_VERSION_TRIAL
extern Mycila::Task trialTask;
#endif

extern YaSolR::Website website;
