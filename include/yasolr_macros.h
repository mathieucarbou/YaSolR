// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

// GPIO

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

// LOGGING REDIRECTION

#ifdef MYCILA_LOGGER_SUPPORT
  #define LOGD(tag, format, ...) logger.debug(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) logger.info(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) logger.warn(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) logger.error(tag, format, ##__VA_ARGS__)
  #define IS_LOGD(tag)           (logger.isDebugEnabled())
#else
  #define LOGD(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
  #define IS_LOGD(tag)           (esp_log_level_get(tag) >= ESP_LOG_DEBUG)
#endif

// YASOLR

#define TAG "YASOLR"

#define YASOLR_TRUE     "true"
#define YASOLR_FALSE    "false"
#define YASOLR_ON       "on"
#define YASOLR_OFF      "off"
#define YASOLR_BOOL(a)  ((a) ? YASOLR_TRUE : YASOLR_FALSE)
#define YASOLR_STATE(a) ((a) ? YASOLR_ON : YASOLR_OFF)

#define YASOLR_LANG_EN 1
#define YASOLR_LANG_FR 2

#ifndef YASOLR_LANG
  #define YASOLR_LANG YASOLR_LANG_EN
#endif

#if YASOLR_LANG == YASOLR_LANG_FR
  #include "i18n/fr.h"
#else
  #include "i18n/en.h"
#endif

// default settings

#define YASOLR_ADMIN_USERNAME              "admin"
#define YASOLR_DIMMER_LSA_GP8211S          "LSA + DAC GP8211S (DFR1071)"
#define YASOLR_DIMMER_LSA_GP8403           "LSA + DAC GP8403 (DFR0971)"
#define YASOLR_DIMMER_LSA_GP8413           "LSA + DAC GP8413 (DFR1073)"
#define YASOLR_DIMMER_LSA_PWM              "LSA + PWM->Analog 0-10V only"
#define YASOLR_DIMMER_LSA_PWM_ZCD          "LSA + PWM->Analog 0-10V + ZCD"
#define YASOLR_DIMMER_RANDOM_SSR           "Random Solid State Relay + ZCD"
#define YASOLR_DIMMER_ROBODYN              "Robodyn 24A / 40A"
#define YASOLR_DIMMER_TRIAC                "Triac + ZCD"
#define YASOLR_DIMMER_ZC_SSR               "Zero-crossing Solid State Relay"
#define YASOLR_DS18_SEARCH_MAX_RETRY       30
#define YASOLR_GRAPH_POINTS                60
#define YASOLR_HIDDEN_PWD                  "********"
#define YASOLR_LOG_FILE                    "/logs.txt"
#define YASOLR_MQTT_KEEPALIVE              60
#define YASOLR_MQTT_MEASUREMENT_EXPIRATION 60000
#define YASOLR_MQTT_SERVER_CERT_FILE       "/mqtt-server.pem"
#define YASOLR_MQTT_WILL_TOPIC             "/status"
#define YASOLR_PID_IC_MODE_1               "1: Clamp"
#define YASOLR_PID_IC_MODE_2               "2: Advanced"
#define YASOLR_PID_P_MODE_1                "1: On Error"
#define YASOLR_PID_P_MODE_2                "2: On Input"
#define YASOLR_PZEM_ADDRESS_OUTPUT1        0x01
#define YASOLR_PZEM_ADDRESS_OUTPUT2        0x02
#define YASOLR_RELAY_TYPE_NC               "NC"
#define YASOLR_RELAY_TYPE_NO               "NO"
#define YASOLR_SAFEBOOT_PARTITION_NAME     "safeboot" // See: https://github.com/mathieucarbou/MycilaSafeBoot
#define YASOLR_SAFEBOOT_PARTITION_SIZE     655360     // See: https://github.com/mathieucarbou/MycilaSafeBoot
#define YASOLR_SERIAL_BAUDRATE             115200
#define YASOLR_UART_1_NAME                 "Serial1"
#define YASOLR_UART_2_NAME                 "Serial2"
#define YASOLR_UART_NONE                   "N/A"
#define YASOLR_WEEK_DAYS                   "sun,mon,tue,wed,thu,fri,sat"
#define YASOLR_WEEK_DAYS_EMPTY             "none"

// UART

#if SOC_UART_NUM > 2
  #define YASOLR_UART_CHOICES YASOLR_UART_NONE "," YASOLR_UART_1_NAME "," YASOLR_UART_2_NAME
  #define JSY_UART_DEFAULT    YASOLR_UART_2_NAME
  #define PZEM_UART_DEFAULT   YASOLR_UART_1_NAME
#else
  #define YASOLR_UART_CHOICES YASOLR_UART_NONE "," YASOLR_UART_1_NAME
  #define JSY_UART_DEFAULT    YASOLR_UART_1_NAME
  #define PZEM_UART_DEFAULT   YASOLR_UART_NONE
#endif

// UDP communication

#define YASOLR_UDP_PORT 53964
// #define YASOLR_UDP_MSG_TYPE_JSY_DATA 0x01 // old json
#define YASOLR_UDP_MSG_TYPE_JSY_DATA 0x02 // new json

// password configuration keys

#define KEY_ADMIN_PASSWORD "admin_pwd"

// enable configuration keys

#define KEY_ENABLE_AP_MODE             "ap_mode_enable"
#define KEY_ENABLE_DEBUG               "debug_enable"
#define KEY_ENABLE_DISPLAY             "disp_enable"
#define KEY_ENABLE_DS18_SYSTEM         "ds18_sys_enable"
#define KEY_ENABLE_HA_DISCOVERY        "ha_disco_enable"
#define KEY_ENABLE_JSY                 "jsy_enable"
#define KEY_ENABLE_JSY_REMOTE          "jsyr_enable"
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
#define KEY_ENABLE_VICTRON_MODBUS      "vic_mb_enable"
#define KEY_ENABLE_ZCD                 "zcd_enable"

// configuration keys

#define KEY_DISPLAY_ROTATION               "disp_angle"
#define KEY_DISPLAY_SPEED                  "disp_speed"
#define KEY_DISPLAY_TYPE                   "disp_type"
#define KEY_GRID_FREQUENCY                 "grid_freq"
#define KEY_GRID_POWER_MQTT_TOPIC          "grid_pow_mqtt"
#define KEY_GRID_VOLTAGE_MQTT_TOPIC        "grid_volt_mqtt"
#define KEY_HA_DISCOVERY_TOPIC             "ha_disco_topic"
#define KEY_HOSTNAME                       "hostname"
#define KEY_JSY_UART                       "jsy_uart"
#define KEY_MQTT_PASSWORD                  "mqtt_pwd"
#define KEY_MQTT_PORT                      "mqtt_port"
#define KEY_MQTT_PUBLISH_INTERVAL          "mqtt_pub_itvl"
#define KEY_MQTT_SECURED                   "mqtt_secure"
#define KEY_MQTT_SERVER                    "mqtt_server"
#define KEY_MQTT_TOPIC                     "mqtt_topic"
#define KEY_MQTT_USERNAME                  "mqtt_user"
#define KEY_NET_DNS                        "net_dns"
#define KEY_NET_GATEWAY                    "net_gw"
#define KEY_NET_IP                         "net_ip"
#define KEY_NET_SUBNET                     "net_subnet"
#define KEY_NTP_SERVER                     "ntp_server"
#define KEY_NTP_TIMEZONE                   "ntp_timezone"
#define KEY_OUTPUT1_BYPASS_TIMEOUT         "o1_bp_timeout"
#define KEY_OUTPUT1_DAYS                   "o1_days"
#define KEY_OUTPUT1_DIMMER_LIMIT           "o1_dim_limit"
#define KEY_OUTPUT1_DIMMER_MAX             "o1_dim_max"
#define KEY_OUTPUT1_DIMMER_MIN             "o1_dim_min"
#define KEY_OUTPUT1_DIMMER_TEMP_LIMITER    "o1_dim_max_t"
#define KEY_OUTPUT1_DIMMER_TYPE            "o1_dim_type"
#define KEY_OUTPUT1_EXCESS_LIMITER         "o1_excess_limit"
#define KEY_OUTPUT1_RELAY_TYPE             "o1_relay_type"
#define KEY_OUTPUT1_RESISTANCE             "o1_resistance"
#define KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC "o1_temp_mqtt"
#define KEY_OUTPUT1_TEMPERATURE_START      "o1_temp_start"
#define KEY_OUTPUT1_TEMPERATURE_STOP       "o1_temp_stop"
#define KEY_OUTPUT1_TIME_START             "o1_time_start"
#define KEY_OUTPUT1_TIME_STOP              "o1_time_stop"
#define KEY_OUTPUT2_BYPASS_TIMEOUT         "o2_bp_timeout"
#define KEY_OUTPUT2_DAYS                   "o2_days"
#define KEY_OUTPUT2_DIMMER_LIMIT           "o2_dim_limit"
#define KEY_OUTPUT2_DIMMER_MAX             "o2_dim_max"
#define KEY_OUTPUT2_DIMMER_MIN             "o2_dim_min"
#define KEY_OUTPUT2_DIMMER_TEMP_LIMITER    "o2_dim_max_t"
#define KEY_OUTPUT2_DIMMER_TYPE            "o2_dim_type"
#define KEY_OUTPUT2_EXCESS_LIMITER         "o2_excess_limit"
#define KEY_OUTPUT2_RELAY_TYPE             "o2_relay_type"
#define KEY_OUTPUT2_RESISTANCE             "o2_resistance"
#define KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC "o2_temp_mqtt"
#define KEY_OUTPUT2_TEMPERATURE_START      "o2_temp_start"
#define KEY_OUTPUT2_TEMPERATURE_STOP       "o2_temp_stop"
#define KEY_OUTPUT2_TIME_START             "o2_time_start"
#define KEY_OUTPUT2_TIME_STOP              "o2_time_stop"
#define KEY_OUTPUT2_TIME_STOP              "o2_time_stop"
#define KEY_PID_IC_MODE                    "pid_icmode"
#define KEY_PID_KD                         "pid_kd"
#define KEY_PID_KI                         "pid_ki"
#define KEY_PID_KP                         "pid_kp"
#define KEY_PID_OUT_MAX                    "pid_out_max"
#define KEY_PID_OUT_MIN                    "pid_out_min"
#define KEY_PID_P_MODE                     "pid_pmode"
#define KEY_PID_SETPOINT                   "pid_setpoint"
#define KEY_PZEM_UART                      "pzem_uart"
#define KEY_RELAY1_LOAD                    "relay1_load"
#define KEY_RELAY1_TOLERANCE               "relay1_tol"
#define KEY_RELAY1_TYPE                    "relay1_type"
#define KEY_RELAY2_LOAD                    "relay2_load"
#define KEY_RELAY2_TOLERANCE               "relay2_tol"
#define KEY_RELAY2_TYPE                    "relay2_type"
#define KEY_UDP_PORT                       "udp_port"
#define KEY_VICTRON_MODBUS_PORT            "vic_mb_port"
#define KEY_VICTRON_MODBUS_SERVER          "vic_mb_server"
#define KEY_WIFI_BSSID                     "wifi_bssid"
#define KEY_WIFI_PASSWORD                  "wifi_pwd"
#define KEY_WIFI_SSID                      "wifi_ssid"

// pinout configuration keys

#define KEY_PIN_I2C_SCL        "pin_i2c_scl"
#define KEY_PIN_I2C_SDA        "pin_i2c_sda"
#define KEY_PIN_JSY_TX         "pin_jsy_tx"
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

// pinout

#ifndef YASOLR_LIGHTS_GREEN_PIN
  #define YASOLR_LIGHTS_GREEN_PIN -1
#endif
#ifndef YASOLR_LIGHTS_YELLOW_PIN
  #define YASOLR_LIGHTS_YELLOW_PIN -1
#endif
#ifndef YASOLR_LIGHTS_RED_PIN
  #define YASOLR_LIGHTS_RED_PIN -1
#endif
#ifndef YASOLR_I2C_SCL_PIN
  #define YASOLR_I2C_SCL_PIN SCL
#endif
#ifndef YASOLR_I2C_SDA_PIN
  #define YASOLR_I2C_SDA_PIN SDA
#endif
#ifndef YASOLR_SYSTEM_TEMP_PIN
  #define YASOLR_SYSTEM_TEMP_PIN -1
#endif
#ifndef YASOLR_RELAY1_PIN
  #define YASOLR_RELAY1_PIN -1
#endif
#ifndef YASOLR_RELAY2_PIN
  #define YASOLR_RELAY2_PIN -1
#endif
#ifndef YASOLR_OUTPUT1_DIMMER_PIN
  #define YASOLR_OUTPUT1_DIMMER_PIN -1
#endif
#ifndef YASOLR_OUTPUT1_RELAY_PIN
  #define YASOLR_OUTPUT1_RELAY_PIN -1
#endif
#ifndef YASOLR_OUTPUT1_TEMP_PIN
  #define YASOLR_OUTPUT1_TEMP_PIN -1
#endif
#ifndef YASOLR_OUTPUT2_DIMMER_PIN
  #define YASOLR_OUTPUT2_DIMMER_PIN -1
#endif
#ifndef YASOLR_OUTPUT2_RELAY_PIN
  #define YASOLR_OUTPUT2_RELAY_PIN -1
#endif
#ifndef YASOLR_OUTPUT2_TEMP_PIN
  #define YASOLR_OUTPUT2_TEMP_PIN -1
#endif
#ifndef YASOLR_ZCD_PIN
  #define YASOLR_ZCD_PIN -1
#endif
#ifndef YASOLR_JSY_RX_PIN
  #define YASOLR_JSY_RX_PIN -1
#endif
#ifndef YASOLR_JSY_TX_PIN
  #define YASOLR_JSY_TX_PIN -1
#endif
#ifndef YASOLR_PZEM_RX_PIN
  #define YASOLR_PZEM_RX_PIN -1
#endif
#ifndef YASOLR_PZEM_TX_PIN
  #define YASOLR_PZEM_TX_PIN -1
#endif
