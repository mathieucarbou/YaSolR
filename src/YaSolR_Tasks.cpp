// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

static const Mycila::TaskPredicate DEBUG_ENABLED = []() {
  return Mycila::Logger.isDebugEnabled();
};
static const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  Mycila::Logger.debug(TAG, "%s in %d ms", me.getName(), elapsed / Mycila::TimeUnit::MILLISECONDS);
};

// Core tasks

Mycila::Task buttonTask("Button.processEvents()", [](void* params) { Mycila::Button.processEvents(); });
Mycila::Task displayTask("YaSolR.updateDisplay()", [](void* params) { YaSolR::YaSolR.updateDisplay(); });
Mycila::Task espConnectTask("ESPConnect.loop()", [](void* params) { ESPConnect.loop(); });
Mycila::Task lightsTask("Lights", [](void* params) { Mycila::Lights.setYellow(relay1.isOn() || relay2.isOn() || output1BypassRelay.isOn() || output2BypassRelay.isOn() || output1Dimmer.isOn() || output2Dimmer.isOn()); });
Mycila::Task stackMonitorTask("TaskMonitor.log()", [](void* params) { Mycila::TaskMonitor.log(); });
Mycila::Task systemTemperatureTask("systemTemperatureSensor.read()", [](void* params) { systemTemperatureSensor.read(); });
Mycila::Task websiteTask("YaSolR.updateWebsite()", [](void* params) { YaSolR::YaSolR.updateWebsite(); });

#ifdef APP_VERSION_TRIAL
Mycila::Task trialTask("Trial.validate()", [](void* params) { Mycila::Trial.validate(); });
#endif

// Network tasks not on OTA and and only when mqtt is enabled / connected

Mycila::Task publisherTask("YaSolR.publishMQTT()", [](void* params) { YaSolR::YaSolR.publishMQTT(); });

// Router tasks

Mycila::Task autoRelayTask("RelayManager.autoCommute()", [](void* params) { Mycila::RelayManager.autoCommute(); });
Mycila::Task output1AutoBypassTask("output1.autoBypass()", [](void* params) { output1.autoBypass(); });
Mycila::Task output1DimmerLimitTask("output1.applyDimmerLimit()", [](void* params) { output1.applyDimmerLimit(); });
Mycila::Task output1TemperatureTask("output1TemperatureSensor.read()", [](void* params) { output1TemperatureSensor.read(); });
Mycila::Task output2AutoBypassTask("output2.autoBypass()", [](void* params) { output2.autoBypass(); });
Mycila::Task output2DimmerLimitTask("output2.applyDimmerLimit()", [](void* params) { output2.applyDimmerLimit(); });
Mycila::Task output2TemperatureTask("output2TemperatureSensor.read()", [](void* params) { output2TemperatureSensor.read(); });

// On Demand tasks

Mycila::Task haDiscoTask("YaSolR.publishHADiscovery()", [](void* params) { YaSolR::YaSolR.publishHADiscovery(); });

Mycila::Task restartTask("restartTask", [](void* params) {
  Mycila::Logger.warn(TAG, "Restarting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
  Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::OFF, Mycila::LightState::ON);
  Mycila::Buzzer.beep(2);
  Mycila::Buzzer.end();
  Mycila::System.restart(500);
});

Mycila::Task resetTask("resetTask", [](void* params) {
  Mycila::Logger.warn(TAG, "Resetting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
  Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::ON, Mycila::LightState::ON);
  Mycila::Buzzer.beep(3);
  Mycila::Buzzer.end();
  Mycila::Config.clear();
  Mycila::System.restart(500);
});

Mycila::Task startNetworkServicesTask("startNetworkServicesTask", [](void* params) {
  Mycila::Logger.info(TAG, "Starting network services...");
  Mycila::HTTPd.begin();
  if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE) && Mycila::Config.getBool(KEY_MQTT_ENABLE))
    Mycila::MQTT.begin(YaSolR::YaSolR.getMQTTConfig());
  else
    Mycila::Logger.warn(TAG, "MQTT not enabled");
  if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE))
    Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
  Mycila::Buzzer.beep();
});

Mycila::Task stopNetworkServicesTask("stopNetworkServicesTask", [](void* params) {
  Mycila::Logger.info(TAG, "Stopping network services...");
  Mycila::MQTT.end();
  Mycila::HTTPd.end();
  Mycila::Buzzer.beep(2);
});

Mycila::Task otaPrepareTask("otaPrepareTask", [](void* params) {
  Mycila::Logger.info(TAG, "Preparing OTA update...");
  lightsTask.pause();
  publisherTask.pause();
  output1Dimmer.end();
  output2Dimmer.end();
  output1BypassRelay.end();
  output2BypassRelay.end();
  relay1.end();
  relay2.end();
  Mycila::Lights.setAllOn();
  Mycila::JSY.end();
  Mycila::ZCD.end();
  Mycila::MQTT.end();
#ifdef APP_VERSION_TRIAL
  trialTask.pause();
  Mycila::Trial.end();
#endif
});

// reconfigure tasks

Mycila::Task configureButtonTask("configureButtonTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Push Button...");
  Mycila::Button.end();
  if (Mycila::Config.getBool(KEY_BUTTON_ENABLE)) {
    Mycila::Button.begin(Mycila::Config.get(KEY_BUTTON_PIN).toInt(), Mycila::Config.get(KEY_BUTTON_ACTION), "reset");
  } else {
    Mycila::Logger.warn(TAG, "Button not enabled");
  }
});

Mycila::Task configureBuzzerTask("configureBuzzerTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Buzzer...");
  Mycila::Buzzer.end();
  if (Mycila::Config.getBool(KEY_BUZZER_ENABLE)) {
    Mycila::Buzzer.begin(Mycila::Config.get(KEY_BUZZER_PIN).toInt());
  } else {
    Mycila::Logger.warn(TAG, "Buzzer not enabled");
  }
});

Mycila::Task configureDisplayTask("configureDisplayTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Display...");
  Mycila::EasyDisplay.end();
  if (Mycila::Config.getBool(KEY_DISPLAY_ENABLE)) {
    Mycila::EasyDisplay.setPowerSaveDelay(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt());
    Mycila::EasyDisplay.begin(YaSolR::YaSolR.getDisplayType(), Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN).toInt(), Mycila::Config.get(KEY_DISPLAY_DATA_PIN).toInt(), YaSolR::YaSolR.getDisplayRotation());
  } else {
    Mycila::Logger.warn(TAG, "Display not enabled");
  }
});

Mycila::Task configureGridTask("configureGridTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Grid Power...");
  Mycila::Grid.end();
  Mycila::Grid.begin(Mycila::Config.get(KEY_GRID_POWER_MQTT_TOPIC));
});

Mycila::Task configureJSYTask("configureJSYTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure JSY...");
  Mycila::JSY.end();
  if (Mycila::Config.getBool(KEY_JSY_ENABLE)) {
    Mycila::JSY.begin(Mycila::Config.get(KEY_JSY_RX_PIN).toInt(), Mycila::Config.get(KEY_JSY_TX_PIN).toInt(), &MYCILA_JSY_SERIAL, true);
    if (Mycila::JSY.isEnabled()) {
      if (Mycila::JSY.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400) {
        Mycila::Logger.info(TAG, "Trying to update JSY Baud Rate to 38400...");
        if (!Mycila::JSY.updateBaudRate(Mycila::JSYBaudRate::BAUD_38400)) {
          Mycila::Logger.warn(TAG, "Failed to update JSY Baud Rate to 38400");
        }
      }
    } else {
      Mycila::Logger.warn(TAG, "JSY failed to start");
    }
  } else {
    Mycila::Logger.warn(TAG, "JSY not enabled");
  }
});

Mycila::Task configureLightsTask("configureLightsTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Lights...");
  Mycila::LightState g = Mycila::Lights.getGreen();
  Mycila::LightState y = Mycila::Lights.getYellow();
  Mycila::LightState r = Mycila::Lights.getRed();
  Mycila::Lights.end();
  if (Mycila::Config.getBool(KEY_LIGHTS_ENABLE)) {
    Mycila::Lights.begin(Mycila::Config.get(KEY_LIGHTS_GREEN_PIN).toInt(), Mycila::Config.get(KEY_LIGHTS_YELLOW_PIN).toInt(), Mycila::Config.get(KEY_LIGHTS_RED_PIN).toInt());
  } else {
    Mycila::Logger.warn(TAG, "Physical LEDs not enabled");
  }
  Mycila::Lights.set(g, y, r);
});

Mycila::Task configureMQTTTask("configureMQTTTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configuring MQTT...");
  Mycila::MQTT.end();
  if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE) && Mycila::Config.getBool(KEY_MQTT_ENABLE))
    Mycila::MQTT.begin(YaSolR::YaSolR.getMQTTConfig());
  else
    Mycila::Logger.warn(TAG, "MQTT not enabled");
});

Mycila::Task configureNetworkTask("configureNetworkTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Network...");
  Mycila::MQTT.end();
  Mycila::HTTPd.end();
  ESPConnect.end();
  Mycila::Logger.info(TAG, "Hostname: %s", Mycila::Config.get(KEY_HOSTNAME).c_str());
  ESPConnect.setAutoRestart(false);
  ESPConnect.setBlocking(false);
  ESPConnect.setCaptivePortalTimeout(Mycila::Config.get(KEY_CAPTURE_PORTAL_TIMEOUT).toInt());
  ESPConnect.setConnectTimeout(Mycila::Config.get(KEY_WIFI_CONNECTION_TIMEOUT).toInt());
  ESPConnect.begin(&webServer, Mycila::Config.get(KEY_HOSTNAME), Mycila::AppInfo.name + "-" + Mycila::AppInfo.id, Mycila::Config.get(KEY_ADMIN_PASSWORD), {Mycila::Config.get(KEY_WIFI_SSID), Mycila::Config.get(KEY_WIFI_PASSWORD), Mycila::Config.getBool(KEY_AP_MODE_ENABLE)});
});

Mycila::Task configureNTPTask("configureNTPTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configuring NTP...");
  Mycila::NTP.setTimeZone(Mycila::Config.get(KEY_NTP_TIMEZONE));
  if (ESPConnect.isConnected() && !Mycila::Config.getBool(KEY_AP_MODE_ENABLE)) {
    Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
  }
});

Mycila::Task configureOutput1BypassRelayTask("configureOutput1BypassRelayTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 1 Bypass Relay...");
  output1BypassRelay.end();
  if (Mycila::Config.getBool(KEY_OUTPUT1_RELAY_ENABLE)) {
    output1BypassRelay.begin(Mycila::Config.get(KEY_OUTPUT1_RELAY_PIN).toInt(), Mycila::Config.get(KEY_OUTPUT1_RELAY_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Output 1 Bypass Relay not enabled");
  }
});

Mycila::Task configureOutput1DimmerTask("configureOutput1DimmerTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 1 Dimmer...");
  output1Dimmer.end();
  if (Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_ENABLE)) {
    output1Dimmer.begin(Mycila::Config.get(KEY_OUTPUT1_DIMMER_PIN).toInt(), YaSolR::YaSolR.getDimmerType(output1Dimmer.getName()));
  } else {
    Mycila::Logger.warn(TAG, "Output 1 Dimmer not enabled");
  }
});

Mycila::Task configureOutput1TemperatureSensorTask("configureOutput1TemperatureSensorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 1 Temperature Sensor...");
  output1TemperatureSensor.end();
  if (Mycila::Config.getBool(KEY_OUTPUT1_TEMP_ENABLE)) {
    output1TemperatureSensor.begin(Mycila::Config.get(KEY_OUTPUT1_TEMP_PIN).toInt(), 6 * YASOLR_TEMPERATURE_READ_INTERVAL);
  } else {
    Mycila::Logger.warn(TAG, "Output 1 Temperature Sensor not enabled");
  }
});

Mycila::Task configureOutput2BypassRelayTask("configureOutput2BypassRelayTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 2 Bypass Relay...");
  output2BypassRelay.end();
  if (Mycila::Config.getBool(KEY_OUTPUT2_RELAY_ENABLE)) {
    output2BypassRelay.begin(Mycila::Config.get(KEY_OUTPUT2_RELAY_PIN).toInt(), Mycila::Config.get(KEY_OUTPUT2_RELAY_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Output 2 Bypass Relay not enabled");
  }
});

Mycila::Task configureOutput2DimmerTask("configureOutput2DimmerTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 2 Dimmer...");
  output2Dimmer.end();
  if (Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_ENABLE)) {
    output2Dimmer.begin(Mycila::Config.get(KEY_OUTPUT2_DIMMER_PIN).toInt(), YaSolR::YaSolR.getDimmerType(output2Dimmer.getName()));
  } else {
    Mycila::Logger.warn(TAG, "Output 2 Dimmer not enabled");
  }
});

Mycila::Task configureOutput2TemperatureSensorTask("configureOutput2TemperatureSensorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 2 Temperature Sensor...");
  output2TemperatureSensor.end();
  if (Mycila::Config.getBool(KEY_OUTPUT2_TEMP_ENABLE)) {
    output2TemperatureSensor.begin(Mycila::Config.get(KEY_OUTPUT2_TEMP_PIN).toInt(), 6 * YASOLR_TEMPERATURE_READ_INTERVAL);
  } else {
    Mycila::Logger.warn(TAG, "Output 2 Temperature Sensor not enabled");
  }
});

Mycila::Task configureRelay1Task("configureRelay1Task", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Relay 1...");
  relay1.end();
  if (Mycila::Config.getBool(KEY_RELAY1_ENABLE)) {
    relay1.begin(Mycila::Config.get(KEY_RELAY1_PIN).toInt(), Mycila::Config.get(KEY_RELAY1_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Relay 1 not enabled");
  }
});

Mycila::Task configureRelay2Task("configureRelay2Task", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Relay 2...");
  relay2.end();
  if (Mycila::Config.getBool(KEY_RELAY2_ENABLE)) {
    relay2.begin(Mycila::Config.get(KEY_RELAY2_PIN).toInt(), Mycila::Config.get(KEY_RELAY2_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Relay 2 not enabled");
  }
});

Mycila::Task configureSystemTemperatureSensorTask("configureSystemTemperatureSensorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure System Temperature Sensor...");
  systemTemperatureSensor.end();
  if (Mycila::Config.getBool(KEY_SYSTEM_TEMP_ENABLE)) {
    systemTemperatureSensor.begin(Mycila::Config.get(KEY_SYSTEM_TEMP_PIN).toInt(), 6 * YASOLR_TEMPERATURE_READ_INTERVAL);
  } else {
    Mycila::Logger.warn(TAG, "System Temperature Sensor not enabled");
  }
});

Mycila::Task configureTaskMonitorTask("configureTaskMonitorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Task Stack Monitor...");
  Mycila::TaskMonitor.begin(5);
  Mycila::TaskMonitor.addTask("loopTask");
  Mycila::TaskMonitor.addTask("async_tcp");
  Mycila::TaskMonitor.addTask(MYCILA_MQTT_TASK_NAME);
  Mycila::TaskMonitor.addTask("jsyTask");
  // Mycila::TaskMonitor.addTask("ioTask");
});

Mycila::Task configureZCDTask("configureZCDTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Zero-Cross Detection...");
  Mycila::ZCD.end();
  if (Mycila::Config.getBool(KEY_ZCD_ENABLE)) {
    Mycila::ZCD.begin(Mycila::Config.get(KEY_ZCD_PIN).toInt(), Mycila::Config.get(KEY_GRID_FREQ).toInt() == 60 ? 60 : 50);
  } else {
    Mycila::Logger.warn(TAG, "Zero-Cross Detection not enabled");
  }
});

void YaSolR::YaSolRClass::_initTasks() {
  // TODO: profiler with json output, status json output of task status (enable / disabled / last time / last elapsed, etc.) (like coroutine lib: https://github.com/bxparks/AceRoutine/)
  // TODO: move TaskManager to oss

  buttonTask.setType(Mycila::TaskType::FOREVER);
  buttonTask.setManager(&loopTaskManager);
  buttonTask.setEnabled(true);

  displayTask.setType(Mycila::TaskType::FOREVER);
  displayTask.setManager(&loopTaskManager);
  displayTask.setEnabled(true);
  displayTask.setInterval(500 * Mycila::TimeUnit::MILLISECONDS);

  espConnectTask.setType(Mycila::TaskType::FOREVER);
  espConnectTask.setManager(&loopTaskManager);
  espConnectTask.setEnabled(true);

  websiteTask.setType(Mycila::TaskType::FOREVER);
  websiteTask.setManager(&loopTaskManager);
  websiteTask.setEnabledWhen([]() { return Mycila::HTTPd.isRunning() && !dashboard.isAsyncAccessInProgress(); });
  websiteTask.setInterval(1 * Mycila::TimeUnit::SECONDS);
  websiteTask.setCallback(LOG_EXEC_TIME);

  stackMonitorTask.setType(Mycila::TaskType::FOREVER);
  stackMonitorTask.setManager(&loopTaskManager);
  stackMonitorTask.setEnabledWhen(DEBUG_ENABLED);
  stackMonitorTask.setInterval(10 * Mycila::TimeUnit::SECONDS);

  systemTemperatureTask.setType(Mycila::TaskType::FOREVER);
  systemTemperatureTask.setManager(&loopTaskManager);
  systemTemperatureTask.setEnabledWhen([]() { return systemTemperatureSensor.isEnabled(); });
  systemTemperatureTask.setInterval(YASOLR_TEMPERATURE_READ_INTERVAL * Mycila::TimeUnit::SECONDS);

  lightsTask.setType(Mycila::TaskType::FOREVER);
  lightsTask.setManager(&loopTaskManager);
  lightsTask.setEnabled(true);
  lightsTask.setInterval(500 * Mycila::TimeUnit::MILLISECONDS);

#ifdef APP_VERSION_TRIAL
  trialTask.setType(Mycila::TaskType::FOREVER);
  trialTask.setManager(&loopTaskManager);
  trialTask.setEnabled(true);
  trialTask.setInterval(20 * Mycila::TimeUnit::SECONDS);
#endif

  publisherTask.setType(Mycila::TaskType::FOREVER);
  publisherTask.setManager(&loopTaskManager);
  publisherTask.setEnabledWhen([]() { return Mycila::MQTT.isConnected(); });
  publisherTask.setIntervalSupplier([]() { return Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt() * Mycila::TimeUnit::SECONDS; });
  publisherTask.setCallback(LOG_EXEC_TIME);

  autoRelayTask.setType(Mycila::TaskType::FOREVER);
  autoRelayTask.setManager(&loopTaskManager);
  autoRelayTask.setEnabledWhen([]() { return relay1.isEnabled() || relay2.isEnabled(); });
  autoRelayTask.setInterval(YASOLR_RELAY_PAUSE_DURATION * Mycila::TimeUnit::SECONDS);

  output1AutoBypassTask.setType(Mycila::TaskType::FOREVER);
  output1AutoBypassTask.setManager(&loopTaskManager);
  output1AutoBypassTask.setEnabled(true);
  output1AutoBypassTask.setInterval(500 * Mycila::TimeUnit::MILLISECONDS);

  output1DimmerLimitTask.setType(Mycila::TaskType::FOREVER);
  output1DimmerLimitTask.setManager(&loopTaskManager);
  output1DimmerLimitTask.setEnabledWhen([]() { return output1Dimmer.isEnabled(); });
  output1DimmerLimitTask.setInterval(500 * Mycila::TimeUnit::MILLISECONDS);

  output1TemperatureTask.setType(Mycila::TaskType::FOREVER);
  output1TemperatureTask.setManager(&loopTaskManager);
  output1TemperatureTask.setEnabledWhen([]() { return output1TemperatureSensor.isEnabled(); });
  output1TemperatureTask.setInterval(YASOLR_TEMPERATURE_READ_INTERVAL * Mycila::TimeUnit::SECONDS);

  output2AutoBypassTask.setType(Mycila::TaskType::FOREVER);
  output2AutoBypassTask.setManager(&loopTaskManager);
  output2AutoBypassTask.setEnabled(true);
  output2AutoBypassTask.setInterval(500 * Mycila::TimeUnit::MILLISECONDS);

  output2DimmerLimitTask.setType(Mycila::TaskType::FOREVER);
  output2DimmerLimitTask.setManager(&loopTaskManager);
  output2DimmerLimitTask.setEnabledWhen([]() { return output2Dimmer.isEnabled(); });
  output2DimmerLimitTask.setInterval(500 * Mycila::TimeUnit::MILLISECONDS);

  output2TemperatureTask.setType(Mycila::TaskType::FOREVER);
  output2TemperatureTask.setManager(&loopTaskManager);
  output2TemperatureTask.setEnabledWhen([]() { return output2TemperatureSensor.isEnabled(); });
  output2TemperatureTask.setInterval(YASOLR_TEMPERATURE_READ_INTERVAL * Mycila::TimeUnit::SECONDS);

  haDiscoTask.setType(Mycila::TaskType::ONCE);
  haDiscoTask.setManager(&loopTaskManager);
  haDiscoTask.setCallback(LOG_EXEC_TIME);

  restartTask.setType(Mycila::TaskType::ONCE);
  restartTask.setManager(&loopTaskManager);

  resetTask.setType(Mycila::TaskType::ONCE);
  resetTask.setManager(&loopTaskManager);

  startNetworkServicesTask.setType(Mycila::TaskType::ONCE);
  startNetworkServicesTask.setManager(&loopTaskManager);

  stopNetworkServicesTask.setType(Mycila::TaskType::ONCE);
  stopNetworkServicesTask.setManager(&loopTaskManager);

  otaPrepareTask.setType(Mycila::TaskType::ONCE);
  otaPrepareTask.setManager(&loopTaskManager);

  configureButtonTask.setType(Mycila::TaskType::ONCE);
  configureButtonTask.setManager(&loopTaskManager);

  configureBuzzerTask.setType(Mycila::TaskType::ONCE);
  configureBuzzerTask.setManager(&loopTaskManager);

  configureDisplayTask.setType(Mycila::TaskType::ONCE);
  configureDisplayTask.setManager(&loopTaskManager);

  configureGridTask.setType(Mycila::TaskType::ONCE);
  configureGridTask.setManager(&loopTaskManager);

  configureJSYTask.setType(Mycila::TaskType::ONCE);
  configureJSYTask.setManager(&loopTaskManager);

  configureLightsTask.setType(Mycila::TaskType::ONCE);
  configureLightsTask.setManager(&loopTaskManager);

  configureMQTTTask.setType(Mycila::TaskType::ONCE);
  configureMQTTTask.setManager(&loopTaskManager);

  configureNetworkTask.setType(Mycila::TaskType::ONCE);
  configureNetworkTask.setManager(&loopTaskManager);

  configureNTPTask.setType(Mycila::TaskType::ONCE);
  configureNTPTask.setManager(&loopTaskManager);

  configureOutput1BypassRelayTask.setType(Mycila::TaskType::ONCE);
  configureOutput1BypassRelayTask.setManager(&loopTaskManager);

  configureOutput1DimmerTask.setType(Mycila::TaskType::ONCE);
  configureOutput1DimmerTask.setManager(&loopTaskManager);

  configureOutput1TemperatureSensorTask.setType(Mycila::TaskType::ONCE);
  configureOutput1TemperatureSensorTask.setManager(&loopTaskManager);

  configureOutput2BypassRelayTask.setType(Mycila::TaskType::ONCE);
  configureOutput2BypassRelayTask.setManager(&loopTaskManager);

  configureOutput2DimmerTask.setType(Mycila::TaskType::ONCE);
  configureOutput2DimmerTask.setManager(&loopTaskManager);

  configureOutput2TemperatureSensorTask.setType(Mycila::TaskType::ONCE);
  configureOutput2TemperatureSensorTask.setManager(&loopTaskManager);

  configureRelay1Task.setType(Mycila::TaskType::ONCE);
  configureRelay1Task.setManager(&loopTaskManager);

  configureRelay2Task.setType(Mycila::TaskType::ONCE);
  configureRelay2Task.setManager(&loopTaskManager);

  configureSystemTemperatureSensorTask.setType(Mycila::TaskType::ONCE);
  configureSystemTemperatureSensorTask.setManager(&loopTaskManager);

  configureTaskMonitorTask.setType(Mycila::TaskType::ONCE);
  configureTaskMonitorTask.setManager(&loopTaskManager);

  configureZCDTask.setType(Mycila::TaskType::ONCE);
  configureZCDTask.setManager(&loopTaskManager);

#ifdef MYCILA_TASK_MANAGER_DEBUG
  configureButtonTask.setDebugWhen(DEBUG_ENABLED);
  configureBuzzerTask.setDebugWhen(DEBUG_ENABLED);
  configureDisplayTask.setDebugWhen(DEBUG_ENABLED);
  configureGridTask.setDebugWhen(DEBUG_ENABLED);
  configureJSYTask.setDebugWhen(DEBUG_ENABLED);
  configureLightsTask.setDebugWhen(DEBUG_ENABLED);
  configureMQTTTask.setDebugWhen(DEBUG_ENABLED);
  configureNetworkTask.setDebugWhen(DEBUG_ENABLED);
  configureNTPTask.setDebugWhen(DEBUG_ENABLED);
  configureOutput1BypassRelayTask.setDebugWhen(DEBUG_ENABLED);
  configureOutput1DimmerTask.setDebugWhen(DEBUG_ENABLED);
  configureOutput1TemperatureSensorTask.setDebugWhen(DEBUG_ENABLED);
  configureOutput2BypassRelayTask.setDebugWhen(DEBUG_ENABLED);
  configureOutput2DimmerTask.setDebugWhen(DEBUG_ENABLED);
  configureOutput2TemperatureSensorTask.setDebugWhen(DEBUG_ENABLED);
  configureRelay1Task.setDebugWhen(DEBUG_ENABLED);
  configureRelay2Task.setDebugWhen(DEBUG_ENABLED);
  configureSystemTemperatureSensorTask.setDebugWhen(DEBUG_ENABLED);
  configureTaskMonitorTask.setDebugWhen(DEBUG_ENABLED);
  configureZCDTask.setDebugWhen(DEBUG_ENABLED);
  otaPrepareTask.setDebugWhen(DEBUG_ENABLED);
  output1TemperatureTask.setDebugWhen(DEBUG_ENABLED);
  output2TemperatureTask.setDebugWhen(DEBUG_ENABLED);
  resetTask.setDebugWhen(DEBUG_ENABLED);
  restartTask.setDebugWhen(DEBUG_ENABLED);
  stackMonitorTask.setDebugWhen(DEBUG_ENABLED);
  startNetworkServicesTask.setDebugWhen(DEBUG_ENABLED);
  stopNetworkServicesTask.setDebugWhen(DEBUG_ENABLED);
  systemTemperatureTask.setDebugWhen(DEBUG_ENABLED);
#ifdef APP_VERSION_TRIAL
  trialTask.setDebugWhen(DEBUG_ENABLED);
#endif
#endif
}
