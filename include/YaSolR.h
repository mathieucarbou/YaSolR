// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <YaSolRClass.h>
#include <YaSolRDeclarations.h>
#include <YaSolRMacros.h>

#include <MycilaAppInfo.h>
#include <MycilaButton.h>
#include <MycilaBuzzer.h>
#include <MycilaConfig.h>
#include <MycilaDimmer.h>
#include <MycilaESPConnect.h>
#include <MycilaEasyDisplay.h>
#include <MycilaGrid.h>
#include <MycilaHADiscovery.h>
#include <MycilaHTTPd.h>
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
#include <MycilaSystem.h>
#include <MycilaTaskManager.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTemperatureSensor.h>
#include <MycilaTime.h>
#include <MycilaZeroCrossDetection.h>

#ifdef APP_VERSION_TRIAL
#include <MycilaTrial.h>
#endif

#ifdef APP_VERSION_PRO
#include <WebSerialLite.h>
#endif
