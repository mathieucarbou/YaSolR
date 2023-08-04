// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#include <LittleFS.h>

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
  Mycila::Config.configure(KEY_OUTPUT1_PZEM_ENABLE, YASOLR_BOOL(YASOLR_PZEM_ENABLE));
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
  Mycila::Config.configure(KEY_OUTPUT2_PZEM_ENABLE, YASOLR_BOOL(YASOLR_PZEM_ENABLE));
  // zcd
  Mycila::Config.configure(KEY_ZCD_ENABLE, YASOLR_BOOL(YASOLR_ZCD_ENABLE));
  Mycila::Config.configure(KEY_ZCD_PIN, String(YASOLR_ZCD_PIN));
  Mycila::Config.configure(KEY_GRID_FREQ, String(YASOLR_GRID_FREQUENCY));
  Mycila::Config.configure(KEY_GRID_VOLTAGE, String(YASOLR_GRID_VOLTAGE));
  // jsy
  Mycila::Config.configure(KEY_JSY_ENABLE, YASOLR_BOOL(YASOLR_JSY_ENABLE));
  Mycila::Config.configure(KEY_JSY_RX_PIN, String(YASOLR_JSY_RX_PIN));
  Mycila::Config.configure(KEY_JSY_TX_PIN, String(YASOLR_JSY_TX_PIN));
  Mycila::Config.configure(KEY_GRID_POWER_MQTT_TOPIC, YASOLR_GRID_POWER_MQTT_TOPIC);
  Mycila::Config.configure(KEY_GRID_VOLTAGE_MQTT_TOPIC, YASOLR_GRID_VOLTAGE_MQTT_TOPIC);
  // pzem
  Mycila::Config.configure(KEY_PZEM_RX_PIN, String(YASOLR_PZEM_RX_PIN));
  Mycila::Config.configure(KEY_PZEM_TX_PIN, String(YASOLR_PZEM_TX_PIN));
}

// yasolr

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

Mycila::DimmerType YaSolR::YaSolRClass::getDimmerType(const char* name) const {
  const char* key = nullptr;
  if (strcmp(name, NAME_OUTPUT1) == 0)
    key = KEY_OUTPUT1_DIMMER_TYPE;
  else if (strcmp(name, NAME_OUTPUT2) == 0)
    key = KEY_OUTPUT2_DIMMER_TYPE;
  else
    return Mycila::DimmerType::TRIAC;
  String val = Mycila::Config.get(key);
  if (val == "TRIAC")
    return Mycila::DimmerType::TRIAC;
  if (val == "SSR_RANDOM")
    return Mycila::DimmerType::SSR_RANDOM;
  if (val == "SSR_ZC")
    return Mycila::DimmerType::SSR_ZC;
  return Mycila::DimmerType::TRIAC;
}

Mycila::MQTTConfig YaSolR::YaSolRClass::getMQTTConfig() const {
  bool secured = Mycila::Config.getBool(KEY_MQTT_SECURED);
  String serverCert = "";
  if (secured && LittleFS.exists("/mqtt-server.crt")) {
    Mycila::Logger.debug(TAG, "Loading MQTT server certificate...");
    File serverCertFile = LittleFS.open("/mqtt-server.crt", "r");
    serverCert = serverCertFile.readString();
    serverCertFile.close();
    Mycila::Logger.debug(TAG, "Loaded MQTT server certificate:\n%s", serverCert.c_str());
  }
  return {
    Mycila::Config.get(KEY_MQTT_SERVER),
    static_cast<uint16_t>(Mycila::Config.get(KEY_MQTT_PORT).toInt()),
    secured,
    serverCert,
    Mycila::Config.get(KEY_MQTT_USERNAME),
    Mycila::Config.get(KEY_MQTT_PASSWORD),
    Mycila::AppInfo.name + "-" + Mycila::AppInfo.id,
    Mycila::Config.get(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC,
    YASOLR_MQTT_KEEPALIVE};
}

// relay manager

uint16_t Mycila::RelayManagerConfigClass::getPowerThreshold(const char* name) const {
  if (strcmp(name, NAME_RELAY1) == 0)
    return Mycila::Config.get(KEY_RELAY1_POWER).toInt();
  if (strcmp(name, NAME_RELAY2) == 0)
    return Mycila::Config.get(KEY_RELAY1_POWER).toInt();
  return 0;
}

// router output

bool Mycila::RouterOutputConfigClass::isAutoDimmerEnabled(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_AUTO);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_AUTO);
  return false;
}
uint8_t Mycila::RouterOutputConfigClass::getDimmerLevelLimit(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_DIMMER_LEVEL_LIMIT).toInt();
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_DIMMER_LEVEL_LIMIT).toInt();
  return 100;
}
bool Mycila::RouterOutputConfigClass::isAutoBypassEnabled(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT1_AUTO_BYPASS_ENABLE);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT2_AUTO_BYPASS_ENABLE);
  return false;
}
uint8_t Mycila::RouterOutputConfigClass::getAutoStartTemperature(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_AUTO_START_TEMPERATURE).toInt();
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_AUTO_START_TEMPERATURE).toInt();
  return 100;
}
uint8_t Mycila::RouterOutputConfigClass::getAutoStopTemperature(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_AUTO_STOP_TEMPERATURE).toInt();
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_AUTO_STOP_TEMPERATURE).toInt();
  return 100;
}
String Mycila::RouterOutputConfigClass::getAutoStartTime(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_AUTO_START_TIME);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_AUTO_START_TIME);
  return "00:00";
}
String Mycila::RouterOutputConfigClass::getAutoStopTime(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_AUTO_STOP_TIME);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_AUTO_STOP_TIME);
  return "00:00";
}
String Mycila::RouterOutputConfigClass::getWeekDays(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_AUTO_WEEK_DAYS);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_AUTO_WEEK_DAYS);
  return "sun,mon,tue,wed,thu,fri,sat";
}
