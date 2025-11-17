// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <functional>
#include <queue>
#include <string>

Mycila::ConfigStorageNVS storage;
Mycila::Config config(storage);

static std::queue<std::function<void()>> reconfigureQueue;
static Mycila::Task reconfigureTask("Reconfigure", []() {
  auto fn = reconfigureQueue.front();
  reconfigureQueue.pop();
  fn();
  dashboardInitTask.resume();
});

void yasolr_init_config() {
  ESP_LOGI(TAG, "Configuring %s", Mycila::AppInfo.nameModelVersion.c_str());

  // setup config system
  config.configure(KEY_ADMIN_PASSWORD);
  config.configure(KEY_DISPLAY_ROTATION, "0");
  config.configure(KEY_DISPLAY_SPEED, "3");
  config.configure(KEY_DISPLAY_TYPE, "SH1106");
  config.configure(KEY_ENABLE_AP_MODE, YASOLR_FALSE);
  config.configure(KEY_ENABLE_DEBUG, YASOLR_TRUE);
  config.configure(KEY_ENABLE_DEBUG_BOOT, YASOLR_FALSE);
  config.configure(KEY_ENABLE_DISPLAY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_SYSTEM_DS18, YASOLR_FALSE);
  config.configure(KEY_ENABLE_HA_DISCOVERY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_JSY_REMOTE, YASOLR_FALSE);
  config.configure(KEY_ENABLE_JSY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_LIGHTS, YASOLR_FALSE);
  config.configure(KEY_ENABLE_MQTT, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_DIMMER, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_DS18, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_PZEM, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT1_RELAY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_DIMMER, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_DS18, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_PZEM, YASOLR_FALSE);
  config.configure(KEY_ENABLE_OUTPUT2_RELAY, YASOLR_FALSE);
  config.configure(KEY_ENABLE_RELAY1, YASOLR_FALSE);
  config.configure(KEY_ENABLE_RELAY2, YASOLR_FALSE);
  config.configure(KEY_ENABLE_VICTRON_MODBUS, YASOLR_FALSE);
  config.configure(KEY_GRID_FREQUENCY, "0");
  config.configure(KEY_GRID_POWER_MQTT_TOPIC);
  config.configure(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  config.configure(KEY_HA_DISCOVERY_TOPIC, YASOLR_HA_DISCOVERY_TOPIC);
  config.configure(KEY_HOSTNAME, Mycila::AppInfo.defaultHostname);
  config.configure(KEY_JSY_UART, JSY_UART_DEFAULT);
  config.configure(KEY_MQTT_PASSWORD);
  config.configure(KEY_MQTT_PORT, "1883");
  config.configure(KEY_MQTT_PUBLISH_INTERVAL, "5");
  config.configure(KEY_MQTT_SECURED, YASOLR_FALSE);
  config.configure(KEY_MQTT_SERVER);
  config.configure(KEY_MQTT_TOPIC, Mycila::AppInfo.defaultMqttClientId);
  config.configure(KEY_MQTT_USERNAME);
  config.configure(KEY_NET_DNS);
  config.configure(KEY_NET_GATEWAY);
  config.configure(KEY_NET_IP);
  config.configure(KEY_NET_SUBNET);
  config.configure(KEY_NTP_SERVER, "pool.ntp.org");
  config.configure(KEY_NTP_TIMEZONE, "Europe/Paris");
  config.configure(KEY_OUTPUT1_BYPASS_TIMEOUT, "0");
  config.configure(KEY_OUTPUT1_DAYS, YASOLR_WEEK_DAYS);
  config.configure(KEY_OUTPUT1_DIMMER_LIMIT, "100");
  config.configure(KEY_OUTPUT1_DIMMER_MAX, "100");
  config.configure(KEY_OUTPUT1_DIMMER_MIN, "0");
  config.configure(KEY_OUTPUT1_DIMMER_TEMP_LIMITER, "0");
  config.configure(KEY_OUTPUT1_DIMMER_TYPE, "");
  config.configure(KEY_OUTPUT1_EXCESS_LIMITER, "0");
  config.configure(KEY_OUTPUT1_EXCESS_RATIO, "100");
  config.configure(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_OUTPUT1_RESISTANCE, "0");
  config.configure(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  config.configure(KEY_OUTPUT1_TEMPERATURE_START, "50");
  config.configure(KEY_OUTPUT1_TEMPERATURE_STOP, "60");
  config.configure(KEY_OUTPUT1_TIME_START, "22:00");
  config.configure(KEY_OUTPUT1_TIME_STOP, "06:00");
  config.configure(KEY_OUTPUT2_BYPASS_TIMEOUT, "0");
  config.configure(KEY_OUTPUT2_DAYS, YASOLR_WEEK_DAYS);
  config.configure(KEY_OUTPUT2_DIMMER_LIMIT, "100");
  config.configure(KEY_OUTPUT2_DIMMER_MAX, "100");
  config.configure(KEY_OUTPUT2_DIMMER_MIN, "0");
  config.configure(KEY_OUTPUT2_DIMMER_TEMP_LIMITER, "0");
  config.configure(KEY_OUTPUT2_DIMMER_TYPE, "");
  config.configure(KEY_OUTPUT2_EXCESS_LIMITER, "0");
  config.configure(KEY_OUTPUT2_EXCESS_RATIO, "100");
  config.configure(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_OUTPUT2_RESISTANCE, "0");
  config.configure(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  config.configure(KEY_OUTPUT2_TEMPERATURE_START, "50");
  config.configure(KEY_OUTPUT2_TEMPERATURE_STOP, "60");
  config.configure(KEY_OUTPUT2_TIME_START, "22:00");
  config.configure(KEY_OUTPUT2_TIME_STOP, "06:00");
  config.configure(KEY_PID_KD, "0.05");
  config.configure(KEY_PID_KI, "0.3");
  config.configure(KEY_PID_KP, "0.1");
  config.configure(KEY_PID_MODE_D, YASOLR_PID_MODE_ERROR);
  config.configure(KEY_PID_MODE_P, YASOLR_PID_MODE_INPUT);
  config.configure(KEY_PID_OUT_MAX, "4000");
  config.configure(KEY_PID_OUT_MIN, "-300");
  config.configure(KEY_PID_SETPOINT, "0");
  config.configure(KEY_PIN_I2C_SCL, std::to_string(YASOLR_I2C_SCL_PIN));
  config.configure(KEY_PIN_I2C_SDA, std::to_string(YASOLR_I2C_SDA_PIN));
  config.configure(KEY_PIN_JSY_RX, std::to_string(YASOLR_JSY_RX_PIN));
  config.configure(KEY_PIN_JSY_TX, std::to_string(YASOLR_JSY_TX_PIN));
  config.configure(KEY_PIN_LIGHTS_GREEN, std::to_string(YASOLR_LIGHTS_GREEN_PIN));
  config.configure(KEY_PIN_LIGHTS_RED, std::to_string(YASOLR_LIGHTS_RED_PIN));
  config.configure(KEY_PIN_LIGHTS_YELLOW, std::to_string(YASOLR_LIGHTS_YELLOW_PIN));
  config.configure(KEY_PIN_OUTPUT1_DIMMER, std::to_string(YASOLR_OUTPUT1_DIMMER_PIN));
  config.configure(KEY_PIN_OUTPUT1_DS18, std::to_string(YASOLR_OUTPUT1_TEMP_PIN));
  config.configure(KEY_PIN_OUTPUT1_RELAY, std::to_string(YASOLR_OUTPUT1_RELAY_PIN));
  config.configure(KEY_PIN_OUTPUT2_DIMMER, std::to_string(YASOLR_OUTPUT2_DIMMER_PIN));
  config.configure(KEY_PIN_OUTPUT2_DS18, std::to_string(YASOLR_OUTPUT2_TEMP_PIN));
  config.configure(KEY_PIN_OUTPUT2_RELAY, std::to_string(YASOLR_OUTPUT2_RELAY_PIN));
  config.configure(KEY_PIN_PZEM_RX, std::to_string(YASOLR_PZEM_RX_PIN));
  config.configure(KEY_PIN_PZEM_TX, std::to_string(YASOLR_PZEM_TX_PIN));
  config.configure(KEY_PIN_RELAY1, std::to_string(YASOLR_RELAY1_PIN));
  config.configure(KEY_PIN_RELAY2, std::to_string(YASOLR_RELAY2_PIN));
  config.configure(KEY_PIN_ROUTER_DS18, std::to_string(YASOLR_SYSTEM_TEMP_PIN));
  config.configure(KEY_PIN_ZCD, std::to_string(YASOLR_ZCD_PIN));
  config.configure(KEY_PZEM_UART, PZEM_UART_DEFAULT);
  config.configure(KEY_RELAY1_LOAD, "0");
  config.configure(KEY_RELAY1_TOLERANCE, "7");
  config.configure(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_RELAY2_LOAD, "0");
  config.configure(KEY_RELAY2_TOLERANCE, "7");
  config.configure(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_UDP_PORT, std::to_string(YASOLR_UDP_PORT));
  config.configure(KEY_VICTRON_MODBUS_PORT, "502");
  config.configure(KEY_VICTRON_MODBUS_SERVER);
  config.configure(KEY_WIFI_BSSID);
  config.configure(KEY_WIFI_PASSWORD);
  config.configure(KEY_WIFI_SSID);

  // init and preload
  config.begin("YASOLR", true);

  config.listen([]() {
    ESP_LOGI(TAG, "Configuration restored!");
    restartTask.resume();
  });

  config.listen([](const char* k, const char* newValue) {
    ESP_LOGI(TAG, "'%s' => '%s'", k, newValue);
    const std::string key = k;

    if (key == KEY_RELAY1_LOAD) {
      if (relay1) {
        relay1->setNominalLoad(config.getLong(KEY_RELAY1_LOAD));
        if (!relay1->getNominalLoad()) {
          relay1->trySwitchRelay(false);
        }
      }

    } else if (key == KEY_RELAY2_LOAD) {
      if (relay2) {
        relay2->setNominalLoad(config.getLong(KEY_RELAY2_LOAD));
        if (!relay2->getNominalLoad()) {
          relay2->trySwitchRelay(false);
        }
      }

    } else if (key == KEY_RELAY1_TOLERANCE) {
      if (relay1) {
        relay1->setTolerance(config.getFloat(KEY_RELAY1_TOLERANCE) / 100.0f);
      }

    } else if (key == KEY_RELAY2_TOLERANCE) {
      if (relay2) {
        relay2->setTolerance(config.getFloat(KEY_RELAY2_TOLERANCE) / 100.0f);
      }

    } else if (key == KEY_OUTPUT1_RESISTANCE) {
      output1.config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_DIMMER) {
      if (config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER)) {
        output1.config.autoDimmer = true;
      } else {
        output1.config.autoDimmer = false;
        output1.setDimmerOff();
      }

    } else if (key == KEY_OUTPUT1_DIMMER_MIN) {
      output1.setDimmerDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100.0f);

    } else if (key == KEY_OUTPUT1_DIMMER_MAX) {
      output1.setDimmerDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100.0f);

    } else if (key == KEY_OUTPUT1_DIMMER_LIMIT) {
      output1.setDimmerDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100.0f);

    } else if (key == KEY_OUTPUT1_DIMMER_TEMP_LIMITER) {
      output1.config.dimmerTempLimit = config.getLong(KEY_OUTPUT1_DIMMER_TEMP_LIMITER);

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_BYPASS) {
      output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_START) {
      output1.config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_STOP) {
      output1.config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);

    } else if (key == KEY_OUTPUT1_TIME_START) {
      output1.config.autoStartTime = config.getString(KEY_OUTPUT1_TIME_START);

    } else if (key == KEY_OUTPUT1_TIME_STOP) {
      output1.config.autoStopTime = config.getString(KEY_OUTPUT1_TIME_STOP);

    } else if (key == KEY_OUTPUT1_DAYS) {
      output1.config.weekDays = config.getString(KEY_OUTPUT1_DAYS);

    } else if (key == KEY_OUTPUT1_EXCESS_LIMITER) {
      output1.config.excessPowerLimiter = config.getInt(KEY_OUTPUT1_EXCESS_LIMITER);

    } else if (key == KEY_OUTPUT1_EXCESS_RATIO) {
      output1.config.excessPowerRatio = config.getFloat(KEY_OUTPUT1_EXCESS_RATIO) / 100.0f;

    } else if (key == KEY_OUTPUT1_BYPASS_TIMEOUT) {
      output1.config.bypassTimeoutSec = config.getInt(KEY_OUTPUT1_BYPASS_TIMEOUT);

    } else if (key == KEY_OUTPUT2_RESISTANCE) {
      output2.config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_DIMMER) {
      if (config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER)) {
        output2.config.autoDimmer = true;
      } else {
        output2.config.autoDimmer = false;
        output2.setDimmerOff();
      }

    } else if (key == KEY_OUTPUT2_DIMMER_MIN) {
      output2.setDimmerDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100.0f);

    } else if (key == KEY_OUTPUT2_DIMMER_MAX) {
      output2.setDimmerDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100.0f);

    } else if (key == KEY_OUTPUT2_DIMMER_LIMIT) {
      output2.setDimmerDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100.0f);

    } else if (key == KEY_OUTPUT2_DIMMER_TEMP_LIMITER) {
      output2.config.dimmerTempLimit = config.getLong(KEY_OUTPUT2_DIMMER_TEMP_LIMITER);

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_BYPASS) {
      output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_START) {
      output2.config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_STOP) {
      output2.config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);

    } else if (key == KEY_OUTPUT2_TIME_START) {
      output2.config.autoStartTime = config.getString(KEY_OUTPUT2_TIME_START);

    } else if (key == KEY_OUTPUT2_TIME_STOP) {
      output2.config.autoStopTime = config.getString(KEY_OUTPUT2_TIME_STOP);

    } else if (key == KEY_OUTPUT2_DAYS) {
      output2.config.weekDays = config.getString(KEY_OUTPUT2_DAYS);

    } else if (key == KEY_OUTPUT2_EXCESS_LIMITER) {
      output2.config.excessPowerLimiter = config.getInt(KEY_OUTPUT2_EXCESS_LIMITER);

    } else if (key == KEY_OUTPUT2_EXCESS_RATIO) {
      output2.config.excessPowerRatio = config.getFloat(KEY_OUTPUT2_EXCESS_RATIO) / 100.0f;

    } else if (key == KEY_OUTPUT2_BYPASS_TIMEOUT) {
      output2.config.bypassTimeoutSec = config.getInt(KEY_OUTPUT2_BYPASS_TIMEOUT);

    } else if (key == KEY_NTP_TIMEZONE) {
      reconfigureQueue.push([]() {
        Mycila::NTP.setTimeZone(config.getString(KEY_NTP_TIMEZONE));
      });

    } else if (key == KEY_NTP_SERVER) {
      if (!config.getBool(KEY_ENABLE_AP_MODE))
        reconfigureQueue.push([]() {
          Mycila::NTP.sync(config.getString(KEY_NTP_SERVER));
        });

    } else if (key == KEY_PID_KP || key == KEY_PID_KI || key == KEY_PID_KD || key == KEY_PID_OUT_MIN || key == KEY_PID_OUT_MAX || key == KEY_PID_MODE_P || key == KEY_PID_MODE_D || key == KEY_PID_SETPOINT) {
      reconfigureQueue.push(yasolr_configure_pid);

    } else if (key == KEY_MQTT_PUBLISH_INTERVAL) {
      mqttPublishTask->setInterval(config.getLong(KEY_MQTT_PUBLISH_INTERVAL) * 1000);

    } else if (key == KEY_ENABLE_DEBUG) {
      reconfigureQueue.push(yasolr_configure_logging);

    } else if (key == KEY_ENABLE_MQTT) {
      reconfigureQueue.push([]() {
        yasolr_configure_mqtt();
        if (!config.getBool(KEY_ENABLE_AP_MODE) && mqttConnectTask) {
          mqttConnectTask->resume();
        }
      });

    } else if (key == KEY_ENABLE_HA_DISCOVERY) {
      if (haDiscoveryTask) {
        haDiscoveryTask->resume();
      }

    } else if (key == KEY_ENABLE_JSY_REMOTE) {
      reconfigureQueue.push([]() {
        yasolr_configure_jsy_remote();
        if (!config.getBool(KEY_ENABLE_AP_MODE) && jsyRemoteTask) {
          jsyRemoteTask->resume();
        }
      });

    } else if (key == KEY_ENABLE_DISPLAY) {
      reconfigureQueue.push(yasolr_configure_display);

    } else if (key == KEY_ENABLE_LIGHTS) {
      reconfigureQueue.push(yasolr_configure_lights);

    } else if (key == KEY_ENABLE_SYSTEM_DS18) {
      reconfigureQueue.push(yasolr_configure_router_ds18);

    } else if (key == KEY_ENABLE_OUTPUT1_DS18) {
      reconfigureQueue.push(yasolr_configure_output1_ds18);

    } else if (key == KEY_ENABLE_OUTPUT2_DS18) {
      reconfigureQueue.push(yasolr_configure_output2_ds18);

    } else if (key == KEY_ENABLE_RELAY1) {
      reconfigureQueue.push(yasolr_configure_relay1);

    } else if (key == KEY_ENABLE_RELAY2) {
      reconfigureQueue.push(yasolr_configure_relay2);

    } else if (key == KEY_ENABLE_VICTRON_MODBUS) {
      reconfigureQueue.push([]() {
        yasolr_configure_victron();
        if (!config.getBool(KEY_ENABLE_AP_MODE) && victronConnectTask) {
          victronConnectTask->resume();
        }
      });

    } else if (key == KEY_ENABLE_JSY) {
      reconfigureQueue.push(yasolr_configure_jsy);

    } else if (key == KEY_ENABLE_OUTPUT1_PZEM) {
      reconfigureQueue.push(yasolr_configure_output1_pzem);

    } else if (key == KEY_ENABLE_OUTPUT2_PZEM) {
      reconfigureQueue.push(yasolr_configure_output2_pzem);

    } else if (key == KEY_ENABLE_OUTPUT1_DIMMER) {
      reconfigureQueue.push(yasolr_configure_output1_dimmer);

    } else if (key == KEY_ENABLE_OUTPUT2_DIMMER) {
      reconfigureQueue.push(yasolr_configure_output2_dimmer);

    } else if (key == KEY_ENABLE_OUTPUT1_RELAY) {
      reconfigureQueue.push(yasolr_configure_output1_bypass_relay);

    } else if (key == KEY_ENABLE_OUTPUT2_RELAY) {
      reconfigureQueue.push(yasolr_configure_output2_bypass_relay);
    }

    dashboardInitTask.resume();
    if (mqttPublishConfigTask)
      mqttPublishConfigTask->resume();
  });

  reconfigureTask.setEnabledWhen([]() { return !reconfigureQueue.empty(); });
  unsafeTaskManager.addTask(reconfigureTask);
}
