// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

typedef enum {
  DISPLAY_HOME = 1,
  DISPLAY_NETWORK,
  DISPLAY_ROUTER,
  DISPLAY_OUTPUT1,
  DISPLAY_OUTPUT2,
} DisplayKind;

Mycila::Task carouselTask("Carousel", [](void* params) {
  void* data = displayTask.getData();
  if (data == nullptr) {
    displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_HOME));
  } else {
    switch ((DisplayKind) reinterpret_cast<int>(data)) {
      case DisplayKind::DISPLAY_HOME:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_NETWORK));
        break;
      case DisplayKind::DISPLAY_NETWORK:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_ROUTER));
        break;
      case DisplayKind::DISPLAY_ROUTER:
        if (config.getBool(KEY_ENABLE_OUTPUT1_DIMMER)) {
          displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_OUTPUT1));
          break;
        } else {
          [[fallthrough]];
        }
      case DisplayKind::DISPLAY_OUTPUT1:
        if (config.getBool(KEY_ENABLE_OUTPUT2_DIMMER)) {
          displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_OUTPUT2));
          break;
        } else {
          [[fallthrough]];
        }
      case DisplayKind::DISPLAY_OUTPUT2:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_HOME));
        break;
      default:
        displayTask.setData(reinterpret_cast<void*>(DisplayKind::DISPLAY_HOME));
        break;
    }
  }
});

Mycila::Task displayTask("Display", [](void* params) {
  void* data = displayTask.getData();
  DisplayKind kind = data == nullptr ? DisplayKind::DISPLAY_HOME : (DisplayKind) reinterpret_cast<int>(data);

  display.home.clear();
  display.home.printf("%-6.6s %-3.3s %-7.7s #%d\n", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str(), kind);

  switch (kind) {
    case DisplayKind::DISPLAY_NETWORK: {
      switch (ESPConnect.getState()) {
        case ESPConnectState::AP_STARTING:
          display.home.printf("Starting Access Point\n");
          break;
        case ESPConnectState::AP_STARTED:
          display.home.printf("AP: %s17.17s\n", ESPConnect.getWiFiSSID().c_str());
          break;
        case ESPConnectState::NETWORK_CONNECTING:
          display.home.printf("Connecting...\n");
          break;
        case ESPConnectState::NETWORK_CONNECTED:
          display.home.printf("SSID: %15.15s\n", ESPConnect.getWiFiSSID().c_str());
          break;
        case ESPConnectState::NETWORK_TIMEOUT:
          display.home.printf("Unable to connect!\n");
          break;
        case ESPConnectState::NETWORK_DISCONNECTED:
          display.home.printf("Disconnected!\n");
          break;
        case ESPConnectState::NETWORK_RECONNECTING:
          display.home.printf("Reconnecting...\n");
          break;
        case ESPConnectState::PORTAL_STARTING:
          display.home.printf("Starting Portal...\n");
          break;
        case ESPConnectState::PORTAL_STARTED:
          display.home.printf("Portal: %13.13s\n", ESPConnect.getWiFiSSID().c_str());
          break;
        case ESPConnectState::PORTAL_COMPLETE:
          display.home.printf("Network configured!\n");
          break;
        case ESPConnectState::PORTAL_TIMEOUT:
          display.home.printf("Portal timeout\n");
          break;
        default:
          display.home.printf("Unknown network state\n");
          break;
      }
      display.home.printf("IP:   %15.15s\n", ESPConnect.getIPAddress().toString().c_str());
      display.home.printf("Host: %15.15s\n", Mycila::AppInfo.defaultHostname.c_str());
      display.home.printf("MAC %s\n", ESPConnect.getMACAddress().c_str());
      break;
    }

    case DisplayKind::DISPLAY_ROUTER: {
      Mycila::GridMetrics gridMetrics;
      grid.getMetrics(gridMetrics);
      Mycila::RouterMetrics routerMetrics;
      router.getMetrics(routerMetrics);
      display.home.printf("Grid   Power: %5d W\n", static_cast<int>(round(gridMetrics.power)));
      if (gridMetrics.connected)
        display.home.printf("Routed Power: %5d W\n", static_cast<int>(round(routerMetrics.power)));
      else
        display.home.printf("Routed Power:   ERROR\n");
      if (config.get(KEY_RELAY1_LOAD).toInt())
        display.home.printf(relay1.isOn() ? "Relay 1: on  %6ld W\n" : "Relay 1: off %6ld W\n", config.get(KEY_RELAY1_LOAD).toInt());
      else
        display.home.printf(relay1.isOn() ? "Relay 1: on\n" : "Relay 1: off\n");
      if (config.get(KEY_RELAY2_LOAD).toInt())
        display.home.printf(relay2.isOn() ? "Relay 2: on  %6ld W\n" : "Relay 2: off %6ld W\n", config.get(KEY_RELAY2_LOAD).toInt());
      else
        display.home.printf(relay2.isOn() ? "Relay 2: on\n" : "Relay 2: off\n");
      break;
    }

    case DisplayKind::DISPLAY_OUTPUT1: {
      Mycila::RouterOutputMetrics outputMetrics;
      output1.getMetrics(outputMetrics);
      if (outputMetrics.connected)
        display.home.printf("Output 1: %11.11s\n", output1.getStateName());
      else
        display.home.printf("Output 1:  GRID ERROR\n");
      display.home.printf("Resistance: %4d Ohms\n", static_cast<int>(round(outputMetrics.resistance)));
      display.home.printf("Dimmer: %5d %5d W\n", dimmerO1.getLevel(), static_cast<int>(round(outputMetrics.power)));
      if (ds18O1.isEnabled())
        display.home.printf("Temperature:   %4.1f", ds18O1.getLastTemperature());
      else
        display.home.printf("Temperature:   --.-");
      display.home.printf("\xb0");
      display.home.printf("C\n");
      break;
    }

    case DisplayKind::DISPLAY_OUTPUT2: {
      Mycila::RouterOutputMetrics outputMetrics;
      output2.getMetrics(outputMetrics);
      if (outputMetrics.connected)
        display.home.printf("Output 2: %11.11s\n", output2.getStateName());
      else
        display.home.printf("Output 2:  GRID ERROR\n");
      display.home.printf("Resistance: %4d Ohms\n", static_cast<int>(round(outputMetrics.resistance)));
      display.home.printf("Dimmer: %5d %5d W\n", dimmerO2.getLevel(), static_cast<int>(round(outputMetrics.power)));
      if (ds18O2.isEnabled())
        display.home.printf("Temperature:   %4.1f", ds18O2.getLastTemperature());
      else
        display.home.printf("Temperature:   --.-");
      display.home.printf("\xb0");
      display.home.printf("C\n");
      break;
    }

    default: {
      struct tm timeInfo;
      display.home.printf("Restarts: %11" PRIu32 "\n", Mycila::System.getBootCount());
      display.home.printf("Uptime: %13.13s\n", Mycila::Time::toDHHMMSS(Mycila::System.getUptime()).c_str());
      if (Mycila::NTP.isSynced() && getLocalTime(&timeInfo, 5))
        display.home.printf("NTP Time:       %02u:%02u\n", timeInfo.tm_hour, timeInfo.tm_min);
      else
        display.home.printf("NTP Time:       --:--\n");
      if (ds18Sys.isEnabled())
        display.home.printf("Temperature:   %4.1f", ds18Sys.getLastTemperature());
      else
        display.home.printf("Temperature:   --.-");
      display.home.printf("\xb0");
      display.home.printf("C\n");
      break;
    }
  }

  display.display();
});
