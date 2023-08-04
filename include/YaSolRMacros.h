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

#ifndef YASOLR_SERIAL_BAUDRATE
#define YASOLR_SERIAL_BAUDRATE 115200
#endif

#ifndef YASOLR_ADMIN_USERNAME
#define YASOLR_ADMIN_USERNAME "admin"
#endif

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
