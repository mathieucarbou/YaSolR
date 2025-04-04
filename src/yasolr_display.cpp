// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <string>

#define YASOLR_DISPLAY_LINES     5
#define YASOLR_DISPLAY_LINE_SIZE 21

Mycila::EasyDisplay* display = nullptr;

static uint8_t startingInformation = 1;
static uint32_t lastDisplayUpdate = 0;

void yasolr_init_display() {
  if (config.getBool(KEY_ENABLE_DISPLAY)) {
    logger.info(TAG, "Initialize display");

    display = new Mycila::EasyDisplay(YASOLR_DISPLAY_LINES, YASOLR_DISPLAY_LINE_SIZE, 4, u8g2_font_6x12_tf);

    const std::string& displayType = config.getString(KEY_DISPLAY_TYPE);
    if (displayType == "SSD1306")
      display->begin(Mycila::EasyDisplayType::SSD1306, config.getLong(KEY_PIN_I2C_SCL), config.getLong(KEY_PIN_I2C_SDA), config.getLong(KEY_DISPLAY_ROTATION));
    else if (displayType == "SH1107")
      display->begin(Mycila::EasyDisplayType::SH1107, config.getLong(KEY_PIN_I2C_SCL), config.getLong(KEY_PIN_I2C_SDA), config.getLong(KEY_DISPLAY_ROTATION));
    else if (displayType == "SH1106")
      display->begin(Mycila::EasyDisplayType::SH1106, config.getLong(KEY_PIN_I2C_SCL), config.getLong(KEY_PIN_I2C_SDA), config.getLong(KEY_DISPLAY_ROTATION));

    if (!display->isEnabled()) {
      logger.error(TAG, "Display failed to initialize!");
      display->end();
      delete display;
      display = nullptr;
      return;
    }

    display->setActive(true);

    Mycila::Task* displayTask = new Mycila::Task("Display", [](void* params) {
      if (lastDisplayUpdate && millis() - lastDisplayUpdate < config.getLong(KEY_DISPLAY_SPEED) * 1000)
        return;

      // Serial.printf("clear()\n");
      display->home.clear();

      for (uint8_t info = startingInformation, lines = 0; lines < YASOLR_DISPLAY_LINES; info++) {
        bool wrote = false;

        switch (info) {
          case 1: {
            display->home.printf("%-6.6s %-3.3s %-10.10s", Mycila::AppInfo.name.c_str(), Mycila::AppInfo.model.c_str(), Mycila::AppInfo.version.c_str());
            wrote = true;
            break;
          }
          case 2: {
            display->home.printf("Uptime: %13.13s", Mycila::Time::toDHHMMSS(Mycila::System::getUptime()).c_str());
            wrote = true;
            break;
          }
          case 3: {
            struct tm timeInfo;
            if (Mycila::NTP.isSynced() && getLocalTime(&timeInfo, 5)) {
              display->home.printf("Time:        %02u:%02u:%02u", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
              wrote = true;
            }
            break;
          }
          case 4: {
            switch (espConnect.getState()) {
              case Mycila::ESPConnect::State::AP_STARTING:
                display->home.printf("Starting Access Point");
                break;
              case Mycila::ESPConnect::State::AP_STARTED:
                display->home.printf("AP: %s17.17s", espConnect.getWiFiSSID().c_str());
                break;
              case Mycila::ESPConnect::State::NETWORK_CONNECTING:
                display->home.printf("Connecting...");
                break;
              case Mycila::ESPConnect::State::NETWORK_CONNECTED:
                display->home.printf("IP:   %15.15s", espConnect.getIPAddress().toString().c_str());
                break;
              case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
                display->home.printf("Unable to connect!");
                break;
              case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
                display->home.printf("Disconnected!");
                break;
              case Mycila::ESPConnect::State::NETWORK_RECONNECTING:
                display->home.printf("Reconnecting...");
                break;
              case Mycila::ESPConnect::State::PORTAL_STARTING:
                display->home.printf("Starting Portal...");
                break;
              case Mycila::ESPConnect::State::PORTAL_STARTED:
                display->home.printf("Portal: %13.13s", espConnect.getWiFiSSID().c_str());
                break;
              case Mycila::ESPConnect::State::PORTAL_COMPLETE:
                display->home.printf("Network configured!");
                break;
              case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
                display->home.printf("Portal timeout");
                break;
              default:
                display->home.printf("Unknown network state");
                break;
            }
            wrote = true;
            break;
          }
          case 5: {
            display->home.printf("Grid   P: %9d W", static_cast<int>(std::round(grid.getPower().value_or(0))));
            wrote = true;
            break;
          }
          case 6: {
            Mycila::Router::Metrics routerMetrics;
            router.getRouterMeasurements(routerMetrics);
            display->home.printf("Router P: %9d W", static_cast<int>(std::round(routerMetrics.power)));
            wrote = true;
            break;
          }
          case 7: {
            if (ds18Sys && ds18Sys->getTemperature()) {
              display->home.printf("Router T: %8.1f ", ds18Sys->getTemperature().value());
              display->home.printf("\xb0");
              display->home.printf("C");
              wrote = true;
            }
            break;
          }
          case 8: {
            if (output1) {
              display->home.printf("Output 1: %11.11s", output1->getStateName());
              wrote = true;
            }
            break;
          }
          case 9: {
            if (output1 && output1->isDimmerEnabled()) {
              display->home.printf("Output 1 Duty: %4d %%", static_cast<int>(std::round(output1->getDimmerDutyCycleLive() * 100.0f)));
              wrote = true;
            }
            break;
          }
          case 10: {
            if (output1 && output1->temperature()) {
              display->home.printf("Output 1 T: %6.1f ", output1->temperature().get());
              display->home.printf("\xb0");
              display->home.printf("C");
              wrote = true;
            }
            break;
          }
          case 11: {
            if (output2) {
              display->home.printf("Output 2: %11.11s", output2->getStateName());
              wrote = true;
            }
            break;
          }
          case 12: {
            if (output2 && output2->isDimmerEnabled()) {
              display->home.printf("Output 2 Duty: %4d %%", static_cast<int>(std::round(output2->getDimmerDutyCycleLive() * 100.0f)));
              wrote = true;
            }
            break;
          }
          case 13: {
            if (output2 && output2->temperature()) {
              display->home.printf("Output 2 T: %6.1f ", output2->temperature().get());
              display->home.printf("\xb0");
              display->home.printf("C");
              wrote = true;
            }
            break;
          }
          case 14: {
            if (relay1 && relay1->isEnabled()) {
              display->home.printf("Relay 1: %12s", YASOLR_STATE(relay1->isOn()));
              wrote = true;
            }
            break;
          }
          case 15: {
            if (relay2 && relay2->isEnabled()) {
              display->home.printf("Relay 2: %12s", YASOLR_STATE(relay2->isOn()));
              wrote = true;
            }
            break;
          }
          default:
            display->home.print("---------------------");
            wrote = true;
            info = 0;
            break;
        }

        if (wrote) {
          if (!lines++)
            startingInformation = info + 1;
          if (lines < YASOLR_DISPLAY_LINES)
            display->home.println();
        }

        // Serial.printf("lines: %d, info: %d, startingInformation: %d\n", lines, info, startingInformation);
      }

      display->display();
      lastDisplayUpdate = millis();
    });

    displayTask->setInterval(1000);

    coreTaskManager.addTask(*displayTask);
  }
}
