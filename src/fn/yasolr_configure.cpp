// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

void yasolr_configure() {
  logger.info(TAG, "Configuring %s", Mycila::AppInfo.nameModelVersion.c_str());

  // WDT
  Mycila::TaskManager::configureWDT(10, true);

  // Task Monitor
  Mycila::TaskMonitor.begin();
  // Mycila::TaskMonitor.addTask("arduino_events");            // Network (stack size cannot be set)
  // Mycila::TaskMonitor.addTask("wifi");                      // WiFI (stack size cannot be set)
  Mycila::TaskMonitor.addTask("mqtt_task");                 // MQTT (set stack size with MYCILA_MQTT_STACK_SIZE)
  Mycila::TaskMonitor.addTask("async_tcp");                 // AsyncTCP (set stack size with CONFIG_ASYNC_TCP_STACK_SIZE)
  Mycila::TaskMonitor.addTask(coreTaskManager.getName());   // YaSolR
  Mycila::TaskMonitor.addTask(unsafeTaskManager.getName()); // YaSolR

  // Grid
  grid.localMetrics().setExpiration(10000);                             // local is fast
  grid.remoteMetrics().setExpiration(10000);                            // remote JSY is fast
  grid.pzemMetrics().setExpiration(10000);                              // local is fast
  grid.mqttPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);   // through mqtt
  grid.mqttVoltage().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // through mqtt
  grid.getPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);    // local is fast

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

  //////////////
  // HARDWARE //
  //////////////

  router.localMetrics().setExpiration(10000); // local is fast
  router.remoteMetrics().setExpiration(10000); // remote JSY is fast

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
  output1.localMetrics().setExpiration(10000); // local is fast

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
  output2.localMetrics().setExpiration(10000); // local is fast

  // Home Assistant Discovery
  haDiscovery.setDiscoveryTopic(config.getString(KEY_HA_DISCOVERY_TOPIC));
  haDiscovery.setWillTopic(config.getString(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC);
  haDiscovery.begin({
                      .id = Mycila::AppInfo.defaultMqttClientId,
                      .name = Mycila::AppInfo.defaultSSID,
                      .version = Mycila::AppInfo.version,
                      .model = Mycila::AppInfo.name + " " + Mycila::AppInfo.model,
                      .manufacturer = Mycila::AppInfo.manufacturer,
                      .url = std::string("http://") + espConnect.getIPAddress().toString().c_str(),
                    },
                    config.get(KEY_MQTT_TOPIC),
                    512,
                    [](const char* topic, const char* payload) { mqtt.publish(topic, payload, true); });

  // Electricity: Relays
  if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY))
    bypassRelayO1.begin(config.getLong(KEY_PIN_OUTPUT1_RELAY), config.isEqual(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY))
    bypassRelayO2.begin(config.getLong(KEY_PIN_OUTPUT2_RELAY), config.isEqual(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_RELAY1))
    relay1.begin(config.getLong(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_RELAY2))
    relay2.begin(config.getLong(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

  // Relays
  routerRelay1.setLoad(config.getLong(KEY_RELAY1_LOAD));
  routerRelay2.setLoad(config.getLong(KEY_RELAY2_LOAD));

  // Dimmers
  dimmerO1.setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100);
  dimmerO1.setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100);
  dimmerO1.setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100);
  dimmerO2.setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100);
  dimmerO2.setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100);
  dimmerO2.setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100);

  // coreTaskManager
  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1 * Mycila::TaskDuration::SECONDS);
  networkManagerTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  relayTask.setEnabledWhen([]() { return !router.isCalibrationRunning() && (routerRelay1.isAutoRelayEnabled() || routerRelay2.isAutoRelayEnabled()); });
  relayTask.setInterval(7 * Mycila::TaskDuration::SECONDS);
  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500 * Mycila::TaskDuration::MILLISECONDS);

  // unsafeTaskManager
  mqttPublishTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishTask.setInterval(config.getLong(KEY_MQTT_PUBLISH_INTERVAL) * Mycila::TaskDuration::SECONDS);
  mqttPublishStaticTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishConfigTask.setEnabledWhen([]() { return mqtt.isConnected(); });

  // coreTaskManager
  calibrationTask.setManager(coreTaskManager);
  networkStartTask.setManager(coreTaskManager);
  networkManagerTask.setManager(coreTaskManager);
  relayTask.setManager(coreTaskManager);
  resetTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
  routerTask.setManager(coreTaskManager);
  safeBootTask.setManager(coreTaskManager);

  // unsafeTaskManager
  haDiscoveryTask.setManager(unsafeTaskManager);
  mqttConfigTask.setManager(unsafeTaskManager);
  mqttPublishConfigTask.setManager(unsafeTaskManager);
  mqttPublishStaticTask.setManager(unsafeTaskManager);
  mqttPublishTask.setManager(unsafeTaskManager);

  // Router
  router.addOutput(output1);
  router.addOutput(output2);
};
