// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task displayTask("Display", [](void* params) {
  void* data = displayTask.getData();
  DisplayKind kind = data == nullptr ? DisplayKind::DISPLAY_HOME : (DisplayKind) reinterpret_cast<int>(data);

  display.home.clear();
  display.home.printf("%-6.6s %-3.3s %-7.7s #%d\n", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str(), static_cast<int>(kind));

  switch (kind) {
    case DisplayKind::DISPLAY_NETWORK: {
      switch (espConnect.getState()) {
        case Mycila::ESPConnect::State::AP_STARTING:
          display.home.printf("Starting Access Point\n");
          break;
        case Mycila::ESPConnect::State::AP_STARTED:
          display.home.printf("AP: %s17.17s\n", espConnect.getWiFiSSID().c_str());
          break;
        case Mycila::ESPConnect::State::NETWORK_CONNECTING:
          display.home.printf("Connecting...\n");
          break;
        case Mycila::ESPConnect::State::NETWORK_CONNECTED:
          display.home.printf("SSID: %15.15s\n", espConnect.getWiFiSSID().c_str());
          break;
        case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
          display.home.printf("Unable to connect!\n");
          break;
        case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
          display.home.printf("Disconnected!\n");
          break;
        case Mycila::ESPConnect::State::NETWORK_RECONNECTING:
          display.home.printf("Reconnecting...\n");
          break;
        case Mycila::ESPConnect::State::PORTAL_STARTING:
          display.home.printf("Starting Portal...\n");
          break;
        case Mycila::ESPConnect::State::PORTAL_STARTED:
          display.home.printf("Portal: %13.13s\n", espConnect.getWiFiSSID().c_str());
          break;
        case Mycila::ESPConnect::State::PORTAL_COMPLETE:
          display.home.printf("Network configured!\n");
          break;
        case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
          display.home.printf("Portal timeout\n");
          break;
        default:
          display.home.printf("Unknown network state\n");
          break;
      }
      display.home.printf("IP:   %15.15s\n", espConnect.getIPAddress().toString().c_str());
      display.home.printf("Host: %15.15s\n", Mycila::AppInfo.defaultHostname.c_str());
      display.home.printf("MAC %s\n", espConnect.getMACAddress().c_str());
      break;
    }

    case DisplayKind::DISPLAY_ROUTER: {
      Mycila::Router::Metrics routerMetrics;
      router.getRouterMeasurements(routerMetrics);
      display.home.printf("Grid   Power: %5d W\n", static_cast<int>(round(grid.getPower().orElse(0))));
      display.home.printf("Routed Power: %5d W\n", static_cast<int>(round(routerMetrics.power)));
      if (config.getLong(KEY_RELAY1_LOAD))
        display.home.printf(relay1.isOn() ? "Relay 1: on  %6ld W\n" : "Relay 1: off %6ld W\n", config.getLong(KEY_RELAY1_LOAD));
      else
        display.home.printf(relay1.isOn() ? "Relay 1: on\n" : "Relay 1: off\n");
      if (config.getLong(KEY_RELAY2_LOAD))
        display.home.printf(relay2.isOn() ? "Relay 2: on  %6ld W\n" : "Relay 2: off %6ld W\n", config.getLong(KEY_RELAY2_LOAD));
      else
        display.home.printf(relay2.isOn() ? "Relay 2: on\n" : "Relay 2: off\n");
      break;
    }

    case DisplayKind::DISPLAY_OUTPUT1: {
      display.home.printf("Output 1: %11.11s\n", output1.getStateName());
      display.home.printf("Resistance: %4d Ohms\n", static_cast<int>(round(output1.config.calibratedResistance)));
      display.home.printf("Dimmer: %3d %%  %4d W\n", static_cast<int>(round(output1.getDimmerDutyCycleLive() * 100)), static_cast<int>(round(output1.getOutputPower())));
      if (output1.temperature())
        display.home.printf("Temperature:  %4.1f ", output1.temperature().get());
      else
        display.home.printf("Temperature:  --.- ");
      display.home.printf("\xb0");
      display.home.printf("C\n");
      break;
    }

    case DisplayKind::DISPLAY_OUTPUT2: {
      display.home.printf("Output 2: %11.11s\n", output2.getStateName());
      display.home.printf("Resistance: %4d Ohms\n", static_cast<int>(round(output2.config.calibratedResistance)));
      display.home.printf("Dimmer: %3d %%  %4d W\n", static_cast<int>(round(output2.getDimmerDutyCycleLive() * 100)), static_cast<int>(round(output2.getOutputPower())));
      if (output2.temperature())
        display.home.printf("Temperature:  %4.1f ", output2.temperature().get());
      else
        display.home.printf("Temperature:  --.- ");
      display.home.printf("\xb0");
      display.home.printf("C\n");
      break;
    }

    default: {
      struct tm timeInfo;
      display.home.printf("Restarts: %11" PRIu32 "\n", Mycila::System::getBootCount());
      display.home.printf("Uptime: %13.13s\n", Mycila::Time::toDHHMMSS(Mycila::System::getUptime()).c_str());
      if (Mycila::NTP.isSynced() && getLocalTime(&timeInfo, 5))
        display.home.printf("NTP Time:       %02u:%02u\n", timeInfo.tm_hour, timeInfo.tm_min);
      else
        display.home.printf("NTP Time:       --:--\n");
      if (ds18Sys.isEnabled())
        display.home.printf("Temperature:  %4.1f ", ds18Sys.getTemperature().value_or(0));
      else
        display.home.printf("Temperature:  --.- ");
      display.home.printf("\xb0");
      display.home.printf("C\n");
      break;
    }
  }

  display.display();
});
