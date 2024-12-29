// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

Mycila::Config config;

void yasolr_start_config() {
  // setup config system
  config.begin("YASOLR");
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
  config.configure(KEY_ENABLE_JSY_REMOTE, YASOLR_FALSE);
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
  config.configure(KEY_ENABLE_ZCD, YASOLR_FALSE);
  config.configure(KEY_GRID_FREQUENCY, "0");
  config.configure(KEY_GRID_POWER_MQTT_TOPIC);
  config.configure(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  config.configure(KEY_HA_DISCOVERY_TOPIC, MYCILA_HA_DISCOVERY_TOPIC);
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
  config.configure(KEY_OUTPUT1_DAYS, YASOLR_WEEK_DAYS);
  config.configure(KEY_OUTPUT1_DIMMER_LIMIT, "100");
  config.configure(KEY_OUTPUT1_DIMMER_STOP_TEMP, "0");
  config.configure(KEY_OUTPUT1_DIMMER_MAX, "100");
  config.configure(KEY_OUTPUT1_DIMMER_MIN, "0");
  config.configure(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_OUTPUT1_RESERVED_EXCESS, "100");
  config.configure(KEY_OUTPUT1_RESISTANCE, "0");
  config.configure(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  config.configure(KEY_OUTPUT1_TEMPERATURE_START, "50");
  config.configure(KEY_OUTPUT1_TEMPERATURE_STOP, "60");
  config.configure(KEY_OUTPUT1_TIME_START, "22:00");
  config.configure(KEY_OUTPUT1_TIME_STOP, "06:00");
  config.configure(KEY_OUTPUT2_DAYS, YASOLR_WEEK_DAYS);
  config.configure(KEY_OUTPUT2_DIMMER_LIMIT, "100");
  config.configure(KEY_OUTPUT2_DIMMER_STOP_TEMP, "0");
  config.configure(KEY_OUTPUT2_DIMMER_MAX, "100");
  config.configure(KEY_OUTPUT2_DIMMER_MIN, "0");
  config.configure(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_OUTPUT2_RESERVED_EXCESS, "100");
  config.configure(KEY_OUTPUT2_RESISTANCE, "0");
  config.configure(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  config.configure(KEY_OUTPUT2_TEMPERATURE_START, "50");
  config.configure(KEY_OUTPUT2_TEMPERATURE_STOP, "60");
  config.configure(KEY_OUTPUT2_TIME_START, "22:00");
  config.configure(KEY_OUTPUT2_TIME_STOP, "06:00");
  config.configure(KEY_PID_D_MODE, "1");
  config.configure(KEY_PID_IC_MODE, "2");
  config.configure(KEY_PID_KD, "0.05");
  config.configure(KEY_PID_KI, "0.2");
  config.configure(KEY_PID_KP, "0.1");
  config.configure(KEY_PID_OUT_MAX, "5000");
  config.configure(KEY_PID_OUT_MIN, "-500");
  config.configure(KEY_PID_P_MODE, "2");
  config.configure(KEY_PID_SETPOINT, "0");
  config.configure(KEY_PIN_DISPLAY_SCL, std::to_string(YASOLR_DISPLAY_CLOCK_PIN));
  config.configure(KEY_PIN_DISPLAY_SDA, std::to_string(YASOLR_DISPLAY_DATA_PIN));
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
  config.configure(KEY_RELAY1_LOAD, "0");
  config.configure(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_RELAY2_LOAD, "0");
  config.configure(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NO);
  config.configure(KEY_UDP_PORT, std::to_string(YASOLR_UDP_PORT));
  config.configure(KEY_WIFI_PASSWORD);
  config.configure(KEY_WIFI_SSID);

  config.listen([]() {
    logger.info(TAG, "Configuration restored!");
    restartTask.resume();
  });

  config.listen([](const char* k, const std::string& newValue) {
    logger.info(TAG, "'%s' => '%s'", k, newValue.c_str());
    const std::string key = k;

    if (key == KEY_ENABLE_DEBUG) {
      yasolr_configure_logging();

    } else if (key == KEY_RELAY1_LOAD) {
      routerRelay1.setLoad(config.getLong(KEY_RELAY1_LOAD));

    } else if (key == KEY_RELAY2_LOAD) {
      routerRelay2.setLoad(config.getLong(KEY_RELAY2_LOAD));

    } else if (key == KEY_OUTPUT1_RESISTANCE) {
      output1.config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);

    } else if (key == KEY_OUTPUT2_RESISTANCE) {
      output2.config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_DIMMER) {
      output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
      dimmerO1.off();

    } else if (key == KEY_OUTPUT1_DIMMER_MIN) {
      dimmerO1.setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_MAX) {
      dimmerO1.setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_LIMIT) {
      dimmerO1.setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_STOP_TEMP) {
      output1.config.dimmerTempLimit = config.getLong(KEY_OUTPUT1_DIMMER_STOP_TEMP);

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_BYPASS) {
      output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_START) {
      output1.config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_STOP) {
      output1.config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);

    } else if (key == KEY_OUTPUT1_TIME_START) {
      output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);

    } else if (key == KEY_OUTPUT1_TIME_STOP) {
      output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);

    } else if (key == KEY_OUTPUT1_DAYS) {
      output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);

    } else if (key == KEY_OUTPUT1_RESERVED_EXCESS) {
      output1.config.reservedExcessPowerRatio = config.getFloat(KEY_OUTPUT1_RESERVED_EXCESS) / 100;

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_DIMMER) {
      output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
      dimmerO2.off();

    } else if (key == KEY_OUTPUT2_DIMMER_MIN) {
      dimmerO2.setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_MAX) {
      dimmerO2.setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_LIMIT) {
      dimmerO2.setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_STOP_TEMP) {
      output2.config.dimmerTempLimit = config.getLong(KEY_OUTPUT2_DIMMER_STOP_TEMP);

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_BYPASS) {
      output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_START) {
      output2.config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_STOP) {
      output2.config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);

    } else if (key == KEY_OUTPUT2_TIME_START) {
      output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);

    } else if (key == KEY_OUTPUT2_TIME_STOP) {
      output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);

    } else if (key == KEY_OUTPUT2_DAYS) {
      output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);

    } else if (key == KEY_OUTPUT2_RESERVED_EXCESS) {
      output2.config.reservedExcessPowerRatio = config.getFloat(KEY_OUTPUT2_RESERVED_EXCESS) / 100;

    } else if (key == KEY_NTP_TIMEZONE) {
      Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

    } else if (key == KEY_NTP_SERVER) {
      if (!config.getBool(KEY_ENABLE_AP_MODE))
        Mycila::NTP.sync(config.get(KEY_NTP_SERVER));

    } else if (key == KEY_PID_KP || key == KEY_PID_KI || key == KEY_PID_KD || key == KEY_PID_OUT_MIN || key == KEY_PID_OUT_MAX || key == KEY_PID_P_MODE || key == KEY_PID_D_MODE || key == KEY_PID_IC_MODE || key == KEY_PID_SETPOINT) {
      pidController.setProportionalMode((Mycila::PID::ProportionalMode)config.getLong(KEY_PID_P_MODE));
      pidController.setDerivativeMode((Mycila::PID::DerivativeMode)config.getLong(KEY_PID_D_MODE));
      pidController.setIntegralCorrectionMode((Mycila::PID::IntegralCorrectionMode)config.getLong(KEY_PID_IC_MODE));
      pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
      pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
      pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));
      logger.info(TAG, "PID Controller reconfigured!");
    }

    dashboardInitTask.resume();
    if (mqttPublishConfigTask)
      mqttPublishConfigTask->resume();
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });

  // pre-init logging
  logger.setLevel(config.getBool(KEY_ENABLE_DEBUG) ? ARDUHAL_LOG_LEVEL_DEBUG : ARDUHAL_LOG_LEVEL_INFO);
  esp_log_level_set("*", static_cast<esp_log_level_t>(logger.getLevel()));
}
