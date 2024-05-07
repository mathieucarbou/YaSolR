// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

// relay manager

uint16_t Mycila::RelayManagerConfigClass::getRelayLoad(const char* name) const {
  if (strcmp(name, "relay1") == 0)
    return config.get(KEY_RELAY1_LOAD).toInt();
  if (strcmp(name, "relay2") == 0)
    return config.get(KEY_RELAY2_LOAD).toInt();
  return 0;
}

// router output

bool Mycila::RouterOutputConfigClass::isAutoDimmerEnabled(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  if (strcmp(name, "output2") == 0)
    return config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  return false;
}
uint8_t Mycila::RouterOutputConfigClass::getDimmerLevelLimit(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.get(KEY_OUTPUT1_DIMMER_LIMITER).toInt();
  if (strcmp(name, "output2") == 0)
    return config.get(KEY_OUTPUT2_DIMMER_LIMITER).toInt();
  return YASOLR_DIMMER_MAX_LEVEL;
}
bool Mycila::RouterOutputConfigClass::isAutoBypassEnabled(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  if (strcmp(name, "output2") == 0)
    return config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  return false;
}
uint8_t Mycila::RouterOutputConfigClass::getAutoStartTemperature(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.get(KEY_OUTPUT1_TEMPERATURE_START).toInt();
  if (strcmp(name, "output2") == 0)
    return config.get(KEY_OUTPUT2_TEMPERATURE_START).toInt();
  return 100;
}
uint8_t Mycila::RouterOutputConfigClass::getAutoStopTemperature(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.get(KEY_OUTPUT1_TEMPERATURE_STOP).toInt();
  if (strcmp(name, "output2") == 0)
    return config.get(KEY_OUTPUT2_TEMPERATURE_STOP).toInt();
  return 100;
}
String Mycila::RouterOutputConfigClass::getAutoStartTime(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.get(KEY_OUTPUT1_TIME_START);
  if (strcmp(name, "output2") == 0)
    return config.get(KEY_OUTPUT2_TIME_START);
  return "00:00";
}
String Mycila::RouterOutputConfigClass::getAutoStopTime(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.get(KEY_OUTPUT1_TIME_STOP);
  if (strcmp(name, "output2") == 0)
    return config.get(KEY_OUTPUT2_TIME_STOP);
  return "00:00";
}
String Mycila::RouterOutputConfigClass::getWeekDays(const char* name) const {
  if (strcmp(name, "output1") == 0)
    return config.get(KEY_OUTPUT1_DAYS);
  if (strcmp(name, "output2") == 0)
    return config.get(KEY_OUTPUT2_DAYS);
  return "sun,mon,tue,wed,thu,fri,sat";
}
