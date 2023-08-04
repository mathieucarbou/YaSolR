// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#include <HardwareSerial.h>

#define TAG "YASOLR"

Mycila::Relay relay1("relay1");
Mycila::Relay relay2("relay2");

Mycila::RouterOutput output1("output1");
Mycila::RouterOutput output2("output2");

Mycila::TemperatureSensor systemTemperatureSensor(NAME_SYSTEM);

YaSolR::Website website;

void loop() {
  // TODO: add task manager: remove yield, task.managed(&tm)
  // TODO: add profiler
  // TODO: json / Serial output of task manager status: task enable / disabled / last time / last elapsed, etc
  // TODO: json / Serial output of profiler
  if (buttonTask.tryRun())
    yield();
  if (displayTask.tryRun())
    yield();
  if (espConnectTask.tryRun())
    yield();
  if (lightsTask.tryRun())
    yield();
  if (stackMonitorTask.tryRun())
    yield();
  if (autoRelayTask.tryRun())
    yield();
  if (mqttTask.tryRun())
    yield();
  if (websiteTask.tryRun())
    yield();
  if (restartTask.tryRun())
    yield();
  if (resetTask.tryRun())
    yield();
  if (startNetworkServicesTask.tryRun())
    yield();
  if (stopNetworkServicesTask.tryRun())
    yield();
  if (otaPrepareTask.tryRun())
    yield();
  if (reconfigureTask.tryRun())
    yield();

#ifdef APP_VERSION_TRIAL
  if (trialTask.tryRun())
    yield();
#endif

  uint32_t start;
  uint32_t elapsed;

  // TODO: converter to task
  start = millis();
  systemTemperatureSensor.loop();
  elapsed = millis() - start;
  if (elapsed > 200)
    Mycila::Logger.warn(TAG, "Abnormal loop duration: %u ms (Temp)", elapsed);
  yield();

  // TODO: converter to task
  start = millis();
  Mycila::Router.loop();
  elapsed = millis() - start;
  if (elapsed > 200)
    Mycila::Logger.warn(TAG, "Abnormal loop duration: %u ms (Router)", elapsed);
  yield();
}

// I/O loop

void ioTask(void* params) {
  while (!restartTask.isEnabled() && !resetTask.isEnabled()) {
    if (otaTask.tryRun())
      yield();
    if (haDiscoTask.tryRun())
      yield();
    if (publisherTask.tryRun())
      yield();
  }
  vTaskDelete(NULL);
}

// setup

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

  // load config
  YaSolR::YaSolR.begin();

  Mycila::Logger.info(TAG, "Starting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());

  // start logging
#ifdef APP_VERSION_PRO
  WebSerial.begin(&Mycila::HTTPd.server, "/console", Mycila::HTTPdConfig.getUsername(), Mycila::HTTPdConfig.getPassword());
  Mycila::Logger.forwardTo(&WebSerial);
#endif

  Mycila::Button.begin();
  Mycila::Lights.begin(Mycila::LightState::ON, Mycila::LightState::OFF, Mycila::LightState::ON);
  Mycila::Buzzer.begin();

  // display
  if (Mycila::Config.getBool(KEY_DISPLAY_ENABLE)) {
    Mycila::EasyDisplay.setPowerSaveDelay(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt());
    Mycila::EasyDisplay.begin(YaSolR::YaSolR.getDisplayType(), Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN).toInt(), Mycila::Config.get(KEY_DISPLAY_DATA_PIN).toInt(), YaSolR::YaSolR.getDisplayRotation());
  } else {
    Mycila::Logger.warn(TAG, "Display not enabled");
  }

  // JSY
  if (Mycila::Config.getBool(KEY_JSY_ENABLE)) {
    Mycila::JSY.begin(Mycila::Config.get(KEY_JSY_RX_PIN).toInt(), Mycila::Config.get(KEY_JSY_TX_PIN).toInt(), &MYCILA_JSY_SERIAL, true);
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

  systemTemperatureSensor.begin();
  website.begin();

  // ntp
  Mycila::NTP.setTimeZone(Mycila::Config.get(KEY_NTP_TIMEZONE));

  // network
  Mycila::Logger.info(TAG, "Enable Network...");
  Mycila::Logger.info(TAG, "Hostname: %s", Mycila::Config.get(KEY_HOSTNAME).c_str());
  ESPConnect.setAutoRestart(false);
  ESPConnect.setBlocking(false);
  ESPConnect.setCaptivePortalTimeout(Mycila::Config.get(KEY_CAPTURE_PORTAL_TIMEOUT).toInt());
  ESPConnect.setConnectTimeout(Mycila::Config.get(KEY_WIFI_CONNECTION_TIMEOUT).toInt());
  ESPConnect.begin(&Mycila::HTTPd.server, Mycila::Config.get(KEY_HOSTNAME), Mycila::AppInfo.name + "-" + Mycila::AppInfo.id, Mycila::Config.get(KEY_ADMIN_PASSWORD), {Mycila::Config.get(KEY_WIFI_SSID), Mycila::Config.get(KEY_WIFI_PASSWORD), Mycila::Config.getBool(KEY_AP_MODE_ENABLE)});

  // system monitoring
  Mycila::TaskMonitor.begin(5);
  Mycila::TaskMonitor.addTask("loopTask");
  Mycila::TaskMonitor.addTask("async_tcp");
  Mycila::TaskMonitor.addTask("mqttclient");
  Mycila::TaskMonitor.addTask("jsyTask");
  Mycila::TaskMonitor.addTask("ioTask");
  // Mycila::TaskMonitor.addTask("otaTask");

  // STARTUP READY!
  Mycila::Buzzer.beep();
  Mycila::Logger.info(TAG, "Started %s %s %s", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());

  assert(xTaskCreateUniversal(ioTask, "ioTask", 256 * 18, nullptr, 1, nullptr, xPortGetCoreID()) == pdPASS);
}
