// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaAppInfo.h>
#include <MycilaButton.h>
#include <MycilaBuzzer.h>
#include <MycilaConfig.h>
#include <MycilaDisplay.h>
#include <MycilaGrid.h>
#include <MycilaHTTPd.h>
#include <MycilaJSY.h>
#include <MycilaLights.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaNTP.h>
#include <MycilaOTA.h>
#include <MycilaRelayManager.h>
#include <MycilaRouter.h>
#include <MycilaSystem.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTemperatureSensor.h>
#include <MycilaZeroCrossDetection.h>

#include <YaSolR.h>
#include <YaSolRConfig.h>
#include <YaSolRConnector.h>
#include <YaSolRController.h>
#include <YaSolRWebsite.h>

#include <HardwareSerial.h>
#include <MycilaESPConnect.h>
#include <assert.h>

#ifdef APP_VERSION_PRO
#include <WebSerialLite.h>
#endif

#ifdef APP_VERSION_TRIAL
#include <MycilaTrial.h>
#endif

#ifndef YASOLR_SERIAL_BAUDRATE
#define YASOLR_SERIAL_BAUDRATE 115200
#endif

#define TAG "YASOLR"

Mycila::Relay relay1("relay1");
Mycila::Relay relay2("relay2");

Mycila::RouterOutput output1("output1");
Mycila::RouterOutput output2("output2");

Mycila::TemperatureSensor systemTemp(NAME_SYSTEM);

YaSolR::Connector connector(&systemTemp);
YaSolR::Website website(&systemTemp);
YaSolR::Controller appController(&systemTemp, &website, &connector);

void loop() {
#ifdef APP_VERSION_TRIAL
  Mycila::Trial.loop();
  yield();
#endif

  const uint32_t start = millis();

  Mycila::Button.loop();
  yield();

  Mycila::Display.loop();
  yield();

  systemTemp.loop();
  yield();

  Mycila::Router.loop();
  yield();

  Mycila::RelayManager.loop();
  yield();

  appController.loop();
  yield();

  ESPConnect.loop();
  yield();

  Mycila::OTA.loop();
  yield();

  Mycila::MQTT.loop();
  yield();

  if (Mycila::Logger.isDebugEnabled()) {
    Mycila::TaskMonitor.loop();
    yield();
  }

  website.loop();
  yield();

  const uint32_t elapsed = millis() - start;
  if (elapsed > 1000)
    Mycila::Logger.warn(TAG, "Abnormal loop duration: %u ms", elapsed);
}

void ioTask(void* params) {
  if (Mycila::Logger.isDebugEnabled())
    Mycila::Logger.debug(TAG, "Task '%s' started on core: %u", pcTaskGetName(NULL), xPortGetCoreID());
  YaSolR::Connector* connector = reinterpret_cast<YaSolR::Connector*>(params);
  while (true) {
    connector->loop();
    delay(100);
  }
}

void setup() {
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

  // loading config
  YaSolR::Config.begin();

  // init logging
  Mycila::Logger.info(TAG, "Starting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());

// start components
#ifdef APP_VERSION_PRO
  WebSerial.begin(&Mycila::HTTPd.server, "/console", Mycila::HTTPdConfig.getUsername(), Mycila::HTTPdConfig.getPassword());
  Mycila::Logger.forwardTo(&WebSerial);
#endif

  appController.begin();
  connector.begin();

  Mycila::Button.begin();
  Mycila::Lights.begin(Mycila::LightState::ON, Mycila::LightState::OFF, Mycila::LightState::ON);
  Mycila::Buzzer.begin();
  Mycila::Display.begin();

  // JSY
  if (Mycila::Config.getBool(KEY_JSY_ENABLE)) {
    Mycila::JSY.begin(Mycila::Config.get(KEY_JSY_RX_PIN).toInt(), Mycila::Config.get(KEY_JSY_TX_PIN).toInt(), &Serial2, true);
    if (Mycila::JSY.isEnabled() && Mycila::JSY.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400) {
      if (Mycila::JSY.updateBaudRate(Mycila::JSYBaudRate::BAUD_38400)) {
        Mycila::Logger.info(TAG, "JSY speed updated to 38400 bauds");
      } else {
        Mycila::Logger.warn(TAG, "Failed to update JSY speed to 38400 bauds");
      }
    }
  } else {
    Mycila::Logger.warn(TAG, "JSY not enabled");
  }

  Mycila::ZCD.begin();
  Mycila::Grid.begin();

  // router
  Mycila::Router.outputs.push_back(output1);
  Mycila::Router.outputs.push_back(output2);
  Mycila::Router.begin();

  // relay manager
  Mycila::RelayManager.relays.push_back(relay1);
  Mycila::RelayManager.relays.push_back(relay2);
  Mycila::RelayManager.begin();

  systemTemp.begin();
  website.begin();

  // ntp
  Mycila::NTP.setTimeZone(Mycila::Config.get(KEY_NTP_TIMEZONE));

  // network
  Mycila::Logger.info(TAG, "Enable Network...");
  Mycila::Logger.info(TAG, "Hostname: %s", Mycila::Config.get(KEY_HOSTNAME).c_str());

  ESPConnect.setAutoRestart(false);
  ESPConnect.setBlocking(false);
  ESPConnect.setCaptivePortalTimeout(Mycila::Config.get(KEY_CAPTURE_PORTAL_TIMEOUT).toInt());
  ESPConnect.setWiFiConnectTimeout(Mycila::Config.get(KEY_WIFI_CONNECTION_TIMEOUT).toInt());
  ESPConnect.begin(&Mycila::HTTPd.server, Mycila::Config.get(KEY_HOSTNAME), Mycila::AppInfo.name + "-" + Mycila::AppInfo.id, Mycila::Config.get(KEY_ADMIN_PASSWORD), {Mycila::Config.get(KEY_WIFI_SSID), Mycila::Config.get(KEY_WIFI_PASSWORD), Mycila::Config.getBool(KEY_AP_MODE_ENABLE)});

  // system monitoring
  Mycila::TaskMonitor.begin(5);
  Mycila::TaskMonitor.addTask("loopTask");
  Mycila::TaskMonitor.addTask("async_tcp");
  Mycila::TaskMonitor.addTask("mqttclient");
  Mycila::TaskMonitor.addTask("jsyTask");
  Mycila::TaskMonitor.addTask("ioTask");

  // STARTUP READY!
  Mycila::Buzzer.beep();
  Mycila::Logger.info(TAG, "Started %s %s %s", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());

  assert(xTaskCreateUniversal(ioTask, "ioTask", 256 * 19, &connector, 1, nullptr, xPortGetCoreID()) == pdPASS);
}
