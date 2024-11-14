// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <thyristor.h>

#include <string>

Mycila::Task initConfigTask("Init Config", [](void* params) {
  logger.warn(TAG, "Configuring %s", Mycila::AppInfo.nameModelVersion.c_str());

  // coreTaskManager
  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1 * Mycila::TaskDuration::SECONDS);
  carouselTask.setEnabledWhen([]() { return display.isEnabled(); });
  carouselTask.setIntervalSupplier([]() { return config.getLong(KEY_DISPLAY_SPEED) * Mycila::TaskDuration::SECONDS; });
  dashboardTask.setEnabledWhen([]() { return espConnect.isConnected() && !dashboard.isAsyncAccessInProgress(); });
  dashboardTask.setInterval(1000 * Mycila::TaskDuration::MILLISECONDS);
  debugTask.setEnabledWhen([]() { return config.getBool(KEY_ENABLE_DEBUG); });
  debugTask.setInterval(20 * Mycila::TaskDuration::SECONDS);
  displayTask.setEnabledWhen([]() { return display.isEnabled(); });
  displayTask.setInterval(500 * Mycila::TaskDuration::MILLISECONDS);
  lightsTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  relayTask.setEnabledWhen([]() { return !router.isCalibrationRunning() && (routerRelay1.isAutoRelayEnabled() || routerRelay2.isAutoRelayEnabled()); });
  relayTask.setInterval(7 * Mycila::TaskDuration::SECONDS);
  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500 * Mycila::TaskDuration::MILLISECONDS);
  zcdTask.setInterval(1000 * Mycila::TaskDuration::MILLISECONDS);
#ifdef APP_MODEL_TRIAL
  trialTask.setInterval(30 * Mycila::TaskDuration::SECONDS);
#endif

  // mqttTaskManager
  mqttPublishTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishTask.setIntervalSupplier([]() { return config.getLong(KEY_MQTT_PUBLISH_INTERVAL) * Mycila::TaskDuration::SECONDS; });
  mqttPublishStaticTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishConfigTask.setEnabledWhen([]() { return mqtt.isConnected(); });

  // pioTaskManager
  ds18Task.setEnabledWhen([]() { return ds18Sys.isEnabled() || ds18O1.isEnabled() || ds18O2.isEnabled(); });
  ds18Task.setInterval(8 * Mycila::TaskDuration::SECONDS);
  networkManagerTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);

  // jsyTaskManager
  jsyTask.setEnabledWhen([]() { return jsy.isEnabled(); });

  // pzemTaskManager
  pzemTask.setEnabledWhen([]() { return (pzemO1.isEnabled() || pzemO2.isEnabled()) && pzemO1PairingTask.isPaused() && pzemO2PairingTask.isPaused(); });

  // routingTaskManager
  routingTask.setEnabledWhen([]() { return !router.isCalibrationRunning() && (output1.isAutoDimmerEnabled() || output2.isAutoDimmerEnabled()); });

  // Grid
  grid.localMetrics().setExpiration(10000);                             // local is fast
  grid.remoteMetrics().setExpiration(10000);                            // remote JSY is fast
  grid.mqttPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);   // through mqtt
  grid.mqttVoltage().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // through mqtt
  grid.pzemVoltage().setExpiration(10000);                              // local is fast
  grid.power().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);       // local is fast

  // Relays
  routerRelay1.setLoad(config.getLong(KEY_RELAY1_LOAD));
  routerRelay2.setLoad(config.getLong(KEY_RELAY2_LOAD));

  // output1
  output1.config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);
  output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  output1.config.dimmerTempLimit = config.getLong(KEY_OUTPUT1_DIMMER_STOP_TEMP);
  output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  output1.config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);
  output1.config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);
  output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);
  output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);
  output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);
  output1.config.reservedExcessPowerRatio = config.getFloat(KEY_OUTPUT1_RESERVED_EXCESS) / 100;
  output1.temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

  // output2
  output2.config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);
  output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  output2.config.dimmerTempLimit = config.getLong(KEY_OUTPUT2_DIMMER_STOP_TEMP);
  output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  output2.config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);
  output2.config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);
  output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);
  output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);
  output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);
  output2.config.reservedExcessPowerRatio = config.getFloat(KEY_OUTPUT2_RESERVED_EXCESS) / 100;
  output2.temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

  // PID Controller

  pidController.setReverse(false);
  pidController.setProportionalMode((Mycila::PID::ProportionalMode)config.getLong(KEY_PID_P_MODE));
  pidController.setDerivativeMode((Mycila::PID::DerivativeMode)config.getLong(KEY_PID_D_MODE));
  pidController.setIntegralCorrectionMode((Mycila::PID::IntegralCorrectionMode)config.getLong(KEY_PID_IC_MODE));
  pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
  pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
  pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));

  // NTP
  Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

  // Home Assistant Discovery
  haDiscovery.setDiscoveryTopic(config.get(KEY_HA_DISCOVERY_TOPIC));
  haDiscovery.setWillTopic((std::string(config.get(KEY_MQTT_TOPIC)) + YASOLR_MQTT_WILL_TOPIC).c_str());
  haDiscovery.begin({
                      .id = Mycila::AppInfo.defaultMqttClientId.c_str(),
                      .name = Mycila::AppInfo.defaultSSID.c_str(),
                      .version = Mycila::AppInfo.version.c_str(),
                      .model = (Mycila::AppInfo.name + " " + Mycila::AppInfo.model).c_str(),
                      .manufacturer = Mycila::AppInfo.manufacturer.c_str(),
                      .url = std::string("http://") + espConnect.getIPAddress().toString().c_str(),
                    },
                    config.get(KEY_MQTT_TOPIC),
                    512,
                    [](const char* topic, const char* payload) {
                      mqtt.publish(topic, payload, true);
                    });

  // Lights
  if (config.getBool(KEY_ENABLE_LIGHTS))
    lights.begin(config.getLong(KEY_PIN_LIGHTS_GREEN), config.getLong(KEY_PIN_LIGHTS_YELLOW), config.getLong(KEY_PIN_LIGHTS_RED));

  // DS18
  if (config.getBool(KEY_ENABLE_DS18_SYSTEM))
    ds18Sys.begin(config.getLong(KEY_PIN_ROUTER_DS18));
  if (config.getBool(KEY_ENABLE_OUTPUT1_DS18))
    ds18O1.begin(config.getLong(KEY_PIN_OUTPUT1_DS18));
  if (config.getBool(KEY_ENABLE_OUTPUT2_DS18))
    ds18O2.begin(config.getLong(KEY_PIN_OUTPUT2_DS18));

  // Electricity: Relays
  if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY))
    bypassRelayO1.begin(config.getLong(KEY_PIN_OUTPUT1_RELAY), config.isEqual(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY))
    bypassRelayO2.begin(config.getLong(KEY_PIN_OUTPUT2_RELAY), config.isEqual(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_RELAY1))
    relay1.begin(config.getLong(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_RELAY2))
    relay2.begin(config.getLong(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

  // Electricity: JSY
  if (config.getBool(KEY_ENABLE_JSY)) {
    jsy.begin(YASOLR_JSY_SERIAL, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));
    if (jsy.isEnabled() && jsy.getBaudRate() != jsy.getMaxAvailableBaudRate())
      jsy.setBaudRate(jsy.getMaxAvailableBaudRate());
  }

  // Electricity: PZEMs
  if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM))
    pzemO1.begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT1);
  if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM))
    pzemO2.begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT2);

  // Display
  if (config.getBool(KEY_ENABLE_DISPLAY)) {
    const std::string displayType = config.get(KEY_DISPLAY_TYPE);
    if (displayType == "SSD1306")
      display.begin(Mycila::EasyDisplayType::SSD1306, config.getLong(KEY_PIN_DISPLAY_SCL), config.getLong(KEY_PIN_DISPLAY_SDA), config.getLong(KEY_DISPLAY_ROTATION));
    else if (displayType == "SH1107")
      display.begin(Mycila::EasyDisplayType::SH1107, config.getLong(KEY_PIN_DISPLAY_SCL), config.getLong(KEY_PIN_DISPLAY_SDA), config.getLong(KEY_DISPLAY_ROTATION));
    else if (displayType == "SH1106")
      display.begin(Mycila::EasyDisplayType::SH1106, config.getLong(KEY_PIN_DISPLAY_SCL), config.getLong(KEY_PIN_DISPLAY_SDA), config.getLong(KEY_DISPLAY_ROTATION));
    display.clearDisplay();
    display.setActive(true);
  }

  // WebSerial
#ifdef APP_MODEL_PRO
  WebSerial.setID(Mycila::AppInfo.firmware.c_str());
  WebSerial.setTitle((Mycila::AppInfo.name + " Web Console").c_str());
  WebSerial.setInput(false);
#endif
  WebSerial.begin(&webServer, "/console");
  logger.forwardTo(&WebSerial);

  // Dashboard
#ifdef APP_MODEL_PRO
  dashboard.setTitle(Mycila::AppInfo.nameModel.c_str());
#endif

  // Network Manager
  Mycila::ESPConnect::IPConfig ipConfig;
  ipConfig.ip.fromString(config.get(KEY_NET_IP));
  ipConfig.gateway.fromString(config.get(KEY_NET_GATEWAY));
  ipConfig.subnet.fromString(config.get(KEY_NET_SUBNET));
  ipConfig.dns.fromString(config.get(KEY_NET_DNS));
  espConnect.setIPConfig(ipConfig);
  espConnect.setAutoRestart(true);
  espConnect.setBlocking(false);
  espConnect.begin(Mycila::AppInfo.defaultHostname.c_str(), Mycila::AppInfo.defaultSSID.c_str(), config.get(KEY_ADMIN_PASSWORD), {config.get(KEY_WIFI_SSID), config.get(KEY_WIFI_PASSWORD), config.getBool(KEY_ENABLE_AP_MODE)});

  // ZCD + Dimmers
  pulseAnalyzer.onZeroCross(Thyristor::zero_cross_int);
  dimmerO1.setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100);
  dimmerO1.setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100);
  dimmerO1.setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100);
  dimmerO2.setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100);
  dimmerO2.setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100);
  dimmerO2.setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100);
  zcdTask.forceRun();
});
