// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#ifdef YASOLR_DISABLE_BROWNOUT_DETECTOR
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#endif

Mycila::Task bootTask("Boot", [](void* params) {
#ifdef YASOLR_DISABLE_BROWNOUT_DETECTOR
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif

  Serial.begin(YASOLR_SERIAL_BAUDRATE);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  // early logging
  logger.forwardTo(&Serial);
  logger.warn("YASOLR", "Booting %s...", Mycila::AppInfo.nameModelVersion.c_str());

  // system
  Mycila::System.begin();

  // trial
#ifdef APP_MODEL_TRIAL
  Mycila::Trial.begin();
#endif

  // setup config system
  config.begin(80);
  config.configure(KEY_ADMIN_PASSWORD);
  config.configure(KEY_DISPLAY_ROTATION, "0");
  config.configure(KEY_DISPLAY_SPEED, "5");
  config.configure(KEY_DISPLAY_TYPE, "SH1106");
  config.configure(KEY_ENABLE_AP_MODE, YASOLR_FALSE);
  config.configure(KEY_ENABLE_DEBUG, YASOLR_FALSE);
  config.configure(KEY_ENABLE_DISPLAY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_DS18_SYSTEM, YASOLR_FALSE);
  config.configure(KEY_ENABLE_HA_DISCOVERY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_JSY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_LIGHTS, YASOLR_FALSE);
  config.configure(KEY_ENABLE_MQTT, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_TRUE);
  config.configure(KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_TRUE);
  config.configure(KEY_ENABLE_OUTPUT1_DS18, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_PZEM, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_RELAY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_DIMMER, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_TRUE);
  config.configure(KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_TRUE);
  config.configure(KEY_ENABLE_OUTPUT2_DS18, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_PZEM, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_RELAY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_DIMMER, YASOLR_FALSE);
  config.configure(KEY_ENABLE_RELAY1, YASOLR_FALSE);
  config.configure(KEY_ENABLE_RELAY2, YASOLR_FALSE);
  config.configure(KEY_ENABLE_ZCD, YASOLR_FALSE);
  config.configure(KEY_GRID_FREQUENCY, "50");
  config.configure(KEY_GRID_POWER_MQTT_TOPIC);
  config.configure(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  config.configure(KEY_HA_DISCOVERY_TOPIC, MYCILA_HA_DISCOVERY_TOPIC);
  config.configure(KEY_MQTT_PASSWORD);
  config.configure(KEY_MQTT_PORT, "1883");
  config.configure(KEY_MQTT_PUBLISH_INTERVAL, "5");
  config.configure(KEY_MQTT_SECURED, YASOLR_FALSE);
  config.configure(KEY_MQTT_SERVER);
  config.configure(KEY_MQTT_TOPIC, Mycila::AppInfo.defaultMqttClientId);
  config.configure(KEY_MQTT_USERNAME, "homeassistant");
  config.configure(KEY_NTP_SERVER, "pool.ntp.org");
  config.configure(KEY_NTP_TIMEZONE, "Europe/Paris");
  config.configure(KEY_OUTPUT1_DAYS, YASOLR_WEEK_DAYS);
  config.configure(KEY_OUTPUT1_DIMMER_LIMITER, String(YASOLR_DIMMER_MAX_LEVEL));
  config.configure(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_OUTPUT1_TEMPERATURE_START, String(YASOLR_OUTPUT_AUTO_START_TEMPERATURE));
  config.configure(KEY_OUTPUT1_TEMPERATURE_STOP, String(YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE));
  config.configure(KEY_OUTPUT1_TIME_START, "22:00");
  config.configure(KEY_OUTPUT1_TIME_STOP, "06:00");
  config.configure(KEY_OUTPUT2_DAYS, YASOLR_WEEK_DAYS);
  config.configure(KEY_OUTPUT2_DIMMER_LIMITER, String(YASOLR_DIMMER_MAX_LEVEL));
  config.configure(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_OUTPUT2_TEMPERATURE_START, String(YASOLR_OUTPUT_AUTO_START_TEMPERATURE));
  config.configure(KEY_OUTPUT2_TEMPERATURE_STOP, String(YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE));
  config.configure(KEY_OUTPUT2_TIME_START, "22:00");
  config.configure(KEY_OUTPUT2_TIME_STOP, "06:00");
  config.configure(KEY_PIN_DISPLAY_SCL, String(YASOLR_DISPLAY_CLOCK_PIN));
  config.configure(KEY_PIN_DISPLAY_SDA, String(YASOLR_DISPLAY_DATA_PIN));
  config.configure(KEY_PIN_JSY_RT, String(YASOLR_JSY_TX_PIN));
  config.configure(KEY_PIN_JSY_RX, String(YASOLR_JSY_RX_PIN));
  config.configure(KEY_PIN_LIGHTS_GREEN, String(YASOLR_LIGHTS_GREEN_PIN));
  config.configure(KEY_PIN_LIGHTS_RED, String(YASOLR_LIGHTS_RED_PIN));
  config.configure(KEY_PIN_LIGHTS_YELLOW, String(YASOLR_LIGHTS_YELLOW_PIN));
  config.configure(KEY_PIN_OUTPUT1_DIMMER, String(YASOLR_OUTPUT1_DIMMER_PIN));
  config.configure(KEY_PIN_OUTPUT1_DS18, String(YASOLR_OUTPUT1_TEMP_PIN));
  config.configure(KEY_PIN_OUTPUT1_RELAY, String(YASOLR_OUTPUT1_RELAY_PIN));
  config.configure(KEY_PIN_OUTPUT2_DIMMER, String(YASOLR_OUTPUT2_DIMMER_PIN));
  config.configure(KEY_PIN_OUTPUT2_DS18, String(YASOLR_OUTPUT2_TEMP_PIN));
  config.configure(KEY_PIN_OUTPUT2_RELAY, String(YASOLR_OUTPUT2_RELAY_PIN));
  config.configure(KEY_PIN_PZEM_RX, String(YASOLR_PZEM_RX_PIN));
  config.configure(KEY_PIN_PZEM_TX, String(YASOLR_PZEM_TX_PIN));
  config.configure(KEY_PIN_RELAY1, String(YASOLR_RELAY1_PIN));
  config.configure(KEY_PIN_RELAY2, String(YASOLR_RELAY2_PIN));
  config.configure(KEY_PIN_ROUTER_DS18, String(YASOLR_SYSTEM_TEMP_PIN));
  config.configure(KEY_PIN_ZCD, String(YASOLR_ZCD_PIN));
  config.configure(KEY_RELAY1_LOAD, "0");
  config.configure(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_RELAY2_LOAD, "0");
  config.configure(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_WIFI_PASSWORD);
  config.configure(KEY_WIFI_SSID);

  // pre-init logging
  logger.setLevel(config.getBool(KEY_ENABLE_DEBUG) ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
  esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));
});
