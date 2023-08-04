// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */

#include <YaSolRController.h>

#include <MycilaESPConnect.h>

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
#include <MycilaRouter.h>
#include <MycilaSystem.h>
#include <MycilaZeroCrossDetection.h>

#include <YaSolR.h>

#define TAG "CONTROL"

void YaSolR::Controller::begin() {
  Mycila::Logger.info(TAG, "Enable Event Manager...");

  Mycila::Button.listen([this](const String& action) {
    if (action == "restart")
      _restartThrottle.setEnable(true);
    else if (action == "reset")
      _resetThrottle.setEnable(true);
    else if (action == "bypass") {
      for (auto& output : Mycila::Router.outputs)
        output.tryBypassRelayToggle();
      _triggerRefresh();
    }
  });

  Mycila::System.listen([this](Mycila::SystemEvent event) {
    if (event == Mycila::SystemEvent::BEFORE_RESET) {
      Mycila::Logger.warn(TAG, "Resetting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
      Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::ON, Mycila::LightState::ON);
      Mycila::Display.end();
      Mycila::Buzzer.beep(3);
      Mycila::Buzzer.end();
      Mycila::Router.end();
      Mycila::RelayManager.end();
      Mycila::JSY.end();
      Mycila::ZCD.end();
      Mycila::Config.end();
      Mycila::Lights.end();
      ESPConnect.end();
    } else if (event == Mycila::SystemEvent::BEFORE_RESTART) {
      Mycila::Logger.warn(TAG, "Restarting %s %s %s...", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
      Mycila::Lights.set(Mycila::LightState::OFF, Mycila::LightState::OFF, Mycila::LightState::ON);
      Mycila::Display.end();
      Mycila::Buzzer.beep(2);
      Mycila::Buzzer.end();
      Mycila::Router.end();
      Mycila::RelayManager.end();
      Mycila::JSY.end();
      Mycila::ZCD.end();
      Mycila::Config.end();
      Mycila::Lights.end();
      ESPConnect.end();
    }
  });

  Mycila::OTA.listen([this]() {
    Mycila::Logger.info(TAG, "Disabling services in preparation for OTA Update...");
    Mycila::RelayManager.end();
    Mycila::Router.end();
    Mycila::Lights.setAllOn();
    Mycila::JSY.end();
    Mycila::ZCD.end();
    Mycila::MQTT.end();
  });

  Mycila::OTA.listen([this](bool success) {
    if (success) {
      Mycila::Logger.info(TAG, "OTA Update Success! Restarting...");
    } else {
      Mycila::Logger.error(TAG, "OTA Failed!");
    }
    _restartThrottle.setEnable(true);
  });

  ESPConnect.listen([this](ESPConnectState previous, ESPConnectState state) {
    Mycila::Logger.debug(TAG, "NetworkState: %s => %s", ESPConnect.getStateName(previous), ESPConnect.getStateName(state));
    switch (state) {
      case ESPConnectState::STA_CONNECTED:
        Mycila::Lights.setRed(false);
        Mycila::Logger.info(TAG, "Connected to WiFi %s with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        _events.push(Event::NETWORK_CONNECTED);
        break;
      case ESPConnectState::AP_CONNECTED:
        Mycila::Lights.setRed(false);
        Mycila::Logger.info(TAG, "Access Point %s started with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        _events.push(Event::NETWORK_CONNECTED);
        break;
      case ESPConnectState::STA_DISCONNECTED:
        Mycila::Lights.setRed(true);
        Mycila::Logger.warn(TAG, "Disconnected from WiFi %s", ESPConnect.getWiFiSSIDConfigured().c_str());
        _events.push(Event::NETWORK_DISCONNECTED);
        break;
      case ESPConnectState::NETWORK_DISABLED:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Disable Network...");
        break;
      case ESPConnectState::STA_CONNECTING:
        Mycila::Lights.setRed(true);
        Mycila::Logger.info(TAG, "Connecting to WiFi %s (timeout: %u seconds)...", ESPConnect.getWiFiSSIDConfigured().c_str(), ESPConnect.getWiFiConnectTimeout());
        break;
      case ESPConnectState::AP_CONNECTING:
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
        bool ap = ESPConnect.isAPMode();
        if (ap) {
          Mycila::Logger.info(TAG, "Captive Portal: Access Point configured");
          Mycila::Config.setBool(KEY_AP_MODE_ENABLE, true);
        } else {
          Mycila::Logger.info(TAG, "Captive Portal: WiFi configured");
          Mycila::Config.setBool(KEY_AP_MODE_ENABLE, false);
          Mycila::Config.set(KEY_WIFI_SSID, ESPConnect.getWiFiSSIDConfigured());
          Mycila::Config.set(KEY_WIFI_PASSWORD, ESPConnect.getWiFiPassword());
        }
        _restartThrottle.setEnable(true);
        break;
      }
      case ESPConnectState::PORTAL_TIMEOUT:
        Mycila::Logger.warn(TAG, "Captive Portal: timed out.");
        _restartThrottle.setEnable(true);
        break;
      case ESPConnectState::STA_TIMEOUT:
        Mycila::Logger.error(TAG, "Unable to connect to SSID: %s", ESPConnect.getWiFiSSIDConfigured().c_str());
        break;
      case ESPConnectState::STA_RECONNECTING:
        Mycila::Logger.info(TAG, "Trying to reconnect to WiFi %s", ESPConnect.getWiFiSSIDConfigured().c_str());
        break;
      default:
        break;
    }

    _triggerRefresh();
  });

  Mycila::Config.listen([this](const String& key, const String& oldValue, const String& newValue) {
    if (key == KEY_MQTT_ENABLE || key == KEY_HA_DISCOVERY_ENABLE)
      _events.push(Event::MQTT_RECONFIGURED);
    else if (key == KEY_NTP_TIMEZONE || key == KEY_NTP_SERVER)
      _events.push(Event::NTP_RECONFIGURED);
    else if (key == KEY_SYSTEM_TEMP_ENABLE)
      _events.push(Event::SYS_TEMP_RECONFIGURED);
    else if (key == KEY_DISPLAY_ENABLE)
      _events.push(Event::DISPLAY_RECONFIGURED);
    else if (key == KEY_BUTTON_ENABLE)
      _events.push(Event::BUTTON_RECONFIGURED);
    else if (key == KEY_AP_MODE_ENABLE && (ESPConnect.getState() == ESPConnectState::AP_CONNECTED ||
                                           ESPConnect.getState() == ESPConnectState::STA_CONNECTING ||
                                           ESPConnect.getState() == ESPConnectState::STA_CONNECTED ||
                                           ESPConnect.getState() == ESPConnectState::STA_TIMEOUT ||
                                           ESPConnect.getState() == ESPConnectState::STA_DISCONNECTED ||
                                           ESPConnect.getState() == ESPConnectState::STA_RECONNECTING))
      _events.push(Event::NETWORK_RECONFIGURED);
    else if (key == KEY_BUZZER_ENABLE)
      _events.push(Event::BUZZER_RECONFIGURED);
    else if (key == KEY_LIGHTS_ENABLE)
      _events.push(Event::LIGHTS_RECONFIGURED);
    else if (key == KEY_RELAY1_ENABLE)
      _events.push(Event::RELAY1_RECONFIGURED);
    else if (key == KEY_RELAY2_ENABLE)
      _events.push(Event::RELAY2_RECONFIGURED);
    else if (key == KEY_OUTPUT1_RELAY_ENABLE)
      _events.push(Event::OUT1_RELAY_RECONFIGURED);
    else if (key == KEY_OUTPUT1_TEMP_ENABLE)
      _events.push(Event::OUT1_TEMP_RECONFIGURED);
    else if (key == KEY_OUTPUT2_RELAY_ENABLE)
      _events.push(Event::OUT2_RELAY_RECONFIGURED);
    else if (key == KEY_OUTPUT2_TEMP_ENABLE)
      _events.push(Event::OUT2_TEMP_RECONFIGURED);
    else if (key == KEY_OUTPUT1_DIMMER_ENABLE)
      _events.push(Event::OUT1_DIMMER_RECONFIGURED);
    else if (key == KEY_OUTPUT2_DIMMER_ENABLE)
      _events.push(Event::OUT2_DIMMER_RECONFIGURED);
    else if (key == KEY_JSY_ENABLE)
      _events.push(Event::JSY_RECONFIGURED);
    else if (key == KEY_ZCD_ENABLE)
      _events.push(Event::ZCD_RECONFIGURED);
    else if (key == KEY_GRID_POWER_MQTT_TOPIC)
      _events.push(Event::POWER_MANAGER_RECONFIGURED);

    _triggerRefresh();
  });

  Mycila::Config.listen([this]() {
    Mycila::Logger.info(TAG, "Configuration restored. Restarting...");
    _restartThrottle.setEnable(true);
  });

  for (auto& relay : Mycila::RelayManager.relays)
    relay.listen([this](bool state) { _triggerRefresh(); });

  for (auto& output : Mycila::Router.outputs) {
    output.dimmer.listen([this](Mycila::DimmerLevel event) { _triggerRefresh(); });
    output.relay.listen([this](bool state) { _triggerRefresh(); });
    output.temperatureSensor.listen([this](const char* name, float temperature) {
      Mycila::Logger.info(TAG, "Temperature '%s' changed to %.02f °C", name, temperature);
      _triggerRefresh();
    });
  }

  _systemTemp->listen([this](const char* name, float temperature) {
    Mycila::Logger.info(TAG, "Temperature '%s' changed to %.02f °C", name, temperature);
    _triggerRefresh();
  });
}

void YaSolR::Controller::loop() {
  if (!_events.empty()) {
    // take last event
    YaSolR::Event event = _events.front();
    _events.pop();

    // discard same events
    while (!_events.empty() && _events.front() == event)
      _events.pop();

    switch (event) {
      case YaSolR::Event::MQTT_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring MQTT...");
        Mycila::MQTT.end();
        Mycila::MQTT.begin();
        break;
      }
      case YaSolR::Event::NTP_RECONFIGURED: {
        Mycila::NTP.setTimeZone(Mycila::Config.get(KEY_NTP_TIMEZONE));
        if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE)) {
          Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
          _triggerRefresh();
        }
        break;
      }
      case YaSolR::Event::RELAY1_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Relay 1...");
        Mycila::RelayManager.relays[0].end();
        Mycila::RelayManager.relays[0].begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::RELAY2_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Relay 2...");
        Mycila::RelayManager.relays[1].end();
        Mycila::RelayManager.relays[1].begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::BUZZER_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Buzzer...");
        Mycila::Buzzer.end();
        Mycila::Buzzer.begin();
        if (Mycila::Buzzer.isEnabled())
          Mycila::Buzzer.beep();
        _triggerRefresh();
        break;
      }

      case YaSolR::Event::BUTTON_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Button...");
        Mycila::Button.end();
        Mycila::Button.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::LIGHTS_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring LEDs...");
        Mycila::LightState g = Mycila::Lights.getGreen();
        Mycila::LightState y = Mycila::Lights.getYellow();
        Mycila::LightState r = Mycila::Lights.getRed();
        Mycila::Lights.end();
        Mycila::Lights.begin(g, y, r);
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::SYS_TEMP_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring System Temperature Sensor...");
        _systemTemp->end();
        _systemTemp->begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::OUT1_RELAY_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Output 1 Bypass '%s'...", Mycila::Router.outputs[0].relay.name);
        Mycila::Router.outputs[0].relay.end();
        Mycila::Router.outputs[0].relay.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::OUT1_TEMP_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Temperature Sensor '%s'...", Mycila::Router.outputs[0].temperatureSensor.name);
        Mycila::Router.outputs[0].temperatureSensor.end();
        Mycila::Router.outputs[0].temperatureSensor.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::OUT1_DIMMER_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Dimmer '%s'...", Mycila::Router.outputs[0].dimmer.name);
        Mycila::Router.outputs[0].dimmer.end();
        Mycila::Router.outputs[0].dimmer.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::OUT2_RELAY_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Output 1 Bypass '%s'...", Mycila::Router.outputs[1].relay.name);
        Mycila::Router.outputs[1].relay.end();
        Mycila::Router.outputs[1].relay.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::OUT2_TEMP_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Temperature Sensor '%s'...", Mycila::Router.outputs[1].temperatureSensor.name);
        Mycila::Router.outputs[1].temperatureSensor.end();
        Mycila::Router.outputs[1].temperatureSensor.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::OUT2_DIMMER_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Dimmer '%s'...", Mycila::Router.outputs[1].dimmer.name);
        Mycila::Router.outputs[1].dimmer.end();
        Mycila::Router.outputs[1].dimmer.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::JSY_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring JSY...");
        Mycila::JSY.end();
        if (Mycila::Config.getBool(KEY_JSY_ENABLE)) {
          Mycila::JSY.begin(Mycila::Config.get(KEY_JSY_RX_PIN).toInt(), Mycila::Config.get(KEY_JSY_TX_PIN).toInt(), &Serial2, true);
          if (Mycila::JSY.isEnabled() && Mycila::JSY.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400) {
            Mycila::Logger.info(TAG, "Trying to update JSY Baud Rate to 38400...");
            if (!Mycila::JSY.updateBaudRate(Mycila::JSYBaudRate::BAUD_38400)) {
              Mycila::Logger.warn(TAG, "Failed to update JSY Baud Rate to 38400");
            }
          }
        } else {
          Mycila::Logger.warn(TAG, "JSY not enabled");
        }
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::ZCD_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring ZCD...");
        Mycila::ZCD.end();
        Mycila::ZCD.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::POWER_MANAGER_RECONFIGURED: {
        Mycila::Logger.info(TAG, "Reconfiguring Grid...");
        Mycila::Grid.end();
        Mycila::Grid.begin();
        _triggerRefresh();
        break;
      }
      case YaSolR::Event::NETWORK_RECONFIGURED:
        Mycila::Logger.info(TAG, "Reconfiguring Network...");
        Mycila::MQTT.end();
        Mycila::HTTPd.end();
        Mycila::OTA.end();
        ESPConnect.end();

        Mycila::Logger.info(TAG, "Enable Network...");
        ESPConnect.setAutoRestart(false);
        ESPConnect.setBlocking(false);
        ESPConnect.setCaptivePortalTimeout(Mycila::Config.get(KEY_CAPTURE_PORTAL_TIMEOUT).toInt());
        ESPConnect.setWiFiConnectTimeout(Mycila::Config.get(KEY_WIFI_CONNECTION_TIMEOUT).toInt());
        ESPConnect.begin(&Mycila::HTTPd.server, Mycila::Config.get(KEY_HOSTNAME), Mycila::AppInfo.name + "-" + Mycila::AppInfo.id, Mycila::Config.get(KEY_ADMIN_PASSWORD), {Mycila::Config.get(KEY_WIFI_SSID), Mycila::Config.get(KEY_WIFI_PASSWORD), Mycila::Config.getBool(KEY_AP_MODE_ENABLE)});

        _triggerRefresh();
        break;

      case YaSolR::Event::NETWORK_CONNECTED:
        Mycila::OTA.begin();
        Mycila::HTTPd.begin();
        Mycila::MQTT.begin();
        if (!Mycila::Config.getBool(KEY_AP_MODE_ENABLE))
          Mycila::NTP.sync(Mycila::Config.get(KEY_NTP_SERVER));
        _triggerRefresh();
        break;

      case YaSolR::Event::NETWORK_DISCONNECTED:
        Mycila::MQTT.end();
        Mycila::HTTPd.end();
        Mycila::OTA.end();
        Mycila::Buzzer.beep(2);
        break;

      case YaSolR::Event::DISPLAY_RECONFIGURED:
        Mycila::Display.end();
        Mycila::Display.begin();
        updateDisplay();
        break;

      default:
        break;
    }
  }

  if (millis() - _lastUpdate >= 500) {
    Mycila::Lights.setYellow(Mycila::RelayManager.relays[0].isOn() ||
                             Mycila::RelayManager.relays[1].isOn() ||
                             Mycila::Router.outputs[0].relay.isOn() ||
                             Mycila::Router.outputs[1].relay.isOn() ||
                             Mycila::Router.outputs[0].dimmer.isOn() ||
                             Mycila::Router.outputs[1].dimmer.isOn());

    updateDisplay();
  }

  if (_updateOnceThrottle.isAllowedMillis(500)) {
    _updateOnceThrottle.setEnable(false);
    _website->update();
    _connector->publish();
  }

  if (_restartThrottle.isAllowedMillis(500)) {
    _restartThrottle.setEnable(false);
    Mycila::System.restart();
  }

  if (_resetThrottle.isAllowedMillis(500)) {
    _resetThrottle.setEnable(false);
    Mycila::System.reset();
  }
}

void YaSolR::Controller::_triggerRefresh() {
  _updateOnceThrottle.setEnable(true);
}

/*
| **LINE** | **CONTENT**                 |
| -------: | :-------------------------- |
|        1 | `YaSolR Pro v1.2.3   17:30` |
|        1 | `YaSolR Pro v1.2.3   ??:??` |
|        2 | `192.168.100.100   12.10 C` |
|        2 | `YaSolR-XXXX       12.10 C` |
|        2 | `Disconnected!     12.10 C` |
|        2 | `Connecting...     12.10 C` |
|        2 | `Reconnecting...   12.10 C` |
|        3 | `O1: 1000 W        12.10 C` |
|        3 | `O1: 1000 W        ??.?? C` |
|        3 | `O1: RELAY OFF     12.10 C` |
|        4 | `O1: 12:22-12:23   12-20 C` |
|        4 | `O1: AUTO OFF`              |
|        5 | `O2: 1000 W        12.10 C` |
|        6 | `O2: 12:22-12:23   12-20 C` |
|      ALT | `RELAY1:  ON   RELAY2:  ON` |
|      ALT | `RELAY1: OFF   RELAY2: OFF` |
*/
void YaSolR::Controller::updateDisplay() {
  if (Mycila::Display.isEnabled()) {
    // const uint32_t start = millis();

    char lines[MYCILA_DISPLAY_LINE_COUNT][MYCILA_DISPLAY_LINE_LENGTH];
    bzero(lines, sizeof(lines));

    int next = 0;

    struct tm timeInfo;
    if (Mycila::NTP.isSynced() && getLocalTime(&timeInfo, 5))
      snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "%-6.6s %-3.3s %-8.8s %02u:%02u", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str(), timeInfo.tm_hour, timeInfo.tm_min);
    else
      snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "%-6.6s %-3.3s %-8.8s ??:??", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
    next++;

    String networkState;
    switch (ESPConnect.getState()) {
      case ESPConnectState::STA_CONNECTED:
        networkState = ESPConnect.getIPAddress().toString();
        break;
      case ESPConnectState::STA_DISCONNECTED:
        networkState = "Disconnected!";
        break;
      case ESPConnectState::STA_CONNECTING:
        networkState = "Connecting...";
        break;
      case ESPConnectState::STA_RECONNECTING:
        networkState = "Reconnecting...";
        break;
      case ESPConnectState::STA_TIMEOUT:
        networkState = "Connect Timeout!";
        break;
      case ESPConnectState::AP_CONNECTED:
      case ESPConnectState::PORTAL_STARTED:
        networkState = ESPConnect.getWiFiSSID();
        break;
      default:
        networkState = "Starting...";
        break;
    }
    snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "%-17.17s %-5.5s C", networkState.c_str(), _systemTemp->getTemperatureAsString().c_str());
    next++;

    snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "G: %5d W      R: %4d W", static_cast<int>(round(Mycila::Grid.getPower())), static_cast<int>(round(Mycila::Router.getTotalRoutedPower())));
    next++;

    for (size_t i = 0; i < 2; i++) {
      const auto& output = Mycila::Router.outputs[i];
      snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "O%u: %-13.13s %-5.5s C", (i + 1), output.getStateString(), output.temperatureSensor.getTemperatureAsString().c_str());
      next++;
    }

    snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "Relay1: %-3.3s   Relay2: %-3.3s", Mycila::RelayManager.relays[0].isOn() ? "ON" : "OFF", Mycila::RelayManager.relays[1].isOn() ? "ON" : "OFF");
    next++;

    for (; next < 6; next++)
      lines[next][0] = '\0';

    Mycila::Display.print(lines);

    // Mycila::Logger.debug("DISPLAY", "Updated in %u ms", millis() - start);
  }
}
