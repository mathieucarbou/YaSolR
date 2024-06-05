// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
#define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                             (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
#define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                      (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

#define YASOLR_TRUE     "true"
#define YASOLR_FALSE    "false"
#define YASOLR_ON       "on"
#define YASOLR_OFF      "off"
#define YASOLR_BOOL(a)  ((a) ? YASOLR_TRUE : YASOLR_FALSE)
#define YASOLR_STATE(a) ((a) ? YASOLR_ON : YASOLR_OFF)

// password configuration keys

#define KEY_ADMIN_PASSWORD "admin_pwd"

// enable configuration keys

#define KEY_ENABLE_AP_MODE             "ap_mode_enable"
#define KEY_ENABLE_DEBUG               "debug_enable"
#define KEY_ENABLE_DISPLAY             "disp_enable"
#define KEY_ENABLE_DS18_SYSTEM         "ds18_sys_enable"
#define KEY_ENABLE_HA_DISCOVERY        "ha_disco_enable"
#define KEY_ENABLE_JSY                 "jsy_enable"
#define KEY_ENABLE_LIGHTS              "lights_enable"
#define KEY_ENABLE_MQTT                "mqtt_enable"
#define KEY_ENABLE_OUTPUT1_AUTO_BYPASS "o1_ab_enable"
#define KEY_ENABLE_OUTPUT1_AUTO_DIMMER "o1_ad_enable"
#define KEY_ENABLE_OUTPUT1_DIMMER      "o1_dim_enable"
#define KEY_ENABLE_OUTPUT1_DS18        "o1_ds18_enable"
#define KEY_ENABLE_OUTPUT1_PZEM        "o1_pzem_enable"
#define KEY_ENABLE_OUTPUT1_RELAY       "o1_relay_enable"
#define KEY_ENABLE_OUTPUT2_AUTO_BYPASS "o2_ab_enable"
#define KEY_ENABLE_OUTPUT2_AUTO_DIMMER "o2_ad_enable"
#define KEY_ENABLE_OUTPUT2_DIMMER      "o2_dim_enable"
#define KEY_ENABLE_OUTPUT2_DS18        "o2_ds18_enable"
#define KEY_ENABLE_OUTPUT2_PZEM        "o2_pzem_enable"
#define KEY_ENABLE_OUTPUT2_RELAY       "o2_relay_enable"
#define KEY_ENABLE_RELAY1              "relay1_enable"
#define KEY_ENABLE_RELAY2              "relay2_enable"
#define KEY_ENABLE_ZCD                 "zcd_enable"

// configuration keys

#define KEY_DISPLAY_ROTATION          "disp_angle"
#define KEY_DISPLAY_TYPE              "disp_type"
#define KEY_GRID_FREQUENCY            "grid_freq"
#define KEY_GRID_POWER_MQTT_TOPIC     "grid_pow_mqtt"
#define KEY_GRID_VOLTAGE              "grid_volt"
#define KEY_GRID_VOLTAGE_MQTT_TOPIC   "grid_volt_mqtt"
#define KEY_HA_DISCOVERY_TOPIC        "ha_disco_topic"
#define KEY_MQTT_PASSWORD             "mqtt_pwd"
#define KEY_MQTT_PORT                 "mqtt_port"
#define KEY_MQTT_PUBLISH_INTERVAL     "mqtt_pub_itvl"
#define KEY_MQTT_SECURED              "mqtt_secure"
#define KEY_MQTT_SERVER               "mqtt_server"
#define KEY_MQTT_TOPIC                "mqtt_topic"
#define KEY_MQTT_USERNAME             "mqtt_user"
#define KEY_NTP_SERVER                "ntp_server"
#define KEY_NTP_TIMEZONE              "ntp_timezone"
#define KEY_OUTPUT1_DAYS              "o1_days"
#define KEY_OUTPUT1_DIMMER_LIMITER    "o1_dim_limit"
#define KEY_OUTPUT1_RELAY_TYPE        "o1_relay_type"
#define KEY_OUTPUT1_TEMPERATURE_START "o1_temp_start"
#define KEY_OUTPUT1_TEMPERATURE_STOP  "o1_temp_stop"
#define KEY_OUTPUT1_TIME_START        "o1_time_start"
#define KEY_OUTPUT1_TIME_STOP         "o1_time_stop"
#define KEY_OUTPUT2_DAYS              "o2_days"
#define KEY_OUTPUT2_DIMMER_LIMITER    "o2_dim_limit"
#define KEY_OUTPUT2_RELAY_TYPE        "o2_relay_type"
#define KEY_OUTPUT2_TEMPERATURE_START "o2_temp_start"
#define KEY_OUTPUT2_TEMPERATURE_STOP  "o2_temp_stop"
#define KEY_OUTPUT2_TIME_START        "o2_time_start"
#define KEY_OUTPUT2_TIME_STOP         "o2_time_stop"
#define KEY_RELAY1_LOAD               "relay1_load"
#define KEY_RELAY1_TYPE               "relay1_type"
#define KEY_RELAY2_LOAD               "relay2_load"
#define KEY_RELAY2_TYPE               "relay2_type"
#define KEY_WIFI_PASSWORD             "wifi_pwd"
#define KEY_WIFI_SSID                 "wifi_ssid"

// pinout configuration keys

#define KEY_PIN_DISPLAY_SCL    "pin_disp_scl"
#define KEY_PIN_DISPLAY_SDA    "pin_disp_sda"
#define KEY_PIN_JSY_RT         "pin_jsy_tx"
#define KEY_PIN_JSY_RX         "pin_jsy_rx"
#define KEY_PIN_LIGHTS_GREEN   "pin_lights_g"
#define KEY_PIN_LIGHTS_RED     "pin_lights_r"
#define KEY_PIN_LIGHTS_YELLOW  "pin_lights_y"
#define KEY_PIN_OUTPUT1_DIMMER "pin_o1_dim"
#define KEY_PIN_OUTPUT1_DS18   "pin_o1_ds18"
#define KEY_PIN_OUTPUT1_RELAY  "pin_o1_relay"
#define KEY_PIN_OUTPUT2_DIMMER "pin_o2_dim"
#define KEY_PIN_OUTPUT2_DS18   "pin_o2_ds18"
#define KEY_PIN_OUTPUT2_RELAY  "pin_o2_relay"
#define KEY_PIN_PZEM_RX        "pin_pzem_rx"
#define KEY_PIN_PZEM_TX        "pin_pzem_tx"
#define KEY_PIN_RELAY1         "pin_relay1"
#define KEY_PIN_RELAY2         "pin_relay2"
#define KEY_PIN_ROUTER_DS18    "pin_ds18"
#define KEY_PIN_ZCD            "pin_zcd"

// default settings

#define YASOLR_ADMIN_USERNAME       "admin"
#define YASOLR_DIMMER_MAX_LEVEL     MYCILA_DIMMER_MAX_LEVEL
#define YASOLR_DISPLAY_LINE_SIZE    21
#define YASOLR_DISPLAY_LINES        5
#define YASOLR_MQTT_KEEPALIVE       60
#define YASOLR_MQTT_WILL_TOPIC      "/status"
#define YASOLR_PZEM_ADDRESS_OUTPUT1 0x01
#define YASOLR_PZEM_ADDRESS_OUTPUT2 0x02
#define YASOLR_RELAY_TYPE_NC        "NC"
#define YASOLR_RELAY_TYPE_NO        "NO"
#define YASOLR_SERIAL_BAUDRATE      115200
#define YASOLR_WEEK_DAYS            "sun,mon,tue,wed,thu,fri,sat"

// pinout

#ifndef YASOLR_LIGHTS_GREEN_PIN
#define YASOLR_LIGHTS_GREEN_PIN 0
#endif
#ifndef YASOLR_LIGHTS_YELLOW_PIN
#define YASOLR_LIGHTS_YELLOW_PIN 2
#endif
#ifndef YASOLR_LIGHTS_RED_PIN
#define YASOLR_LIGHTS_RED_PIN 15
#endif
#ifndef YASOLR_DISPLAY_CLOCK_PIN
#define YASOLR_DISPLAY_CLOCK_PIN SCL
#endif
#ifndef YASOLR_DISPLAY_DATA_PIN
#define YASOLR_DISPLAY_DATA_PIN SDA
#endif
#ifndef YASOLR_SYSTEM_TEMP_PIN
#define YASOLR_SYSTEM_TEMP_PIN 4
#endif
#ifndef YASOLR_RELAY1_PIN
#define YASOLR_RELAY1_PIN 13
#endif
#ifndef YASOLR_RELAY2_PIN
#define YASOLR_RELAY2_PIN 12
#endif
#ifndef YASOLR_OUTPUT1_DIMMER_PIN
#define YASOLR_OUTPUT1_DIMMER_PIN 25
#endif
#ifndef YASOLR_OUTPUT1_RELAY_PIN
#define YASOLR_OUTPUT1_RELAY_PIN 32
#endif
#ifndef YASOLR_OUTPUT1_TEMP_PIN
#define YASOLR_OUTPUT1_TEMP_PIN 18
#endif
#ifndef YASOLR_OUTPUT_AUTO_START_TEMPERATURE
#define YASOLR_OUTPUT_AUTO_START_TEMPERATURE 50
#endif
#ifndef YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE
#define YASOLR_OUTPUT_AUTO_STOP_TEMPERATURE 60
#endif
#ifndef YASOLR_OUTPUT2_DIMMER_PIN
#define YASOLR_OUTPUT2_DIMMER_PIN 26
#endif
#ifndef YASOLR_OUTPUT2_RELAY_PIN
#define YASOLR_OUTPUT2_RELAY_PIN 33
#endif
#ifndef YASOLR_OUTPUT2_TEMP_PIN
#define YASOLR_OUTPUT2_TEMP_PIN 5
#endif
#ifndef YASOLR_ZCD_PIN
#define YASOLR_ZCD_PIN 35
#endif
#ifndef YASOLR_JSY_RX_PIN
#define YASOLR_JSY_RX_PIN 16
#endif
#ifndef YASOLR_JSY_TX_PIN
#define YASOLR_JSY_TX_PIN 17
#endif
#ifndef YASOLR_JSY_SERIAL
#define YASOLR_JSY_SERIAL Serial2
#endif
#ifndef YASOLR_PZEM_RX_PIN
#define YASOLR_PZEM_RX_PIN 14
#endif
#ifndef YASOLR_PZEM_TX_PIN
#define YASOLR_PZEM_TX_PIN 27
#endif
#ifndef YASOLR_PZEM_SERIAL
#define YASOLR_PZEM_SERIAL Serial1
#endif
