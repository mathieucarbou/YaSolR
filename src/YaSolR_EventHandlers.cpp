// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */

#include <YaSolR.h>

#define TAG "EVENTS"

void refresh() {
  websiteTask.requestEarlyRun();
  publisherTask.requestEarlyRun();
}

void YaSolR::YaSolRClass::_initEventHandlers() {
  Mycila::Button.listen([this](const String& action) {
    if (action == "restart")
      restartTask.resume();
    else if (action == "reset")
      resetTask.resume();
    else if (action == "bypass") {
      for (auto& output : Mycila::Router.outputs)
        output.tryBypassRelayToggle();
      refresh();
    }
  });

  Mycila::OTA.listen([this]() {
    otaPrepareTask.resume();
  });

  Mycila::OTA.listen([this](bool success) {
    if (success) {
      Mycila::Logger.info(TAG, "OTA Update Success! Restarting...");
    } else {
      Mycila::Logger.error(TAG, "OTA Failed! Restarting...");
    }
    restartTask.resume();
  });

  Mycila::MQTT.onConnect([](void) {
    Mycila::Logger.info(TAG, "MQTT connected!");
    haDiscoTask.resume();
  });

  ESPConnect.listen([this](ESPConnectState previous, ESPConnectState state) {
    Mycila::Logger.debug(TAG, "NetworkState: %s => %s", ESPConnect.getStateName(previous), ESPConnect.getStateName(state));
    switch (state) {
      case ESPConnectState::NETWORK_CONNECTED:
        Mycila::Lights.setRed(false);
        Mycila::Logger.info(TAG, "Connected with IP address %s", ESPConnect.getIPAddress().toString().c_str());
        startNetworkServicesTask.resume();
        break;
      case ESPConnectState::AP_STARTED:
        Mycila::Lights.setRed(false);
        Mycila::Logger.info(TAG, "Access Point %s started with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        startNetworkServicesTask.resume();
        break;
      case ESPConnectState::NETWORK_DISCONNECTED:
        Mycila::Lights.setRed(true);
        Mycila::Logger.warn(TAG, "Disconnected!");
        stopNetworkServicesTask.resume();
        break;
      case ESPConnectState::NETWORK_DISABLED:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Disable Network...");
        break;
      case ESPConnectState::NETWORK_CONNECTING:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Connecting to network...");
        break;
      case ESPConnectState::AP_STARTING:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Starting Access Point %s...", ESPConnect.getAccessPointSSID().c_str());
        break;
      case ESPConnectState::PORTAL_STARTING:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Starting Captive Portal %s for %u seconds...", ESPConnect.getAccessPointSSID().c_str(), ESPConnect.getCaptivePortalTimeout());
        break;
      case ESPConnectState::PORTAL_STARTED:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Captive Portal started at %s with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        Mycila::Buzzer.beep(2);
        break;
      case ESPConnectState::PORTAL_COMPLETE: {
        bool ap = ESPConnect.hasConfiguredAPMode();
        if (ap) {
          Mycila::Logger.info(TAG, "Captive Portal: Access Point configured");
          Mycila::Config.setBool(KEY_AP_MODE_ENABLE, true);
        } else {
          Mycila::Logger.info(TAG, "Captive Portal: WiFi configured");
          Mycila::Config.setBool(KEY_AP_MODE_ENABLE, false);
          Mycila::Config.set(KEY_WIFI_SSID, ESPConnect.getConfiguredWiFiSSID());
          Mycila::Config.set(KEY_WIFI_PASSWORD, ESPConnect.getConfiguredWiFiPassword());
        }
        restartTask.resume();
        break;
      }
      case ESPConnectState::PORTAL_TIMEOUT:
        Mycila::Logger.warn(TAG, "Captive Portal: timed out.");
        restartTask.resume();
        break;
      case ESPConnectState::NETWORK_TIMEOUT:
        Mycila::Logger.warn(TAG, "Unable to connect!");
        break;
      case ESPConnectState::NETWORK_RECONNECTING:
        Mycila::Logger.info(TAG, "Trying to reconnect...");
        break;
      default:
        break;
    }
  });

  Mycila::Config.listen([this](const String& key, const String& oldValue, const String& newValue) {
    YaSolR::Event event = YaSolR::Event::NONE;
    if (key == KEY_MQTT_ENABLE || key == KEY_HA_DISCOVERY_ENABLE)
      event = Event::MQTT_RECONFIGURED;
    else if (key == KEY_NTP_TIMEZONE || key == KEY_NTP_SERVER)
      event = Event::NTP_RECONFIGURED;
    else if (key == KEY_SYSTEM_TEMP_ENABLE)
      event = Event::SYS_TEMP_RECONFIGURED;
    else if (key == KEY_DISPLAY_ENABLE)
      event = Event::DISPLAY_RECONFIGURED;
    else if (key == KEY_BUTTON_ENABLE)
      event = Event::BUTTON_RECONFIGURED;
    else if (key == KEY_DISPLAY_POWER_SAVE_DELAY)
      Mycila::EasyDisplay.setPowerSaveDelay(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt());
    else if (key == KEY_AP_MODE_ENABLE && (ESPConnect.getState() == ESPConnectState::AP_STARTED ||
                                           ESPConnect.getState() == ESPConnectState::NETWORK_CONNECTING ||
                                           ESPConnect.getState() == ESPConnectState::NETWORK_CONNECTED ||
                                           ESPConnect.getState() == ESPConnectState::NETWORK_TIMEOUT ||
                                           ESPConnect.getState() == ESPConnectState::NETWORK_DISCONNECTED ||
                                           ESPConnect.getState() == ESPConnectState::NETWORK_RECONNECTING))
      event = Event::NETWORK_RECONFIGURED;
    else if (key == KEY_BUZZER_ENABLE)
      event = Event::BUZZER_RECONFIGURED;
    else if (key == KEY_LIGHTS_ENABLE)
      event = Event::LIGHTS_RECONFIGURED;
    else if (key == KEY_RELAY1_ENABLE)
      event = Event::RELAY1_RECONFIGURED;
    else if (key == KEY_RELAY2_ENABLE)
      event = Event::RELAY2_RECONFIGURED;
    else if (key == KEY_OUTPUT1_RELAY_ENABLE)
      event = Event::OUT1_RELAY_RECONFIGURED;
    else if (key == KEY_OUTPUT1_TEMP_ENABLE)
      event = Event::OUT1_TEMP_RECONFIGURED;
    else if (key == KEY_OUTPUT2_RELAY_ENABLE)
      event = Event::OUT2_RELAY_RECONFIGURED;
    else if (key == KEY_OUTPUT2_TEMP_ENABLE)
      event = Event::OUT2_TEMP_RECONFIGURED;
    else if (key == KEY_OUTPUT1_DIMMER_ENABLE)
      event = Event::OUT1_DIMMER_RECONFIGURED;
    else if (key == KEY_OUTPUT2_DIMMER_ENABLE)
      event = Event::OUT2_DIMMER_RECONFIGURED;
    else if (key == KEY_JSY_ENABLE)
      event = Event::JSY_RECONFIGURED;
    else if (key == KEY_ZCD_ENABLE)
      event = Event::ZCD_RECONFIGURED;
    else if (key == KEY_GRID_POWER_MQTT_TOPIC)
      event = Event::GRID_RECONFIGURED;

    if (event != Event::NONE) {
      reconfigureTask.setData(&event).resume();
      reconfigureTask.resume();
    }
  });

  Mycila::Config.listen([this]() {
    Mycila::Logger.info(TAG, "Configuration restored. Restarting...");
    restartTask.resume();
  });

  for (auto& relay : Mycila::RelayManager.relays)
    relay.listen([this](bool state) { refresh(); });

  for (auto& output : Mycila::Router.outputs) {
    output.dimmer.listen([this](Mycila::DimmerLevel event) { refresh(); });
    output.relay.listen([this](bool state) { refresh(); });
    output.temperatureSensor.listen([this](const char* name, float temperature) {
      Mycila::Logger.info(TAG, "Temperature '%s' changed to %.02f °C", name, temperature);
      refresh();
    });
  }

  systemTemperatureSensor.listen([this](const char* name, float temperature) {
    Mycila::Logger.info(TAG, "Temperature '%s' changed to %.02f °C", name, temperature);
    refresh();
  });
}
