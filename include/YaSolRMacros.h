// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#define YASOLR_TRUE "true"
#define YASOLR_FALSE "false"
#define YASOLR_ON "on"
#define YASOLR_OFF "off"
#define YASOLR_BOOL(a) ((a) ? YASOLR_TRUE : YASOLR_FALSE)
#define YASOLR_STATE(a) ((a) ? YASOLR_ON : YASOLR_OFF)

#define HIDDEN_PWD "********"
#define NAME_SYSTEM "system"
#define NAME_OUTPUT1 "output1"
#define NAME_OUTPUT2 "output2"
#define NAME_RELAY1 "relay1"
#define NAME_RELAY2 "relay2"

// configuration keys

#define KEY_HOSTNAME "hostname"
#define KEY_ADMIN_PASSWORD "admin_pwd"
#define KEY_DEBUG_ENABLE "debug_enable"

#define KEY_AP_MODE_ENABLE "ap_mode_enable"

#define KEY_WIFI_SSID "wifi_ssid"
#define KEY_WIFI_PASSWORD "wifi_pwd"
#define KEY_WIFI_CONNECTION_TIMEOUT "wifi_timeout"

#define KEY_CAPTURE_PORTAL_TIMEOUT "portal_timeout"

#define KEY_NTP_SERVER "ntp_server"
#define KEY_NTP_TIMEZONE "ntp_timezone"

#define KEY_MQTT_ENABLE "mqtt_enable"
#define KEY_MQTT_SERVER "mqtt_server"
#define KEY_MQTT_PORT "mqtt_port"
#define KEY_MQTT_SECURED "mqtt_secure"
#define KEY_MQTT_USERNAME "mqtt_user"
#define KEY_MQTT_PASSWORD "mqtt_pwd"
#define KEY_MQTT_TOPIC "mqtt_topic"
#define KEY_MQTT_PUBLISH_INTERVAL "mqtt_pub_itvl"

#define KEY_HA_DISCOVERY_ENABLE "ha_disco_enable"
#define KEY_HA_DISCOVERY_TOPIC "ha_disco_topic"

#define KEY_BUZZER_ENABLE "buzzer_enable"
#define KEY_BUZZER_PIN "buzzer_pin"

#define KEY_BUTTON_ENABLE "button_enable"
#define KEY_BUTTON_PIN "button_pin"
#define KEY_BUTTON_ACTION "button_action"

#define KEY_LIGHTS_ENABLE "lights_enable"
#define KEY_LIGHTS_GREEN_PIN "lights_g_pin"
#define KEY_LIGHTS_YELLOW_PIN "lights_y_pin"
#define KEY_LIGHTS_RED_PIN "lights_r_pin"

#define KEY_DISPLAY_ENABLE "dp_enable"
#define KEY_DISPLAY_TYPE "dp_type"
#define KEY_DISPLAY_CLOCK_PIN "dp_clock_pin"
#define KEY_DISPLAY_DATA_PIN "dp_data_pin"
#define KEY_DISPLAY_ROTATION "dp_rotation"
#define KEY_DISPLAY_POWER_SAVE_DELAY "dp_powsave"

#define KEY_SYSTEM_TEMP_ENABLE "sys_tmp_enable"
#define KEY_SYSTEM_TEMP_PIN "sys_tmp_pin"

#define KEY_RELAY1_ENABLE "relay1_enable"
#define KEY_RELAY1_PIN "relay1_pin"
#define KEY_RELAY1_TYPE "relay1_type"
#define KEY_RELAY1_POWER "relay1_power"

#define KEY_RELAY2_ENABLE "relay2_enable"
#define KEY_RELAY2_PIN "relay2_pin"
#define KEY_RELAY2_TYPE "relay2_type"
#define KEY_RELAY2_POWER "relay2_power"

#define KEY_OUTPUT1_DIMMER_ENABLE "out1_dim_enable"
#define KEY_OUTPUT1_DIMMER_PIN "out1_dim_pin"
#define KEY_OUTPUT1_DIMMER_TYPE "out1_dim_type"
#define KEY_OUTPUT1_DIMMER_AUTO "out1_dim_auto"
#define KEY_OUTPUT1_DIMMER_LEVEL_LIMIT "out1_dim_limit"

#define KEY_OUTPUT1_RELAY_ENABLE "out1_bp_enable"
#define KEY_OUTPUT1_RELAY_PIN "out1_bp_pin"
#define KEY_OUTPUT1_RELAY_TYPE "out1_bp_type"

#define KEY_OUTPUT1_TEMP_ENABLE "out1_tmp_enable"
#define KEY_OUTPUT1_TEMP_PIN "out1_tmp_pin"

#define KEY_OUTPUT1_AUTO_BYPASS_ENABLE "out1_aut_enable"
#define KEY_OUTPUT1_AUTO_START_TEMPERATURE "out1_tmp_low"
#define KEY_OUTPUT1_AUTO_STOP_TEMPERATURE "out1_tmp_high"
#define KEY_OUTPUT1_AUTO_START_TIME "out1_start_time"
#define KEY_OUTPUT1_AUTO_STOP_TIME "out1_end_time"
#define KEY_OUTPUT1_AUTO_WEEK_DAYS "out1_wdays"

#define KEY_OUTPUT2_DIMMER_ENABLE "out2_dim_enable"
#define KEY_OUTPUT2_DIMMER_PIN "out2_dim_pin"
#define KEY_OUTPUT2_DIMMER_TYPE "out2_dim_type"
#define KEY_OUTPUT2_DIMMER_AUTO "out2_dim_auto"
#define KEY_OUTPUT2_DIMMER_LEVEL_LIMIT "out2_dim_limit"

#define KEY_OUTPUT2_RELAY_ENABLE "out2_bp_enable"
#define KEY_OUTPUT2_RELAY_PIN "out2_bp_pin"
#define KEY_OUTPUT2_RELAY_TYPE "out2_bp_type"

#define KEY_OUTPUT2_TEMP_ENABLE "out2_tmp_enable"
#define KEY_OUTPUT2_TEMP_PIN "out2_tmp_pin"

#define KEY_OUTPUT2_AUTO_BYPASS_ENABLE "out2_aut_enable"
#define KEY_OUTPUT2_AUTO_START_TEMPERATURE "out2_tmp_low"
#define KEY_OUTPUT2_AUTO_STOP_TEMPERATURE "out2_tmp_high"
#define KEY_OUTPUT2_AUTO_START_TIME "out2_start_time"
#define KEY_OUTPUT2_AUTO_STOP_TIME "out2_end_time"
#define KEY_OUTPUT2_AUTO_WEEK_DAYS "out2_wdays"

#define KEY_ZCD_ENABLE "zcd_enable"
#define KEY_ZCD_PIN "zcd_pin"
#define KEY_GRID_FREQ "grid_freq"

#define KEY_JSY_ENABLE "jsy_enable"
#define KEY_JSY_RX_PIN "jsy_rx_pin"
#define KEY_JSY_TX_PIN "jsy_tx_pin"

#define KEY_GRID_POWER_MQTT_TOPIC "grid_pow_mqtt"

// default settinga

#ifndef YASOLR_TEMPERATURE_READ_INTERVAL
#define YASOLR_TEMPERATURE_READ_INTERVAL 10
#endif

#ifndef YASOLR_TEMPERATURE_READ_INTERVAL
#define YASOLR_TEMPERATURE_READ_INTERVAL 10
#endif

#ifndef YASOLR_SERIAL_BAUDRATE
#define YASOLR_SERIAL_BAUDRATE 115200
#endif

#ifndef YASOLR_ADMIN_USERNAME
#define YASOLR_ADMIN_USERNAME "admin"
#endif

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

#ifndef YASOLR_MQTT_WILL_TOPIC
#define YASOLR_MQTT_WILL_TOPIC "/status"
#endif

#ifndef YASOLR_MQTT_KEEPALIVE
#define YASOLR_MQTT_KEEPALIVE 60
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

#ifndef YASOLR_SYSTEM_TEMP_PIN
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
