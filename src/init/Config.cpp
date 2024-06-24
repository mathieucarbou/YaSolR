// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task initConfigTask("Init Config", [](void* params) {
  logger.warn(TAG, "Configuring %s...", Mycila::AppInfo.nameModelVersion.c_str());

  // Tasks configuration
  carouselTask.setEnabledWhen([]() { return display.isEnabled(); });
  carouselTask.setIntervalSupplier([]() { return config.get(KEY_DISPLAY_SPEED).toInt() * Mycila::TaskDuration::SECONDS; });
  dashboardCards.setEnabledWhen([]() { return ESPConnect.isConnected() && dashboard.hasClient() && !dashboard.isAsyncAccessInProgress() && !routingTask.isRunning(); });
  dashboardCards.setInterval(751 * Mycila::TaskDuration::MILLISECONDS);
  dashboardCharts.setEnabledWhen([]() { return ESPConnect.isConnected() && dashboard.hasClient() && !dashboard.isAsyncAccessInProgress() && !routingTask.isRunning(); });
  dashboardCharts.setInterval(1000 * Mycila::TaskDuration::MILLISECONDS);
  displayTask.setEnabledWhen([]() { return display.isEnabled(); });
  displayTask.setInterval(293 * Mycila::TaskDuration::MILLISECONDS);
  ds18Task.setEnabledWhen([]() { return ds18Sys.isEnabled() || ds18O1.isEnabled() || ds18O2.isEnabled(); });
  ds18Task.setInterval(1 * Mycila::TaskDuration::SECONDS);
  jsyTask.setEnabledWhen([]() { return jsy.isEnabled(); });
  lightsTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  mqttPublishConfigTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishStaticTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishTask.setIntervalSupplier([]() { return config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt() * Mycila::TaskDuration::SECONDS; });
  profilerTask.setInterval(10 * Mycila::TaskDuration::SECONDS);
  pzemTask.setEnabledWhen([]() { return (pzemO1.isEnabled() || pzemO2.isEnabled()) && pzemO1PairingTask.isPaused() && pzemO2PairingTask.isPaused(); });
  relayTask.setEnabledWhen([]() { return routerRelay1.isAutoRelayEnabled() || routerRelay2.isAutoRelayEnabled(); });
  relayTask.setInterval(7 * Mycila::TaskDuration::SECONDS);
  routerDebugTask.setInterval(5 * Mycila::TaskDuration::SECONDS);
  routerTask.setInterval(537 * Mycila::TaskDuration::MILLISECONDS);
  routingTask.setEnabledWhen([]() { return output1.isDimmerEnabled() || output2.isDimmerEnabled(); });
#ifdef APP_MODEL_TRIAL
  trialTask.setInterval(30 * Mycila::TaskDuration::SECONDS);
#endif

  // Grid
  grid.setExpiration(45);

  // Relays
  routerRelay1.setLoad(config.get(KEY_RELAY1_LOAD).toInt());
  routerRelay2.setLoad(config.get(KEY_RELAY2_LOAD).toInt());

  // output1
  output1.config.calibratedResistance = config.get(KEY_OUTPUT1_RESISTANCE).toFloat();
  output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  output1.config.dimmerDutyLimit = config.get(KEY_OUTPUT1_DIMMER_MAX_DUTY).toInt();
  output1.config.dimmerTempLimit = config.get(KEY_OUTPUT1_DIMMER_MAX_TEMP).toInt();
  output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  output1.config.autoStartTemperature = config.get(KEY_OUTPUT1_TEMPERATURE_START).toInt();
  output1.config.autoStopTemperature = config.get(KEY_OUTPUT1_TEMPERATURE_STOP).toInt();
  output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);
  output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);
  output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);

  // output2
  output2.config.calibratedResistance = config.get(KEY_OUTPUT2_RESISTANCE).toFloat();
  output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  output2.config.dimmerDutyLimit = config.get(KEY_OUTPUT2_DIMMER_MAX_DUTY).toInt();
  output2.config.dimmerTempLimit = config.get(KEY_OUTPUT2_DIMMER_MAX_TEMP).toInt();
  output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  output2.config.autoStartTemperature = config.get(KEY_OUTPUT2_TEMPERATURE_START).toInt();
  output2.config.autoStopTemperature = config.get(KEY_OUTPUT2_TEMPERATURE_STOP).toInt();
  output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);
  output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);
  output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);

  // NTP
  Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

  // Home Assistant Discovery
  haDiscovery.setDiscoveryTopic(config.get(KEY_HA_DISCOVERY_TOPIC));
  haDiscovery.setBaseTopic(config.get(KEY_MQTT_TOPIC));
  haDiscovery.setWillTopic(config.get(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC);
  haDiscovery.setBufferSise(512);
  haDiscovery.setDevice({
    .id = Mycila::AppInfo.defaultMqttClientId,
    .name = Mycila::AppInfo.defaultSSID,
    .version = Mycila::AppInfo.version,
    .model = Mycila::AppInfo.name + " " + Mycila::AppInfo.model,
    .manufacturer = Mycila::AppInfo.manufacturer,
    .url = "http://" + ESPConnect.getIPAddress().toString(),
  });

  // Lights
  if (config.getBool(KEY_ENABLE_LIGHTS))
    lights.begin(config.get(KEY_PIN_LIGHTS_GREEN).toInt(), config.get(KEY_PIN_LIGHTS_YELLOW).toInt(), config.get(KEY_PIN_LIGHTS_RED).toInt());

  // DS18
  ds18Sys.setExpirationDelay(60);
  ds18O1.setExpirationDelay(60);
  ds18O2.setExpirationDelay(60);
  if (config.getBool(KEY_ENABLE_DS18_SYSTEM))
    ds18Sys.begin(config.get(KEY_PIN_ROUTER_DS18).toInt());
  if (config.getBool(KEY_ENABLE_OUTPUT1_DS18))
    ds18O1.begin(config.get(KEY_PIN_OUTPUT1_DS18).toInt());
  if (config.getBool(KEY_ENABLE_OUTPUT2_DS18))
    ds18O2.begin(config.get(KEY_PIN_OUTPUT2_DS18).toInt());

  // Electricity: ZCD
  if (config.getBool(KEY_ENABLE_ZCD))
    zcd.begin(config.get(KEY_PIN_ZCD).toInt(), config.get(KEY_GRID_FREQUENCY).toInt() == 60 ? 60 : 50);

  // Electricity: Dimmers
  if (config.getBool(KEY_ENABLE_OUTPUT1_DIMMER))
    dimmerO1.begin(config.get(KEY_PIN_OUTPUT1_DIMMER).toInt());
  if (config.getBool(KEY_ENABLE_OUTPUT2_DIMMER))
    dimmerO2.begin(config.get(KEY_PIN_OUTPUT2_DIMMER).toInt());

  // Electricity: Relays
  if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY))
    bypassRelayO1.begin(config.get(KEY_PIN_OUTPUT1_RELAY).toInt(), config.get(KEY_OUTPUT1_RELAY_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY))
    bypassRelayO2.begin(config.get(KEY_PIN_OUTPUT2_RELAY).toInt(), config.get(KEY_OUTPUT2_RELAY_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_RELAY1))
    relay1.begin(config.get(KEY_PIN_RELAY1).toInt(), config.get(KEY_RELAY1_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_RELAY2))
    relay2.begin(config.get(KEY_PIN_RELAY2).toInt(), config.get(KEY_RELAY2_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);

  // Electricity: JSY
  if (config.getBool(KEY_ENABLE_JSY)) {
    jsy.begin(YASOLR_JSY_SERIAL, config.get(KEY_PIN_JSY_RX).toInt(), config.get(KEY_PIN_JSY_TX).toInt());
    if (jsy.isEnabled() && jsy.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400)
      jsy.setBaudRate(Mycila::JSYBaudRate::BAUD_38400);
  }

  // Electricity: PZEMs
  if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM))
    pzemO1.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT1);
  if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM))
    pzemO2.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT2);

  // Task Monitor
  Mycila::TaskMonitor.begin();
  Mycila::TaskMonitor.addTask("arduino_events");            // Network
  Mycila::TaskMonitor.addTask("async_tcp");                 // AsyncTCP
  Mycila::TaskMonitor.addTask("async_udp");                 // AsyncUDP
  Mycila::TaskMonitor.addTask("mqtt_task");                 // MQTT
  Mycila::TaskMonitor.addTask("wifi");                      // WiFI
  Mycila::TaskMonitor.addTask(coreTaskManager.getName());   // YaSolR
  Mycila::TaskMonitor.addTask(ioTaskManager.getName());     // YaSolR
  Mycila::TaskMonitor.addTask(routerTaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(jsyTaskManager.getName());    // YaSolR
  Mycila::TaskMonitor.addTask(pzemTaskManager.getName());   // YaSolR

  // Display
  if (config.getBool(KEY_ENABLE_DISPLAY)) {
    const String displayType = config.get(KEY_DISPLAY_TYPE);
    if (displayType == "SSD1306")
      display.begin(Mycila::EasyDisplayType::SSD1306, config.get(KEY_PIN_DISPLAY_SCL).toInt(), config.get(KEY_PIN_DISPLAY_SDA).toInt(), config.get(KEY_DISPLAY_ROTATION).toInt());
    else if (displayType == "SH1107")
      display.begin(Mycila::EasyDisplayType::SH1107, config.get(KEY_PIN_DISPLAY_SCL).toInt(), config.get(KEY_PIN_DISPLAY_SDA).toInt(), config.get(KEY_DISPLAY_ROTATION).toInt());
    else if (displayType == "SH1106")
      display.begin(Mycila::EasyDisplayType::SH1106, config.get(KEY_PIN_DISPLAY_SCL).toInt(), config.get(KEY_PIN_DISPLAY_SDA).toInt(), config.get(KEY_DISPLAY_ROTATION).toInt());
    display.clearDisplay();
    display.setActive(true);
  }

  // WebSerial
#ifdef APP_MODEL_PRO
  WebSerial.setID(Mycila::AppInfo.firmware.c_str());
  WebSerial.setTitle((Mycila::AppInfo.name + " Web Console").c_str());
  WebSerial.setInput(false);
#endif
  WebSerial.setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD).c_str());
  WebSerial.begin(&webServer, "/console");
  logger.forwardTo(&WebSerial);

  // ElegantOTA
#ifdef APP_MODEL_PRO
  ElegantOTA.setID(Mycila::AppInfo.firmware.c_str());
  ElegantOTA.setTitle((Mycila::AppInfo.name + " Web Updater").c_str());
  ElegantOTA.setFWVersion(Mycila::AppInfo.version.c_str());
  ElegantOTA.setFirmwareMode(true);
  ElegantOTA.setFilesystemMode(false);
#endif
  ElegantOTA.setAutoReboot(false);
  ElegantOTA.begin(&webServer, YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD).c_str());

  // Dashboard
  dashboard.setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD).c_str());
#ifdef APP_MODEL_PRO
  dashboard.setTitle(Mycila::AppInfo.nameModel.c_str());
#endif

  // Network Manager
  ESPConnect.setAutoRestart(true);
  ESPConnect.setBlocking(false);
  ESPConnect.begin(webServer, Mycila::AppInfo.defaultHostname, Mycila::AppInfo.defaultSSID, config.get(KEY_ADMIN_PASSWORD), {config.get(KEY_WIFI_SSID), config.get(KEY_WIFI_PASSWORD), config.getBool(KEY_ENABLE_AP_MODE)});
});
