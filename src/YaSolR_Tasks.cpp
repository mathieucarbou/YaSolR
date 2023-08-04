// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

const Mycila::TaskPredicate DEBUG_ENABLED = []() { return Mycila::Logger.isDebugEnabled(); };
const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me) { Mycila::Logger.debug(TAG, "%s in %d ms", me.getName(), me.getLastRuntime(Mycila::TimeUnit::MILLISECONDS)); };

// Core tasks

Mycila::Task buttonTask("Button.processEvents()", [](void* params) { Mycila::Button.processEvents(); }, Mycila::TaskType::FOREVER, 0, true);

Mycila::Task displayTask("YaSolR.updateDisplay()", [](void* params) { YaSolR::YaSolR.updateDisplay(); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS, true);

Mycila::Task espConnectTask("ESPConnect.loop()", [](void* params) { ESPConnect.loop(); }, Mycila::TaskType::FOREVER, 0, true);

Mycila::Task websiteTask = Mycila::Task("YaSolR.updateWebsite()", [](void* params) { YaSolR::YaSolR.updateWebsite(); }, Mycila::TaskType::FOREVER, 1 * Mycila::TimeUnit::SECONDS)
  .onDone(LOG_EXEC_TIME)
  .when([]() { return Mycila::HTTPd.isRunning() && !dashboard.isAsyncAccessInProgress(); });

Mycila::Task stackMonitorTask = Mycila::Task("TaskMonitor.log()", [](void* params) { Mycila::TaskMonitor.log(); }, Mycila::TaskType::FOREVER, 20 * Mycila::TimeUnit::SECONDS)
  .debugIf(DEBUG_ENABLED)
  .when(DEBUG_ENABLED);

Mycila::Task systemTemperatureTask = Mycila::Task("systemTemperatureSensor.read()", [](void* params) { systemTemperatureSensor.read(); }, Mycila::TaskType::FOREVER, YASOLR_TEMPERATURE_READ_INTERVAL * Mycila::TimeUnit::SECONDS)
  .debugIf(DEBUG_ENABLED)
  .when([]() { return systemTemperatureSensor.isEnabled(); });

// Core tasks not on OTA

Mycila::Task lightsTask = Mycila::Task("Lights", [](void* params) { Mycila::Lights.setYellow(relay1.isOn() || relay2.isOn() || output1BypassRelay.isOn() || output2BypassRelay.isOn() || output1Dimmer.isOn() || output2Dimmer.isOn()); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS, true);

#ifdef APP_VERSION_TRIAL
Mycila::Task trialTask = Mycila::Task("Trial.validate()", [](void* params) { Mycila::Trial.validate(); }, Mycila::TaskType::FOREVER, 20 * Mycila::TimeUnit::SECONDS, true)
  .debugIf(DEBUG_ENABLED);
#endif

// Network tasks not on OTA and and only when mqtt is enabled / connected

Mycila::Task mqttTask = Mycila::Task("MQTT.loop()", [](void* params) { Mycila::MQTT.loop(); })
  .when([]() { return Mycila::MQTT.isEnabled(); });

Mycila::Task publisherTask = Mycila::Task("YaSolR.publishMQTT()", [](void* params) { YaSolR::YaSolR.publishMQTT(); }, Mycila::TaskType::FOREVER)
  .onDone(LOG_EXEC_TIME)
  .interval([]() { return Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt() * Mycila::TimeUnit::SECONDS; })
  .when([]() { return Mycila::MQTT.isConnected(); });

// Router tasks

Mycila::Task autoRelayTask = Mycila::Task("RelayManager.autoCommute()", [](void* params) { Mycila::RelayManager.autoCommute(); }, Mycila::TaskType::FOREVER, YASOLR_RELAY_PAUSE_DURATION * Mycila::TimeUnit::SECONDS)
  .when([]() { return relay1.isEnabled() || relay2.isEnabled(); });

Mycila::Task output1AutoBypassTask = Mycila::Task("output1.autoBypass()", [](void* params) { output1.autoBypass(); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS, true);

Mycila::Task output1DimmerLimitTask = Mycila::Task("output1.applyDimmerLimit()", [](void* params) { output1.applyDimmerLimit(); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS)
  .when([]() { return output1Dimmer.isEnabled(); });

Mycila::Task output1TemperatureTask = Mycila::Task("output1TemperatureSensor.read()", [](void* params) { output1TemperatureSensor.read(); }, Mycila::TaskType::FOREVER, YASOLR_TEMPERATURE_READ_INTERVAL * Mycila::TimeUnit::SECONDS)
  .debugIf(DEBUG_ENABLED)
  .when([]() { return output1TemperatureSensor.isEnabled(); });

Mycila::Task output2AutoBypassTask = Mycila::Task("output2.autoBypass()", [](void* params) { output2.autoBypass(); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS, true);

Mycila::Task output2DimmerLimitTask = Mycila::Task("output2.applyDimmerLimit()", [](void* params) { output2.applyDimmerLimit(); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS)
  .when([]() { return output2Dimmer.isEnabled(); });

Mycila::Task output2TemperatureTask = Mycila::Task("output2TemperatureSensor.read()", [](void* params) { output2TemperatureSensor.read(); }, Mycila::TaskType::FOREVER, YASOLR_TEMPERATURE_READ_INTERVAL * Mycila::TimeUnit::SECONDS)
  .debugIf(DEBUG_ENABLED)
  .when([]() { return output2TemperatureSensor.isEnabled(); });

// On Demand tasks

Mycila::Task haDiscoTask = Mycila::Task("YaSolR.publishHADiscovery()", [](void* params) { YaSolR::YaSolR.publishHADiscovery(); }, Mycila::TaskType::ONCE)
  .onDone(LOG_EXEC_TIME);

Mycila::Task restartTask = Mycila::Task("restartTask", [](void* params) {
  Mycila::Logger.warn(TAG, "Restarting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
  Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::OFF, Mycila::LightState::ON);
  Mycila::Buzzer.beep(2);
  Mycila::Buzzer.end();
  Mycila::System.restart(500);
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task resetTask = Mycila::Task("resetTask", [](void* params) {
  Mycila::Logger.warn(TAG, "Resetting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
  Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::ON, Mycila::LightState::ON);
  Mycila::Buzzer.beep(3);
  Mycila::Buzzer.end();
  Mycila::Config.clear();
  Mycila::System.restart(500);
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task startNetworkServicesTask = Mycila::Task("startNetworkServicesTask", [](void* params) {
  Mycila::Logger.info(TAG, "Starting network services...");
  Mycila::HTTPd.begin();
  Mycila::MQTT.begin();
  if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE))
    Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
  Mycila::Buzzer.beep();
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task stopNetworkServicesTask = Mycila::Task("stopNetworkServicesTask", [](void* params) {
  Mycila::Logger.info(TAG, "Stopping network services...");
  Mycila::MQTT.end();
  Mycila::HTTPd.end();
  Mycila::Buzzer.beep(2);
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task otaPrepareTask = Mycila::Task("otaPrepareTask", [](void* params) {
  Mycila::Logger.info(TAG, "Preparing OTA update...");
  lightsTask.pause();
  mqttTask.pause();
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
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

// reconfigure tasks

Mycila::Task configureButtonTask = Mycila::Task("configureButtonTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Push Button...");
  Mycila::Button.end();
  if (Mycila::Config.getBool(KEY_BUTTON_ENABLE)) {
    Mycila::Button.begin(Mycila::Config.get(KEY_BUTTON_PIN).toInt(), Mycila::Config.get(KEY_BUTTON_ACTION), "reset");
  } else {
    Mycila::Logger.warn(TAG, "Button not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureBuzzerTask = Mycila::Task("configureBuzzerTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Buzzer...");
  Mycila::Buzzer.end();
  if (Mycila::Config.getBool(KEY_BUZZER_ENABLE)) {
    Mycila::Buzzer.begin(Mycila::Config.get(KEY_BUZZER_PIN).toInt());
  } else {
    Mycila::Logger.warn(TAG, "Buzzer not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureDisplayTask = Mycila::Task("configureDisplayTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Display...");
  Mycila::EasyDisplay.end();
  if (Mycila::Config.getBool(KEY_DISPLAY_ENABLE)) {
    Mycila::EasyDisplay.setPowerSaveDelay(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt());
    Mycila::EasyDisplay.begin(YaSolR::YaSolR.getDisplayType(), Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN).toInt(), Mycila::Config.get(KEY_DISPLAY_DATA_PIN).toInt(), YaSolR::YaSolR.getDisplayRotation());
  } else {
    Mycila::Logger.warn(TAG, "Display not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureGridTask = Mycila::Task("configureGridTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Grid Power...");
  Mycila::Grid.end();
  Mycila::Grid.begin(Mycila::Config.get(KEY_GRID_POWER_MQTT_TOPIC));
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureJSYTask = Mycila::Task("configureJSYTask", [](void* params) {
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
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureLightsTask = Mycila::Task("configureLightsTask", [](void* params) {
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
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureMQTTTask = Mycila::Task("configureMQTTTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configuring MQTT...");
  Mycila::MQTT.end();
  Mycila::MQTT.begin();
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureNetworkTask = Mycila::Task("configureNetworkTask", [](void* params) {
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
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureNTPTask = Mycila::Task("configureNTPTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configuring NTP...");
  Mycila::NTP.setTimeZone(Mycila::Config.get(KEY_NTP_TIMEZONE));
  if (ESPConnect.isConnected() && !Mycila::Config.getBool(KEY_AP_MODE_ENABLE)) {
    Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureOutput1BypassRelayTask = Mycila::Task("configureOutput1BypassRelayTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 1 Bypass Relay...");
  output1BypassRelay.end();
  if (Mycila::Config.getBool(KEY_OUTPUT1_RELAY_ENABLE)) {
    output1BypassRelay.begin(Mycila::Config.get(KEY_OUTPUT1_RELAY_PIN).toInt(), Mycila::Config.get(KEY_OUTPUT1_RELAY_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Output 1 Bypass Relay not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureOutput1DimmerTask = Mycila::Task("configureOutput1DimmerTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 1 Dimmer...");
  output1Dimmer.end();
  if (Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_ENABLE)) {
    output1Dimmer.begin(Mycila::Config.get(KEY_OUTPUT1_DIMMER_PIN).toInt(), YaSolR::YaSolR.getDimmerType(output1Dimmer.getName()));
  } else {
    Mycila::Logger.warn(TAG, "Output 1 Dimmer not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureOutput1TemperatureSensorTask = Mycila::Task("configureOutput1TemperatureSensorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 1 Temperature Sensor...");
  output1TemperatureSensor.end();
  if (Mycila::Config.getBool(KEY_OUTPUT1_TEMP_ENABLE)) {
    output1TemperatureSensor.begin(Mycila::Config.get(KEY_OUTPUT1_TEMP_PIN).toInt(), 6 * YASOLR_TEMPERATURE_READ_INTERVAL);
  } else {
    Mycila::Logger.warn(TAG, "Output 1 Temperature Sensor not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureOutput2BypassRelayTask = Mycila::Task("configureOutput2BypassRelayTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 2 Bypass Relay...");
  output2BypassRelay.end();
  if (Mycila::Config.getBool(KEY_OUTPUT2_RELAY_ENABLE)) {
    output2BypassRelay.begin(Mycila::Config.get(KEY_OUTPUT2_RELAY_PIN).toInt(), Mycila::Config.get(KEY_OUTPUT2_RELAY_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Output 2 Bypass Relay not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureOutput2DimmerTask = Mycila::Task("configureOutput2DimmerTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 2 Dimmer...");
  output2Dimmer.end();
  if (Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_ENABLE)) {
    output2Dimmer.begin(Mycila::Config.get(KEY_OUTPUT2_DIMMER_PIN).toInt(), YaSolR::YaSolR.getDimmerType(output2Dimmer.getName()));
  } else {
    Mycila::Logger.warn(TAG, "Output 2 Dimmer not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureOutput2TemperatureSensorTask = Mycila::Task("configureOutput2TemperatureSensorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Output 2 Temperature Sensor...");
  output2TemperatureSensor.end();
  if (Mycila::Config.getBool(KEY_OUTPUT2_TEMP_ENABLE)) {
    output2TemperatureSensor.begin(Mycila::Config.get(KEY_OUTPUT2_TEMP_PIN).toInt(), 6 * YASOLR_TEMPERATURE_READ_INTERVAL);
  } else {
    Mycila::Logger.warn(TAG, "Output 2 Temperature Sensor not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureRelay1Task = Mycila::Task("configureRelay1Task", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Relay 1...");
  relay1.end();
  if (Mycila::Config.getBool(KEY_RELAY1_ENABLE)) {
    relay1.begin(Mycila::Config.get(KEY_RELAY1_PIN).toInt(), Mycila::Config.get(KEY_RELAY1_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Relay 1 not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureRelay2Task = Mycila::Task("configureRelay2Task", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Relay 2...");
  relay2.end();
  if (Mycila::Config.getBool(KEY_RELAY2_ENABLE)) {
    relay2.begin(Mycila::Config.get(KEY_RELAY2_PIN).toInt(), Mycila::Config.get(KEY_RELAY2_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  } else {
    Mycila::Logger.warn(TAG, "Relay 2 not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureSystemTemperatureSensorTask = Mycila::Task("configureSystemTemperatureSensorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure System Temperature Sensor...");
  systemTemperatureSensor.end();
  if (Mycila::Config.getBool(KEY_SYSTEM_TEMP_ENABLE)) {
    systemTemperatureSensor.begin(Mycila::Config.get(KEY_SYSTEM_TEMP_PIN).toInt(), 6 * YASOLR_TEMPERATURE_READ_INTERVAL);
  } else {
    Mycila::Logger.warn(TAG, "System Temperature Sensor not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureTaskMonitorTask = Mycila::Task("configureTaskMonitorTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Task Stack Monitor...");
  Mycila::TaskMonitor.begin(5);
  Mycila::TaskMonitor.addTask("loopTask");
  Mycila::TaskMonitor.addTask("async_tcp");
  Mycila::TaskMonitor.addTask("mqttclient");
  Mycila::TaskMonitor.addTask("jsyTask");
  Mycila::TaskMonitor.addTask("ioTask");
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

Mycila::Task configureZCDTask = Mycila::Task("configureZCDTask", [](void* params) {
  Mycila::Logger.info(TAG, "Configure Zero-Cross Detection...");
  Mycila::ZCD.end();
  if (Mycila::Config.getBool(KEY_ZCD_ENABLE)) {
    Mycila::ZCD.begin(Mycila::Config.get(KEY_ZCD_PIN).toInt(), Mycila::Config.get(KEY_GRID_FREQ).toInt() == 60 ? 60 : 50);
  } else {
    Mycila::Logger.warn(TAG, "Zero-Cross Detection not enabled");
  }
}, Mycila::TaskType::ONCE).debugIf(DEBUG_ENABLED);

