// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#ifdef YASOLR_DISABLE_BROWNOUT_DETECTOR
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#endif

#include <HardwareSerial.h>

#define TAG "YASOLR"

Mycila::TaskManager loopTaskManager("loop()", 44);

// TODO: switch to PsychicHttp ?
AsyncWebServer webServer(80);

ESPDash dashboard = ESPDash(&webServer, "/dashboard", false);

Mycila::JSY jsy;

Mycila::Dimmer output1Dimmer;
Mycila::Dimmer output2Dimmer;

Mycila::Relay relay1;
Mycila::Relay relay2;
Mycila::Relay output1BypassRelay;
Mycila::Relay output2BypassRelay;

Mycila::TemperatureSensor output1TemperatureSensor(NAME_OUTPUT1);
Mycila::TemperatureSensor output2TemperatureSensor(NAME_OUTPUT2);
Mycila::TemperatureSensor systemTemperatureSensor(NAME_SYSTEM);

Mycila::RouterOutput output1(NAME_OUTPUT1, &output1Dimmer, &output1TemperatureSensor, &output1BypassRelay);
Mycila::RouterOutput output2(NAME_OUTPUT2, &output2Dimmer, &output2TemperatureSensor, &output2BypassRelay);

// main loop (fast, no I/O)

void loop() {
  loopTaskManager.loop();
#ifdef APP_VERSION_TRIAL
  if (trialTask.tryRun())
    yield();
#endif
}

// setup
void setup() {
#ifdef YASOLR_DISABLE_BROWNOUT_DETECTOR
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif

  Serial.begin(YASOLR_SERIAL_BAUDRATE);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  // logger
  Mycila::Logger.getOutputs().reserve(2);
  Mycila::Logger.forwardTo(&Serial);
  Mycila::Logger.info(TAG, "Booting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());

  // system
  Mycila::System.begin();

  // trial
#ifdef APP_VERSION_TRIAL
  Mycila::Trial.begin();
#endif

  // link objects
  Mycila::Grid.setJSY(&jsy);
  Mycila::Router.setJSY(&jsy);
  Mycila::Router.addOutput({&output1});
  Mycila::Router.addOutput({&output2});

  // load config and initialize
  YaSolR::YaSolR.begin();

  // relay manager
  Mycila::RelayManager.addRelay(NAME_RELAY1, &relay1);
  Mycila::RelayManager.addRelay(NAME_RELAY2, &relay2);

  // start logging
  configureDebugTask.forceRun();
#ifdef APP_VERSION_PRO
  WebSerial.begin(&webServer, "/console", YASOLR_ADMIN_USERNAME, Mycila::Config.get(KEY_ADMIN_PASSWORD));
  Mycila::Logger.forwardTo(&WebSerial);
#endif

  Mycila::Logger.info(TAG, "Starting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());

  // order must be respected below

  configureLightsTask.forceRun();
  Mycila::Lights.set(Mycila::LightState::ON, Mycila::LightState::OFF, Mycila::LightState::ON);
  configureZCDTask.forceRun();
  configureOutput1DimmerTask.forceRun();
  configureOutput2DimmerTask.forceRun();
  configureJSYTask.forceRun();

  // any order below

  configureButtonTask.forceRun();
  configureBuzzerTask.forceRun();
  configureDisplayTask.forceRun();
  configureGridTask.forceRun();
  configureNTPTask.forceRun();
  configureOutput1BypassRelayTask.forceRun();
  configureOutput1TemperatureSensorTask.forceRun();
  configureOutput2BypassRelayTask.forceRun();
  configureOutput2TemperatureSensorTask.forceRun();
  configureRelay1Task.forceRun();
  configureRelay2Task.forceRun();
  configureSystemTemperatureSensorTask.forceRun();
  configureTaskMonitorTask.forceRun();

  configureNetworkTask.forceRun();

  // STARTUP READY!
  Mycila::Logger.info(TAG, "Started %s %s %s", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
}
