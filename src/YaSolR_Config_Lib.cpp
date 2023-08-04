// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

// logger

uint8_t Mycila::LoggerClass::getLevel() const { return Mycila::Config.getBool(KEY_DEBUG_ENABLE) ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO; }

// button

bool Mycila::ButtonConfigClass::isEnabled() const { return Mycila::Config.getBool(KEY_BUTTON_ENABLE); }
int32_t Mycila::ButtonConfigClass::getPin() const { return Mycila::Config.get(KEY_BUTTON_PIN).toInt(); }
String Mycila::ButtonConfigClass::getPressAction() const { return Mycila::Config.get(KEY_BUTTON_ACTION); }
String Mycila::ButtonConfigClass::getLongPressAction() const { return "reset"; }

// ota

String Mycila::OTAConfigClass::getHostname() const { return Mycila::Config.get(KEY_HOSTNAME); }
String Mycila::OTAConfigClass::getUsername() const { return YASOLR_ADMIN_USERNAME; }
String Mycila::OTAConfigClass::getPassword() const { return Mycila::Config.get(KEY_ADMIN_PASSWORD); }
String Mycila::OTAConfigClass::getTitle() const { return AppInfo.name + " Web Updater"; }
String Mycila::OTAConfigClass::getFirmwareName() const { return AppInfo.firmware; }
String Mycila::OTAConfigClass::getFirmwareVersion() const { return AppInfo.version; }

// mqtt

const Mycila::MQTTConfig Mycila::MQTTClass::getConfig() {
  return {
    Mycila::Config.getBool(KEY_MQTT_ENABLE) && !Mycila::Config.getBool(KEY_AP_MODE_ENABLE),
    Mycila::Config.get(KEY_MQTT_SERVER),
    static_cast<uint16_t>(Mycila::Config.get(KEY_MQTT_PORT).toInt()),
    Mycila::Config.getBool(KEY_MQTT_SECURED),
    Mycila::Config.get(KEY_MQTT_USERNAME),
    Mycila::Config.get(KEY_MQTT_PASSWORD),
    AppInfo.name + "-" + AppInfo.id,
    Mycila::Config.get(KEY_MQTT_TOPIC),
    Mycila::Config.get(KEY_MQTT_TOPIC) + MYCILA_MQTT_WILL_TOPIC};
}

// temperature sensors

bool Mycila::TemperatureSensorConfigClass::isEnabled(const char* name) const {
  if (strcmp(name, NAME_SYSTEM) == 0)
    return Mycila::Config.getBool(KEY_SYSTEM_TEMP_ENABLE);
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT1_TEMP_ENABLE);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT2_TEMP_ENABLE);
  return false;
}
int32_t Mycila::TemperatureSensorConfigClass::getPin(const char* name) const {
  if (strcmp(name, NAME_SYSTEM) == 0)
    return Mycila::Config.get(KEY_SYSTEM_TEMP_PIN).toInt();
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_TEMP_PIN).toInt();
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_TEMP_PIN).toInt();
  return GPIO_NUM_NC;
}

// buzzer

bool Mycila::BuzzerConfigClass::isEnabled() const { return Mycila::Config.getBool(KEY_BUZZER_ENABLE); }
int32_t Mycila::BuzzerConfigClass::getPin() const { return Mycila::Config.get(KEY_BUZZER_PIN).toInt(); }

// leds

bool Mycila::LightsConfigClass::isEnabled() const { return Mycila::Config.getBool(KEY_LIGHTS_ENABLE); }
int32_t Mycila::LightsConfigClass::getGreenPin() const { return Mycila::Config.get(KEY_LIGHTS_GREEN_PIN).toInt(); }
int32_t Mycila::LightsConfigClass::getYellowPin() const { return Mycila::Config.get(KEY_LIGHTS_YELLOW_PIN).toInt(); }
int32_t Mycila::LightsConfigClass::getRedPin() const { return Mycila::Config.get(KEY_LIGHTS_RED_PIN).toInt(); }

// bypass

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

// httpd

String Mycila::HTTPdConfigClass::getUsername() const { return YASOLR_ADMIN_USERNAME; }
String Mycila::HTTPdConfigClass::getPassword() const { return Mycila::Config.get(KEY_ADMIN_PASSWORD); }

// relays

const Mycila::RelayConfig Mycila::Relay::getConfig(const char* name) {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return {
      Mycila::Config.getBool(KEY_OUTPUT1_RELAY_ENABLE),
      static_cast<uint8_t>(Mycila::Config.get(KEY_OUTPUT1_RELAY_PIN).toInt()),
      Mycila::Config.get(KEY_OUTPUT1_RELAY_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO};

  if (strcmp(name, NAME_OUTPUT2) == 0)
    return {
      Mycila::Config.getBool(KEY_OUTPUT2_RELAY_ENABLE),
      static_cast<uint8_t>(Mycila::Config.get(KEY_OUTPUT2_RELAY_PIN).toInt()),
      Mycila::Config.get(KEY_OUTPUT2_RELAY_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO};

  if (strcmp(name, NAME_RELAY1) == 0)
    return {
      Mycila::Config.getBool(KEY_RELAY1_ENABLE),
      static_cast<uint8_t>(Mycila::Config.get(KEY_RELAY1_PIN).toInt()),
      Mycila::Config.get(KEY_RELAY1_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO};

  if (strcmp(name, NAME_RELAY2) == 0)
    return {
      Mycila::Config.getBool(KEY_RELAY2_ENABLE),
      static_cast<uint8_t>(Mycila::Config.get(KEY_RELAY2_PIN).toInt()),
      Mycila::Config.get(KEY_RELAY2_TYPE) == "NC" ? Mycila::RelayType::NC : Mycila::RelayType::NO};

  return {false, static_cast<uint8_t>(GPIO_NUM_NC), Mycila::RelayType::NO};
}

uint16_t Mycila::RelayManagerConfigClass::getPowerThreshold(const char* name) const {
  if (strcmp(name, NAME_RELAY1) == 0)
    return Mycila::Config.get(KEY_RELAY1_POWER).toInt();
  if (strcmp(name, NAME_RELAY2) == 0)
    return Mycila::Config.get(KEY_RELAY1_POWER).toInt();
  return 0;
}

// dimmer

bool Mycila::DimmerConfigClass::isEnabled(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_ENABLE);
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_ENABLE);
  return false;
}
int32_t Mycila::DimmerConfigClass::getPin(const char* name) const {
  if (strcmp(name, NAME_OUTPUT1) == 0)
    return Mycila::Config.get(KEY_OUTPUT1_DIMMER_PIN).toInt();
  if (strcmp(name, NAME_OUTPUT2) == 0)
    return Mycila::Config.get(KEY_OUTPUT2_DIMMER_PIN).toInt();
  return GPIO_NUM_NC;
}
Mycila::DimmerType Mycila::DimmerConfigClass::getType(const char* name) const {
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

// power manager

String Mycila::GridConfigClass::getGridPowerMQTTTopic() const { return Mycila::Config.get(KEY_GRID_POWER_MQTT_TOPIC); }

// zero cross detection

bool Mycila::ZeroCrossDetectionConfigClass::isEnabled() const { return Mycila::Config.getBool(KEY_ZCD_ENABLE); }
int32_t Mycila::ZeroCrossDetectionConfigClass::getPin() const { return Mycila::Config.get(KEY_ZCD_PIN).toInt(); }
uint8_t Mycila::ZeroCrossDetectionConfigClass::getGridFrequency() const { return Mycila::Config.get(KEY_GRID_FREQ).toInt() == 60 ? 60 : 50; }
