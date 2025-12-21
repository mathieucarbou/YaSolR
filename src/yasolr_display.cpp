// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <string>

#define YASOLR_DISPLAY_LINES     5
#define YASOLR_DISPLAY_LINE_SIZE 21

Mycila::EasyDisplay* display = nullptr;

static Mycila::Task* displayTask = nullptr;
static uint8_t startingInformation = 1;
static uint32_t lastDisplayUpdate = 0;

void yasolr_configure_display() {
  if (config.get<bool>(KEY_ENABLE_DISPLAY)) {
    if (display == nullptr) {
      ESP_LOGI(TAG, "Enable display");

      display = new Mycila::EasyDisplay(YASOLR_DISPLAY_LINES, YASOLR_DISPLAY_LINE_SIZE, 4, u8g2_font_6x12_tf);

      const char* displayType = config.getString(KEY_DISPLAY_TYPE);
      if (strcmp(displayType, "SSD1306") == 0)
        display->begin(Mycila::EasyDisplayType::SSD1306, config.get<int8_t>(KEY_PIN_I2C_SCL), config.get<int8_t>(KEY_PIN_I2C_SDA), config.get<uint16_t>(KEY_DISPLAY_ROTATION));
      else if (strcmp(displayType, "SH1107") == 0)
        display->begin(Mycila::EasyDisplayType::SH1107, config.get<int8_t>(KEY_PIN_I2C_SCL), config.get<int8_t>(KEY_PIN_I2C_SDA), config.get<uint16_t>(KEY_DISPLAY_ROTATION));
      else if (strcmp(displayType, "SH1106") == 0)
        display->begin(Mycila::EasyDisplayType::SH1106, config.get<int8_t>(KEY_PIN_I2C_SCL), config.get<int8_t>(KEY_PIN_I2C_SDA), config.get<uint16_t>(KEY_DISPLAY_ROTATION));
      if (!display->isEnabled()) {
        ESP_LOGE(TAG, "Display failed to initialize!");
        display->end();
        delete display;
        display = nullptr;
        return;
      }

      display->setActive(true);

      displayTask = new Mycila::Task("Display", []() {
        if (lastDisplayUpdate && millis() - lastDisplayUpdate < config.get<uint8_t>(KEY_DISPLAY_SPEED) * 1000)
          return;

        // Serial.printf("clear()\n");
        display->home.clear();

        for (uint8_t info = startingInformation, lines = 0; lines < YASOLR_DISPLAY_LINES; info++) {
          bool wrote = false;

          switch (info) {
            case 1: {
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
            case 2: {
              display->home.printf("Grid   P: %9d W", static_cast<int>(std::round(grid.getPower().value_or(0))));
              wrote = true;
              break;
            }
            case 3: {
              std::optional<float> routedPower = router.readTotalRoutedPower();
              if (!routedPower.has_value()) {
                routedPower = router.computeTotalRoutedPower(grid.getVoltage().value_or(NAN));
              }
              display->home.printf("Router P: %9d W", static_cast<int>(std::round(routedPower.value_or(0))));
              wrote = true;
              break;
            }
            case 4: {
              if (ds18Sys && ds18Sys->getTemperature().has_value()) {
                display->home.printf("Router T: %8.1f ", ds18Sys->getTemperature().value());
                display->home.printf("\xb0");
                display->home.printf("C");
                wrote = true;
              }
              break;
            }
            case 5: {
              if (output1.getState() != Mycila::Router::Output::State::UNUSED) {
                display->home.printf("Output 1: %11.11s", output1.getStateName());
                wrote = true;
              }
              break;
            }
            case 6: {
              if (output1.getState() != Mycila::Router::Output::State::UNUSED) {
                float duty = output1.getDimmerDutyCycleOnline();
                if (duty > 0) {
                  display->home.printf("Output 1 Duty: %4d %%", static_cast<int>(std::round(output1.getDimmerDutyCycleOnline() * 100.0f)));
                  wrote = true;
                }
              }
              break;
            }
            case 7: {
              if (output1.temperature().isPresent()) {
                display->home.printf("Output 1 T: %6.1f ", output1.temperature().get());
                display->home.printf("\xb0");
                display->home.printf("C");
                wrote = true;
              }
              break;
            }
            case 8: {
              if (output2.getState() != Mycila::Router::Output::State::UNUSED) {
                display->home.printf("Output 2: %11.11s", output2.getStateName());
                wrote = true;
              }
              break;
            }
            case 9: {
              if (output2.getState() != Mycila::Router::Output::State::UNUSED) {
                float duty = output2.getDimmerDutyCycleOnline();
                if (duty > 0) {
                  display->home.printf("Output 2 Duty: %4d %%", static_cast<int>(std::round(output2.getDimmerDutyCycleOnline() * 100.0f)));
                  wrote = true;
                }
              }
              break;
            }
            case 10: {
              if (output2.temperature().isPresent()) {
                display->home.printf("Output 2 T: %6.1f ", output2.temperature().get());
                display->home.printf("\xb0");
                display->home.printf("C");
                wrote = true;
              }
              break;
            }
            case 11: {
              if (relay1 && relay1->isEnabled()) {
                display->home.printf("Relay 1: %12s", YASOLR_STATE(relay1->isOn()));
                wrote = true;
              }
              break;
            }
            case 12: {
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
      if (config.get<bool>(KEY_ENABLE_DEBUG))
        displayTask->enableProfiling();

      coreTaskManager.addTask(*displayTask);
    }

  } else {
    if (display != nullptr) {
      ESP_LOGI(TAG, "Disable display");
      displayTask->pause();
      display->end();
      coreTaskManager.removeTask(*displayTask);
      delete displayTask;
      delete display;
      displayTask = nullptr;
      display = nullptr;
      startingInformation = 1;
      lastDisplayUpdate = 0;
    }
  }
}
