// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

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
void YaSolR::YaSolRClass::updateDisplay() {
  if (Mycila::EasyDisplay.isEnabled()) {
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
      case ESPConnectState::NETWORK_CONNECTED:
        networkState = ESPConnect.getIPAddress().toString();
        break;
      case ESPConnectState::NETWORK_DISCONNECTED:
        networkState = "Disconnected!";
        break;
      case ESPConnectState::NETWORK_CONNECTING:
        networkState = "Connecting...";
        break;
      case ESPConnectState::NETWORK_RECONNECTING:
        networkState = "Reconnecting...";
        break;
      case ESPConnectState::NETWORK_TIMEOUT:
        networkState = "Connect Timeout!";
        break;
      case ESPConnectState::AP_STARTED:
      case ESPConnectState::PORTAL_STARTED:
        networkState = ESPConnect.getWiFiSSID();
        break;
      default:
        networkState = "Starting...";
        break;
    }
    snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "%-17.17s %-5.5s C", networkState.c_str(), systemTemperatureSensor.getTemperatureAsString().c_str());
    next++;

    snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "G: %5d W      R: %4d W", static_cast<int>(round(Mycila::Grid.getActivePower())), static_cast<int>(round(Mycila::Router.getTotalRoutedPower())));
    next++;

    const Mycila::RouterOutput outputs[2] = {output1, output2};
    for (size_t i = 0; i < 2; i++) {
      snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "O%u: %-13.13s %-5.5s C", (i + 1), outputs[i].getStateString(), outputs[i].getTemperatureSensor()->getTemperatureAsString().c_str());
      next++;
    }

    snprintf(lines[next], MYCILA_DISPLAY_LINE_LENGTH, "Relay1: %-3.3s   Relay2: %-3.3s", relay1.isOn() ? "ON" : "OFF", relay2.isOn() ? "ON" : "OFF");
    next++;

    for (; next < 6; next++)
      lines[next][0] = '\0';

    Mycila::EasyDisplay.print(lines);
  }
}
