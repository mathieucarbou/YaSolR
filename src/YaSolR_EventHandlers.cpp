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
  Mycila::Button.listen([this](Mycila::ButtonEvent event) {
    switch (event) {
      case Mycila::ButtonEvent::BUTTON_CLICKED:
        if (Mycila::Config.get(KEY_BUTTON_ACTION) == "bypass") {
          output1.tryBypassRelayToggle();
          output2.tryBypassRelayToggle();
          refresh();
        } else {
          restartTask.resume();
        }
        break;
      case Mycila::ButtonEvent::BUTTON_LONG_PRESS_HOLD:
        resetTask.resume();
        break;
      default:
        assert(false);
        break;
    }
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
    Mycila::Logger.info(TAG, "Set %s: '%s' => '%s'", key.c_str(), oldValue.c_str(), newValue.c_str());
    if (key == KEY_MQTT_ENABLE || key == KEY_HA_DISCOVERY_ENABLE)
      configureMQTTTask.resume();
    else if (key == KEY_AP_MODE_ENABLE && (ESPConnect.getState() == ESPConnectState::AP_STARTED || ESPConnect.getState() == ESPConnectState::NETWORK_CONNECTING || ESPConnect.getState() == ESPConnectState::NETWORK_CONNECTED || ESPConnect.getState() == ESPConnectState::NETWORK_TIMEOUT || ESPConnect.getState() == ESPConnectState::NETWORK_DISCONNECTED || ESPConnect.getState() == ESPConnectState::NETWORK_RECONNECTING))
      configureNetworkTask.resume();
    else if (key == KEY_BUTTON_ENABLE)
      configureButtonTask.resume();
    else if (key == KEY_BUZZER_ENABLE)
      configureBuzzerTask.resume();
    else if (key == KEY_DISPLAY_ENABLE)
      configureDisplayTask.resume();
    else if (key == KEY_DEBUG_ENABLE) {
      configureDebugTask.resume();
      esp_log_level_set("*", static_cast<esp_log_level_t>(Mycila::Logger.getLevel()));
    } else if (key == KEY_DISPLAY_POWER_SAVE_DELAY)
      Mycila::EasyDisplay.setPowerSaveDelay(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt());
    else if (key == KEY_GRID_POWER_MQTT_TOPIC || key == KEY_GRID_VOLTAGE_MQTT_TOPIC || key == KEY_GRID_FREQ)
      configureGridTask.resume();
    else if (key == KEY_JSY_ENABLE)
      configureJSYTask.resume();
    else if (key == KEY_LIGHTS_ENABLE)
      configureLightsTask.resume();
    else if (key == KEY_NTP_TIMEZONE || key == KEY_NTP_SERVER)
      configureNTPTask.resume();
    else if (key == KEY_OUTPUT1_DIMMER_ENABLE)
      configureOutput1DimmerTask.resume();
    else if (key == KEY_OUTPUT1_RELAY_ENABLE)
      configureOutput1BypassRelayTask.resume();
    else if (key == KEY_OUTPUT1_TEMP_ENABLE)
      configureOutput1TemperatureSensorTask.resume();
    else if (key == KEY_OUTPUT1_PZEM_ENABLE)
      configureOutput1PZEMTask.resume();
    else if (key == KEY_OUTPUT2_DIMMER_ENABLE)
      configureOutput2DimmerTask.resume();
    else if (key == KEY_OUTPUT2_RELAY_ENABLE)
      configureOutput2BypassRelayTask.resume();
    else if (key == KEY_OUTPUT2_TEMP_ENABLE)
      configureOutput2TemperatureSensorTask.resume();
    else if (key == KEY_OUTPUT2_PZEM_ENABLE)
      configureOutput2PZEMTask.resume();
    else if (key == KEY_RELAY1_ENABLE)
      configureRelay1Task.resume();
    else if (key == KEY_RELAY2_ENABLE)
      configureRelay2Task.resume();
    else if (key == KEY_SYSTEM_TEMP_ENABLE)
      configureSystemTemperatureSensorTask.resume();
    else if (key == KEY_ZCD_ENABLE)
      configureZCDTask.resume();
  });

  Mycila::Config.listen([this]() {
    Mycila::Logger.info(TAG, "Configuration restored.");
    restartTask.resume();
  });

  output1Dimmer.listen([this](Mycila::DimmerLevel event) {
    switch (event) {
      case Mycila::DimmerLevel::OFF:
        Mycila::Logger.debug(TAG, "Output 1 dimmer: turned off");
        break;
      case Mycila::DimmerLevel::FULL:
        Mycila::Logger.debug(TAG, "Output 1 dimmer: at full power");
        break;
      case Mycila::DimmerLevel::DIM:
        Mycila::Logger.debug(TAG, "Output 1 dimmer: dimming");
        break;
      default:
        assert(false);
        break;
    }
    refresh();
  });
  output2Dimmer.listen([this](Mycila::DimmerLevel event) {
    switch (event) {
      case Mycila::DimmerLevel::OFF:
        Mycila::Logger.debug(TAG, "Output 2 dimmer: turned off");
        break;
      case Mycila::DimmerLevel::FULL:
        Mycila::Logger.debug(TAG, "Output 2 dimmer: at full power");
        break;
      case Mycila::DimmerLevel::DIM:
        Mycila::Logger.debug(TAG, "Output 2 dimmer: dimming");
        break;
      default:
        assert(false);
        break;
    }
    refresh();
  });

  output1BypassRelay.listen([this](bool state) {
    if (state)
      Mycila::Logger.info(TAG, "Output 1 bypass relay: turned on");
    else
      Mycila::Logger.info(TAG, "Output 1 bypass relay: turned off");
    refresh();
  });
  output2BypassRelay.listen([this](bool state) {
    if (state)
      Mycila::Logger.info(TAG, "Output 2 bypass relay: turned on");
    else
      Mycila::Logger.info(TAG, "Output 2 bypass relay: turned off");
    refresh();
  });

  relay1.listen([this](bool state) {
    if (state)
      Mycila::Logger.info(TAG, "Relay 1: turned on");
    else
      Mycila::Logger.info(TAG, "Relay 1: turned off");
    refresh();
  });
  relay2.listen([this](bool state) {
    if (state)
      Mycila::Logger.info(TAG, "Relay 2: turned on");
    else
      Mycila::Logger.info(TAG, "Relay 2: turned off");
    refresh();
  });

  systemTemperatureSensor.listen([this](float temperature) {
    Mycila::Logger.info(TAG, "Temperature '" NAME_SYSTEM "' changed to %.02f °C", temperature);
    refresh();
  });

  output1TemperatureSensor.listen([this](float temperature) {
    Mycila::Logger.info(TAG, "Temperature '" NAME_OUTPUT1 "' changed to %.02f °C", temperature);
    refresh();
  });

  output2TemperatureSensor.listen([this](float temperature) {
    Mycila::Logger.info(TAG, "Temperature '" NAME_OUTPUT2 "' changed to %.02f °C", temperature);
    refresh();
  });
}
