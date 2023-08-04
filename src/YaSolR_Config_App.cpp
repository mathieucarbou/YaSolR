// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#ifndef YASOLR_RELAY_PAUSE_DURATION
#define YASOLR_RELAY_PAUSE_DURATION 60
#endif

#ifndef YASOLR_DEBUG_ENABLE
#define YASOLR_DEBUG_ENABLE false
#endif

// network

#ifndef YASOLR_AP_MODE_ENABLE
#define YASOLR_AP_MODE_ENABLE false
#endif

// ntp

#ifndef YASOLR_NTP_SERVER
#define YASOLR_NTP_SERVER "pool.ntp.org"
#endif

#ifndef YASOLR_NTP_TIMEZONE
#define YASOLR_NTP_TIMEZONE "Europe/Paris"
#endif

// ota

#ifndef YASOLR_MQTT_ENABLE
#define YASOLR_MQTT_ENABLE false
#endif

// mqtt

#ifndef YASOLR_MQTT_PORT
#define YASOLR_MQTT_PORT 1883
#endif

#ifndef YASOLR_MQTT_USER
#define YASOLR_MQTT_USER "homeassistant"
#endif

#ifndef YASOLR_MQTT_SECURED
#define YASOLR_MQTT_SECURED false
#endif

#ifndef YASOLR_MQTT_PUBLISH_INTERVAL
#define YASOLR_MQTT_PUBLISH_INTERVAL 5
#endif

// ha

#ifndef YASOLR_HA_DISCOVERY_ENABLE
#define YASOLR_HA_DISCOVERY_ENABLE false
#endif

// buzzer

#ifndef YASOLR_BUZZER_ENABLE
#define YASOLR_BUZZER_ENABLE false
#endif

#ifndef YASOLR_BUZZER_PIN
#define YASOLR_BUZZER_PIN 19
#endif

// button

#ifndef YASOLR_BUTTON_ENABLE
#define YASOLR_BUTTON_ENABLE false
#endif

#ifndef YASOLR_BUTTON_PIN
#define YASOLR_BUTTON_PIN 23
#endif

#ifndef YASOLR_BUTTON_ACTION
#define YASOLR_BUTTON_ACTION "restart"
#endif

// lights

#ifndef YASOLR_LIGHTS_ENABLE
#define YASOLR_LIGHTS_ENABLE false
#endif

#ifndef YASOLR_LIGHTS_GREEN_PIN
#define YASOLR_LIGHTS_GREEN_PIN 0
#endif

#ifndef YASOLR_LIGHTS_YELLOW_PIN
#define YASOLR_LIGHTS_YELLOW_PIN 2
#endif

#ifndef YASOLR_LIGHTS_RED_PIN
#define YASOLR_LIGHTS_RED_PIN 15
#endif

#ifndef YASOLR_DISPLAY_ENABLE
#define YASOLR_DISPLAY_ENABLE false
#endif

// display

#ifndef YASOLR_DISPLAY_TYPE
#define YASOLR_DISPLAY_TYPE "SH1106"
#endif

#ifndef YASOLR_DISPLAY_CLOCK_PIN
#define YASOLR_DISPLAY_CLOCK_PIN 22
#endif

#ifndef YASOLR_DISPLAY_DATA_PIN
#define YASOLR_DISPLAY_DATA_PIN 21
#endif

#ifndef YASOLR_DISPLAY_ROTATION
#define YASOLR_DISPLAY_ROTATION 0
#endif

#ifndef YASOLR_DISPLAY_POWER_SAVE_DELAY
#define YASOLR_DISPLAY_POWER_SAVE_DELAY 0
#endif

// system temperature

#ifndef YASOLR_SYSTEM_TEMP_ENABLE
#define YASOLR_SYSTEM_TEMP_ENABLE false
#endif

#ifndef YASOLR_SYSTEM_TEMP__PIN
#define YASOLR_SYSTEM_TEMP_PIN 4
#endif

// relay 1

#ifndef YASOLR_RELAY_ENABLE
#define YASOLR_RELAY_ENABLE false
#endif

#ifndef YASOLR_RELAY1_PIN
#define YASOLR_RELAY1_PIN 13
#endif

#ifndef YASOLR_RELAY_TYPE
#define YASOLR_RELAY_TYPE "NO"
#endif

#ifndef YASOLR_RELAY_POWER
#define YASOLR_RELAY_POWER 0
#endif

// relay 2

#ifndef YASOLR_RELAY2_PIN
#define YASOLR_RELAY2_PIN 12
#endif

// output 1

#ifndef YASOLR_OUTPUT_DIMMER_ENABLE
#define YASOLR_OUTPUT_DIMMER_ENABLE false
#endif

#ifndef YASOLR_OUTPUT1_DIMMER_PIN
#define YASOLR_OUTPUT1_DIMMER_PIN 25
#endif

#ifndef YASOLR_OUTPUT_DIMMER_TYPE
#define YASOLR_OUTPUT_DIMMER_TYPE "TRIAC"
#endif

#ifndef YASOLR_OUTPUT_DIMMER_AUTO
#define YASOLR_OUTPUT_DIMMER_AUTO false
#endif

#ifndef YASOLR_OUTPUT_DIMMER_LEVEL_LIMIT
#define YASOLR_OUTPUT_DIMMER_LEVEL_LIMIT 100
#endif

#ifndef YASOLR_OUTPUT_RELAY_ENABLE
#define YASOLR_OUTPUT_RELAY_ENABLE false
#endif

#ifndef YASOLR_OUTPUT1_RELAY_PIN
#define YASOLR_OUTPUT1_RELAY_PIN 32
#endif

#ifndef YASOLR_OUTPUT_RELAY_TYPE
#define YASOLR_OUTPUT_RELAY_TYPE "NO"
#endif

#ifndef YASOLR_OUTPUT_TEMP_ENABLE
#define YASOLR_OUTPUT_TEMP_ENABLE false
#endif

#ifndef YASOLR_OUTPUT1_TEMP_PIN
#define YASOLR_OUTPUT1_TEMP_PIN 18
#endif

#ifndef YASOLR_OUTPUT_AUTO_BYPASS_ENABLE
#define YASOLR_OUTPUT_AUTO_BYPASS_ENABLE false
#endif

#ifndef YASOLR_OUTPUT_AUTO_START_TEMPERATURE
#define YASOLR_OUTPUT_AUTO_START_TEMPERATURE 50
#endif

#ifndef YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE
#define YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE 60
#endif

#ifndef YASOLR_OUTPUT_AUTO_START_TIME
#define YASOLR_OUTPUT_AUTO_START_TIME "22:00"
#endif

#ifndef YASOLR_OUTPUT_AUTO_STOP_TIME
#define YASOLR_OUTPUT_AUTO_STOP_TIME "06:00"
#endif

#ifndef YASOLR_OUTPUT_AUTO_WEEK_DAYS
#define YASOLR_OUTPUT_AUTO_WEEK_DAYS "sun,mon,tue,wed,thu,fri,sat"
#endif

// output 2

#ifndef YASOLR_OUTPUT2_DIMMER_PIN
#define YASOLR_OUTPUT2_DIMMER_PIN 26
#endif

#ifndef YASOLR_OUTPUT2_RELAY_PIN
#define YASOLR_OUTPUT2_RELAY_PIN 33
#endif

#ifndef YASOLR_OUTPUT2_TEMP_PIN
#define YASOLR_OUTPUT2_TEMP_PIN 5
#endif

// electricity

#ifndef YASOLR_ZCD_ENABLE
#define YASOLR_ZCD_ENABLE false
#endif

#ifndef YASOLR_ZCD_PIN
#define YASOLR_ZCD_PIN 35
#endif

#ifndef YASOLR_GRID_FREQUENCY
#define YASOLR_GRID_FREQUENCY 50
#endif

#ifndef YASOLR_GRID_POWER_MQTT_TOPIC
#define YASOLR_GRID_POWER_MQTT_TOPIC ""
#endif

#ifndef YASOLR_JSY_ENABLE
#define YASOLR_JSY_ENABLE false
#endif

#ifndef YASOLR_JSY_RX_PIN
#define YASOLR_JSY_RX_PIN 17
#endif

#ifndef YASOLR_JSY_TX_PIN
#define YASOLR_JSY_TX_PIN 16
#endif

// lang

#ifndef YASOLR_LANG
#define YASOLR_LANG "en"
#endif

#define TAG "YASOLR"

void YaSolR::YaSolRClass::_initConfig() {
  Mycila::Logger.info(TAG, "Initializing Config System...");

  Mycila::Config.begin(84);

  String mqttName = String(Mycila::AppInfo.name + "_" + Mycila::AppInfo.id);
  mqttName.toLowerCase();

  String hostname = String(Mycila::AppInfo.name + "-" + Mycila::AppInfo.id);
  hostname.toLowerCase();

  /// others
  Mycila::Config.configure(KEY_HOSTNAME, hostname);
  Mycila::Config.configure(KEY_ADMIN_PASSWORD);
  // logger
  Mycila::Config.configure(KEY_DEBUG_ENABLE, YASOLR_BOOL(YASOLR_DEBUG_ENABLE));
  // ap
  Mycila::Config.configure(KEY_AP_MODE_ENABLE, YASOLR_BOOL(YASOLR_AP_MODE_ENABLE));
  /// wifi
  Mycila::Config.configure(KEY_WIFI_SSID);
  Mycila::Config.configure(KEY_WIFI_PASSWORD);
  Mycila::Config.configure(KEY_WIFI_CONNECTION_TIMEOUT, String(ESPCONNECT_CONNECTION_TIMEOUT));
  // portal
  Mycila::Config.configure(KEY_CAPTURE_PORTAL_TIMEOUT, String(ESPCONNECT_PORTAL_TIMEOUT));
  // ntp
  Mycila::Config.configure(KEY_NTP_SERVER, YASOLR_NTP_SERVER);
  Mycila::Config.configure(KEY_NTP_TIMEZONE, YASOLR_NTP_TIMEZONE);
  // mqtt
  Mycila::Config.configure(KEY_MQTT_ENABLE, YASOLR_BOOL(YASOLR_MQTT_ENABLE));
  Mycila::Config.configure(KEY_MQTT_SERVER);
  Mycila::Config.configure(KEY_MQTT_PORT, String(YASOLR_MQTT_PORT));
  Mycila::Config.configure(KEY_MQTT_USERNAME, YASOLR_MQTT_USER);
  Mycila::Config.configure(KEY_MQTT_PASSWORD);
  Mycila::Config.configure(KEY_MQTT_TOPIC, mqttName);
  Mycila::Config.configure(KEY_MQTT_SECURED, YASOLR_BOOL(YASOLR_MQTT_SECURED));
  Mycila::Config.configure(KEY_MQTT_PUBLISH_INTERVAL, String(YASOLR_MQTT_PUBLISH_INTERVAL));
  // ha
  Mycila::Config.configure(KEY_HA_DISCOVERY_ENABLE, YASOLR_BOOL(YASOLR_HA_DISCOVERY_ENABLE));
  Mycila::Config.configure(KEY_HA_DISCOVERY_TOPIC, MYCILA_HA_DISCOVERY_TOPIC);
  // buzzer
  Mycila::Config.configure(KEY_BUZZER_ENABLE, YASOLR_BOOL(YASOLR_BUZZER_ENABLE));
  Mycila::Config.configure(KEY_BUZZER_PIN, String(YASOLR_BUZZER_PIN));
  // button
  Mycila::Config.configure(KEY_BUTTON_ENABLE, YASOLR_BOOL(YASOLR_BUTTON_ENABLE));
  Mycila::Config.configure(KEY_BUTTON_PIN, String(YASOLR_BUTTON_PIN));
  Mycila::Config.configure(KEY_BUTTON_ACTION, String(YASOLR_BUTTON_ACTION));
  // leds
  Mycila::Config.configure(KEY_LIGHTS_ENABLE, YASOLR_BOOL(YASOLR_LIGHTS_ENABLE));
  Mycila::Config.configure(KEY_LIGHTS_GREEN_PIN, String(YASOLR_LIGHTS_GREEN_PIN));
  Mycila::Config.configure(KEY_LIGHTS_YELLOW_PIN, String(YASOLR_LIGHTS_YELLOW_PIN));
  Mycila::Config.configure(KEY_LIGHTS_RED_PIN, String(YASOLR_LIGHTS_RED_PIN));
  // display
  Mycila::Config.configure(KEY_DISPLAY_ENABLE, YASOLR_BOOL(YASOLR_DISPLAY_ENABLE));
  Mycila::Config.configure(KEY_DISPLAY_TYPE, YASOLR_DISPLAY_TYPE);
  Mycila::Config.configure(KEY_DISPLAY_CLOCK_PIN, String(YASOLR_DISPLAY_CLOCK_PIN));
  Mycila::Config.configure(KEY_DISPLAY_DATA_PIN, String(YASOLR_DISPLAY_DATA_PIN));
  Mycila::Config.configure(KEY_DISPLAY_ROTATION, String(YASOLR_DISPLAY_ROTATION));
  Mycila::Config.configure(KEY_DISPLAY_POWER_SAVE_DELAY, String(YASOLR_DISPLAY_POWER_SAVE_DELAY));
  // system temperature
  Mycila::Config.configure(KEY_SYSTEM_TEMP_ENABLE, YASOLR_BOOL(YASOLR_SYSTEM_TEMP_ENABLE));
  Mycila::Config.configure(KEY_SYSTEM_TEMP_PIN, String(YASOLR_SYSTEM_TEMP_PIN));
  // relay 1
  Mycila::Config.configure(KEY_RELAY1_ENABLE, YASOLR_BOOL(YASOLR_RELAY_ENABLE));
  Mycila::Config.configure(KEY_RELAY1_PIN, String(YASOLR_RELAY1_PIN));
  Mycila::Config.configure(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE);
  Mycila::Config.configure(KEY_RELAY1_POWER, String(YASOLR_RELAY_POWER));
  // relay 2
  Mycila::Config.configure(KEY_RELAY2_ENABLE, YASOLR_BOOL(YASOLR_RELAY_ENABLE));
  Mycila::Config.configure(KEY_RELAY2_PIN, String(YASOLR_RELAY2_PIN));
  Mycila::Config.configure(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE);
  Mycila::Config.configure(KEY_RELAY2_POWER, String(YASOLR_RELAY_POWER));
  // output 1
  Mycila::Config.configure(KEY_OUTPUT1_DIMMER_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_DIMMER_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT1_DIMMER_PIN, String(YASOLR_OUTPUT1_DIMMER_PIN));
  Mycila::Config.configure(KEY_OUTPUT1_DIMMER_TYPE, YASOLR_OUTPUT_DIMMER_TYPE);
  Mycila::Config.configure(KEY_OUTPUT1_DIMMER_AUTO, YASOLR_BOOL(YASOLR_OUTPUT_DIMMER_AUTO));
  Mycila::Config.configure(KEY_OUTPUT1_DIMMER_LEVEL_LIMIT, String(YASOLR_OUTPUT_DIMMER_LEVEL_LIMIT));
  Mycila::Config.configure(KEY_OUTPUT1_RELAY_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_RELAY_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT1_RELAY_PIN, String(YASOLR_OUTPUT1_RELAY_PIN));
  Mycila::Config.configure(KEY_OUTPUT1_RELAY_TYPE, YASOLR_OUTPUT_RELAY_TYPE);
  Mycila::Config.configure(KEY_OUTPUT1_TEMP_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_TEMP_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT1_TEMP_PIN, String(YASOLR_OUTPUT1_TEMP_PIN));
  Mycila::Config.configure(KEY_OUTPUT1_AUTO_BYPASS_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_AUTO_BYPASS_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT1_AUTO_START_TEMPERATURE, String(YASOLR_OUTPUT_AUTO_START_TEMPERATURE));
  Mycila::Config.configure(KEY_OUTPUT1_AUTO_STOP_TEMPERATURE, String(YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE));
  Mycila::Config.configure(KEY_OUTPUT1_AUTO_START_TIME, String(YASOLR_OUTPUT_AUTO_START_TIME));
  Mycila::Config.configure(KEY_OUTPUT1_AUTO_STOP_TIME, String(YASOLR_OUTPUT_AUTO_STOP_TIME));
  Mycila::Config.configure(KEY_OUTPUT1_AUTO_WEEK_DAYS, String(YASOLR_OUTPUT_AUTO_WEEK_DAYS));
  // output 2
  Mycila::Config.configure(KEY_OUTPUT2_DIMMER_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_DIMMER_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT2_DIMMER_PIN, String(YASOLR_OUTPUT2_DIMMER_PIN));
  Mycila::Config.configure(KEY_OUTPUT2_DIMMER_TYPE, YASOLR_OUTPUT_DIMMER_TYPE);
  Mycila::Config.configure(KEY_OUTPUT2_DIMMER_AUTO, YASOLR_BOOL(YASOLR_OUTPUT_DIMMER_AUTO));
  Mycila::Config.configure(KEY_OUTPUT2_DIMMER_LEVEL_LIMIT, String(YASOLR_OUTPUT_DIMMER_LEVEL_LIMIT));
  Mycila::Config.configure(KEY_OUTPUT2_RELAY_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_RELAY_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT2_RELAY_PIN, String(YASOLR_OUTPUT2_RELAY_PIN));
  Mycila::Config.configure(KEY_OUTPUT2_RELAY_TYPE, YASOLR_OUTPUT_RELAY_TYPE);
  Mycila::Config.configure(KEY_OUTPUT2_TEMP_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_TEMP_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT2_TEMP_PIN, String(YASOLR_OUTPUT2_TEMP_PIN));
  Mycila::Config.configure(KEY_OUTPUT2_AUTO_BYPASS_ENABLE, YASOLR_BOOL(YASOLR_OUTPUT_AUTO_BYPASS_ENABLE));
  Mycila::Config.configure(KEY_OUTPUT2_AUTO_START_TEMPERATURE, String(YASOLR_OUTPUT_AUTO_START_TEMPERATURE));
  Mycila::Config.configure(KEY_OUTPUT2_AUTO_STOP_TEMPERATURE, String(YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE));
  Mycila::Config.configure(KEY_OUTPUT2_AUTO_START_TIME, String(YASOLR_OUTPUT_AUTO_START_TIME));
  Mycila::Config.configure(KEY_OUTPUT2_AUTO_STOP_TIME, String(YASOLR_OUTPUT_AUTO_STOP_TIME));
  Mycila::Config.configure(KEY_OUTPUT2_AUTO_WEEK_DAYS, String(YASOLR_OUTPUT_AUTO_WEEK_DAYS));
  // zcd
  Mycila::Config.configure(KEY_ZCD_ENABLE, YASOLR_BOOL(YASOLR_ZCD_ENABLE));
  Mycila::Config.configure(KEY_ZCD_PIN, String(YASOLR_ZCD_PIN));
  Mycila::Config.configure(KEY_GRID_FREQ, String(YASOLR_GRID_FREQUENCY));
  // jsy
  Mycila::Config.configure(KEY_JSY_ENABLE, YASOLR_BOOL(YASOLR_JSY_ENABLE));
  Mycila::Config.configure(KEY_JSY_RX_PIN, String(YASOLR_JSY_RX_PIN));
  Mycila::Config.configure(KEY_JSY_TX_PIN, String(YASOLR_JSY_TX_PIN));
  Mycila::Config.configure(KEY_GRID_POWER_MQTT_TOPIC, YASOLR_GRID_POWER_MQTT_TOPIC);
}

Mycila::EasyDisplayType YaSolR::YaSolRClass::getDisplayType() const {
  const String val = Mycila::Config.get(KEY_DISPLAY_TYPE);
  if (val == "SSD1306")
    return Mycila::EasyDisplayType::SSD1306;
  if (val == "SH1107")
    return Mycila::EasyDisplayType::SH1107;
  return Mycila::EasyDisplayType::SH1106;
}

uint32_t YaSolR::YaSolRClass::getDisplayRotation() const {
  const uint32_t val = Mycila::Config.get(KEY_DISPLAY_ROTATION).toInt();
  return val == 90 || val == 180 || val == 270 ? val : 0;
}

uint32_t YaSolR::YaSolRClass::getRelayPauseDuration() const {
  return YASOLR_RELAY_PAUSE_DURATION;
}
