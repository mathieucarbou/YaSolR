// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

const Mycila::TaskEnablePredicate noOTA = []() { return !Mycila::OTA.isUpdating(); };
const Mycila::TaskEnablePredicate mqttEnabled = []() { return Mycila::MQTT.isEnabled() && !Mycila::OTA.isUpdating(); };

// Core tasks

Mycila::Task buttonTask("Button.processEvents()", [](void* params) { Mycila::Button.processEvents(); }, Mycila::TaskType::FOREVER, 0, true);

Mycila::Task displayTask("YaSolR.updateDisplay()", [](void* params) { YaSolR::YaSolR.updateDisplay(); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS, true);

Mycila::Task espConnectTask("ESPConnect.loop()", [](void* params) { ESPConnect.loop(); }, Mycila::TaskType::FOREVER, 0, true);

Mycila::Task websiteTask = Mycila::Task("Website.update()", [](void* params) { website.update(); }, Mycila::TaskType::FOREVER, 1 * Mycila::TimeUnit::SECONDS)
  .debug()
  .when([]() { return Mycila::HTTPd.isEnabled(); });

Mycila::Task otaTask = Mycila::Task("OTA.loop()", [](void* params) { Mycila::OTA.loop(); }, Mycila::TaskType::FOREVER)
  .when([]() { return Mycila::OTA.isEnabled(); });

// Core tasks not on OTA

Mycila::Task lightsTask = Mycila::Task("Lights", [](void* params) { Mycila::Lights.setYellow(Mycila::RelayManager.relays[0].isOn() || Mycila::RelayManager.relays[1].isOn() || Mycila::Router.outputs[0].relay.isOn() || Mycila::Router.outputs[1].relay.isOn() || Mycila::Router.outputs[0].dimmer.isOn() || Mycila::Router.outputs[1].dimmer.isOn()); }, Mycila::TaskType::FOREVER, 500 * Mycila::TimeUnit::MILLISECONDS)
  .when(noOTA);

Mycila::Task stackMonitorTask = Mycila::Task("TaskMonitor.log()", [](void* params) { Mycila::TaskMonitor.log(); }, Mycila::TaskType::FOREVER, 10 * Mycila::TimeUnit::SECONDS)
  .debug()
  .when([]() { return Mycila::Logger.isDebugEnabled() && noOTA(); });

#ifdef APP_VERSION_TRIAL
Mycila::Task trialTask = Mycila::Task("Trial.validate()", [](void* params) { Mycila::Trial.validate(); }, Mycila::TaskType::FOREVER, 20 * Mycila::TimeUnit::SECONDS)
  .debug()
  .when(noOTA);
#endif

// Router tasks not on OTA

Mycila::Task autoRelayTask = Mycila::Task("RelayManager.autoCommute()", [](void* params) { Mycila::RelayManager.autoCommute(); }, Mycila::TaskType::FOREVER)
  .interval([]() { return YaSolR::YaSolR.getRelayPauseDuration() * Mycila::TimeUnit::SECONDS; })
  .when(noOTA);

// Network tasks not on OTA and and only when mqtt is enabled / connected

Mycila::Task mqttTask = Mycila::Task("MQTT.loop()", [](void* params) { Mycila::MQTT.loop(); })
  .when(mqttEnabled);

Mycila::Task publisherTask = Mycila::Task("YaSolR.publishMQTT()", [](void* params) { YaSolR::YaSolR.publishMQTT(); }, Mycila::TaskType::FOREVER)
  .debug()
  .interval([]() { return Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt() * Mycila::TimeUnit::SECONDS; })
  .when([]() { return Mycila::MQTT.isConnected() && !Mycila::OTA.isUpdating(); });

// On Demand tasks

Mycila::Task haDiscoTask = Mycila::Task("YaSolR.publishHADiscovery()", [](void* params) { YaSolR::YaSolR.publishHADiscovery(); }, Mycila::TaskType::ONCE)
  .debug();

Mycila::Task restartTask = Mycila::Task("restart()", [](void* params) {
  Mycila::Logger.warn(TAG, "Restarting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
  Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::OFF, Mycila::LightState::ON);
  delay(500);
  Mycila::EasyDisplay.end();
  Mycila::Buzzer.beep(2);
  Mycila::Buzzer.end();
  Mycila::Router.end();
  Mycila::RelayManager.end();
  Mycila::JSY.end();
  Mycila::ZCD.end();
  Mycila::Lights.end();
  ESPConnect.end();
  delay(500);
  Mycila::System.restart();
}, Mycila::TaskType::ONCE).debug();

Mycila::Task resetTask = Mycila::Task("reset()", [](void* params) {
  Mycila::Logger.warn(TAG, "Resetting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
  Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::ON, Mycila::LightState::ON);
  delay(500);
  Mycila::EasyDisplay.end();
  Mycila::Buzzer.beep(3);
  Mycila::Buzzer.end();
  Mycila::Router.end();
  Mycila::RelayManager.end();
  Mycila::JSY.end();
  Mycila::ZCD.end();
  Mycila::Lights.end();
  ESPConnect.end();
  Mycila::Config.clear();
  delay(500);
  Mycila::System.restart();
}, Mycila::TaskType::ONCE).debug();

Mycila::Task startNetworkServicesTask = Mycila::Task("startNetworkServices()", [](void* params) {
  Mycila::Logger.info(TAG, "Starting network services...");
  Mycila::OTA.begin();
  Mycila::HTTPd.begin();
  Mycila::MQTT.begin();
  if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE))
    Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
}, Mycila::TaskType::ONCE).debug();

Mycila::Task stopNetworkServicesTask = Mycila::Task("stopNetworkServices()", [](void* params) {
  Mycila::Logger.info(TAG, "Stopping network services...");
  Mycila::MQTT.end();
  Mycila::HTTPd.end();
  Mycila::OTA.end();
  Mycila::Buzzer.beep(2);
}, Mycila::TaskType::ONCE).debug();

Mycila::Task otaPrepareTask = Mycila::Task("otaPrepare()", [](void* params) {
  Mycila::Logger.info(TAG, "Preparing OTA update...");
  Mycila::RelayManager.end();
  Mycila::Router.end();
  Mycila::Lights.setAllOn();
  Mycila::EasyDisplay.end();
  systemTemperatureSensor.end();
  Mycila::JSY.end();
  Mycila::ZCD.end();
  Mycila::MQTT.end();
#ifdef APP_VERSION_TRIAL
  Mycila::Trial.end();
#endif
}, Mycila::TaskType::ONCE).debug();

Mycila::Task reconfigureTask = Mycila::Task("reconfigure()", [](void* params) {
  const YaSolR::Event event = *(YaSolR::Event*)params;
  switch (event) {
    case YaSolR::Event::MQTT_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring MQTT...");
      Mycila::MQTT.end();
      Mycila::MQTT.begin();
      break;
    }
    case YaSolR::Event::NTP_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring NTP...");
      Mycila::NTP.setTimeZone(Mycila::Config.get(KEY_NTP_TIMEZONE));
      if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE)) {
        Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
      }
      break;
    }
    case YaSolR::Event::RELAY1_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Relay 1...");
      Mycila::RelayManager.relays[0].end();
      Mycila::RelayManager.relays[0].begin();
      break;
    }
    case YaSolR::Event::RELAY2_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Relay 2...");
      Mycila::RelayManager.relays[1].end();
      Mycila::RelayManager.relays[1].begin();
      break;
    }
    case YaSolR::Event::BUZZER_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Buzzer...");
      Mycila::Buzzer.end();
      Mycila::Buzzer.begin();
      if (Mycila::Buzzer.isEnabled())
        Mycila::Buzzer.beep();
      break;
    }
    case YaSolR::Event::BUTTON_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Button...");
      Mycila::Button.end();
      Mycila::Button.begin();
      break;
    }
    case YaSolR::Event::LIGHTS_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring LEDs...");
      Mycila::LightState g = Mycila::Lights.getGreen();
      Mycila::LightState y = Mycila::Lights.getYellow();
      Mycila::LightState r = Mycila::Lights.getRed();
      Mycila::Lights.end();
      Mycila::Lights.begin(g, y, r);
      break;
    }
    case YaSolR::Event::SYS_TEMP_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring System Temperature Sensor...");
      systemTemperatureSensor.end();
      systemTemperatureSensor.begin();
      break;
    }
    case YaSolR::Event::OUT1_RELAY_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Output 1 Bypass '%s'...", Mycila::Router.outputs[0].relay.name);
      Mycila::Router.outputs[0].relay.end();
      Mycila::Router.outputs[0].relay.begin();
      break;
    }
    case YaSolR::Event::OUT1_TEMP_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Temperature Sensor '%s'...", Mycila::Router.outputs[0].temperatureSensor.name);
      Mycila::Router.outputs[0].temperatureSensor.end();
      Mycila::Router.outputs[0].temperatureSensor.begin();
      break;
    }
    case YaSolR::Event::OUT1_DIMMER_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Dimmer '%s'...", Mycila::Router.outputs[0].dimmer.name);
      Mycila::Router.outputs[0].dimmer.end();
      Mycila::Router.outputs[0].dimmer.begin();
      break;
    }
    case YaSolR::Event::OUT2_RELAY_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Output 1 Bypass '%s'...", Mycila::Router.outputs[1].relay.name);
      Mycila::Router.outputs[1].relay.end();
      Mycila::Router.outputs[1].relay.begin();
      break;
    }
    case YaSolR::Event::OUT2_TEMP_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Temperature Sensor '%s'...", Mycila::Router.outputs[1].temperatureSensor.name);
      Mycila::Router.outputs[1].temperatureSensor.end();
      Mycila::Router.outputs[1].temperatureSensor.begin();
      break;
    }
    case YaSolR::Event::OUT2_DIMMER_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Dimmer '%s'...", Mycila::Router.outputs[1].dimmer.name);
      Mycila::Router.outputs[1].dimmer.end();
      Mycila::Router.outputs[1].dimmer.begin();
      break;
    }
    case YaSolR::Event::JSY_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring JSY...");
      Mycila::JSY.end();
      if (Mycila::Config.getBool(KEY_JSY_ENABLE)) {
        Mycila::JSY.begin(Mycila::Config.get(KEY_JSY_RX_PIN).toInt(), Mycila::Config.get(KEY_JSY_TX_PIN).toInt(), &MYCILA_JSY_SERIAL, true);
        if (Mycila::JSY.isEnabled() && Mycila::JSY.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400) {
          Mycila::Logger.info(TAG, "Trying to update JSY Baud Rate to 38400...");
          if (!Mycila::JSY.updateBaudRate(Mycila::JSYBaudRate::BAUD_38400)) {
            Mycila::Logger.warn(TAG, "Failed to update JSY Baud Rate to 38400");
          }
        }
      } else {
        Mycila::Logger.warn(TAG, "JSY not enabled");
      }
      break;
    }
    case YaSolR::Event::ZCD_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring ZCD...");
      Mycila::ZCD.end();
      Mycila::ZCD.begin();
      break;
    }
    case YaSolR::Event::GRID_RECONFIGURED: {
      Mycila::Logger.info(TAG, "Reconfiguring Grid...");
      Mycila::Grid.end();
      Mycila::Grid.begin();
      break;
    }
    case YaSolR::Event::NETWORK_RECONFIGURED:
      Mycila::Logger.info(TAG, "Reconfiguring Network...");
      Mycila::MQTT.end();
      Mycila::HTTPd.end();
      Mycila::OTA.end();
      ESPConnect.end();
      ESPConnect.setCaptivePortalTimeout(Mycila::Config.get(KEY_CAPTURE_PORTAL_TIMEOUT).toInt());
      ESPConnect.setConnectTimeout(Mycila::Config.get(KEY_WIFI_CONNECTION_TIMEOUT).toInt());
      ESPConnect.begin(&Mycila::HTTPd.server, Mycila::Config.get(KEY_HOSTNAME), Mycila::AppInfo.name + "-" + Mycila::AppInfo.id, Mycila::Config.get(KEY_ADMIN_PASSWORD), {Mycila::Config.get(KEY_WIFI_SSID), Mycila::Config.get(KEY_WIFI_PASSWORD), Mycila::Config.getBool(KEY_AP_MODE_ENABLE)});
      break;

    case YaSolR::Event::DISPLAY_RECONFIGURED:
      Mycila::EasyDisplay.end();
      if (Mycila::Config.getBool(KEY_DISPLAY_ENABLE)) {
        Mycila::EasyDisplay.setPowerSaveDelay(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt());
        Mycila::EasyDisplay.begin(YaSolR::YaSolR.getDisplayType(), Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN).toInt(), Mycila::Config.get(KEY_DISPLAY_DATA_PIN).toInt(), YaSolR::YaSolR.getDisplayRotation());
      } else {
        Mycila::Logger.warn(TAG, "Display not enabled");
      }
      break;

    default:
      break;
  }
}, Mycila::TaskType::ONCE).debug();
