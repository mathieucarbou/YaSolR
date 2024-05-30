// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

Mycila::Task initConfigTask("Init Config", [](void* params) {
  logger.warn(TAG, "Configuring %s...", Mycila::AppInfo.nameModelVersion.c_str());

  // Tasks configuration
  carouselTask.setEnabledWhen([]() { return display.isEnabled(); });
  carouselTask.setInterval(8 * Mycila::TaskDuration::SECONDS);
  displayTask.setEnabledWhen([]() { return display.isEnabled(); });
  displayTask.setInterval(293 * Mycila::TaskDuration::MILLISECONDS);
  ds18Task.setEnabledWhen([]() { return ds18Sys.isEnabled() || ds18O1.isEnabled() || ds18O2.isEnabled(); });
  jsyTask.setEnabledWhen([]() { return jsy.isEnabled() && Mycila::Grid.isConnected(); });
  lightsTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  mqttPublishTask.setEnabledWhen([]() { return mqtt.isConnected(); });
  mqttPublishTask.setInterval(config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt() * Mycila::TaskDuration::SECONDS);
  profilerTask.setInterval(10 * Mycila::TaskDuration::SECONDS);
  pzemO1Task.setEnabledWhen([]() { return pzemO1.isEnabled() && pzemO1PairingTask.isPaused() && pzemO2PairingTask.isPaused() && Mycila::Grid.isConnected(); });
  pzemO2Task.setEnabledWhen([]() { return pzemO2.isEnabled() && pzemO1PairingTask.isPaused() && pzemO2PairingTask.isPaused() && Mycila::Grid.isConnected(); });
  relaysTask.setInterval(10 * Mycila::TaskDuration::SECONDS);
  routerTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  dashboardTask.setEnabledWhen([]() { return ESPConnect.isConnected() && dashboard.hasClient() && !dashboard.isAsyncAccessInProgress(); });
  dashboardTask.setInterval(751 * Mycila::TaskDuration::MILLISECONDS);
#ifdef APP_MODEL_TRIAL
  trialTask.setInterval(30 * Mycila::TaskDuration::SECONDS);
#endif

  // Grid
  Mycila::Grid.setFrequency(config.get(KEY_GRID_FREQUENCY).toInt() == 60 ? 60 : 50);
  Mycila::Grid.setVoltage(config.get(KEY_GRID_VOLTAGE).toInt());
  Mycila::Grid.setMQTTGridPowerTopic(config.get(KEY_GRID_POWER_MQTT_TOPIC));
  Mycila::Grid.setMQTTGridVoltageTopic(config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC));

  // NTP
  Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

  // Home Assistant Discovery
  haDiscovery.setDiscoveryTopic(config.get(KEY_HA_DISCOVERY_TOPIC));
  haDiscovery.setBaseTopic(config.get(KEY_MQTT_TOPIC));
  haDiscovery.setWillTopic(config.get(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC);
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
  if (config.getBool(KEY_ENABLE_DS18_SYSTEM))
    ds18Sys.begin(config.get(KEY_PIN_ROUTER_DS18).toInt());
  if (config.getBool(KEY_ENABLE_OUTPUT1_DS18))
    ds18O1.begin(config.get(KEY_PIN_OUTPUT1_DS18).toInt());
  if (config.getBool(KEY_ENABLE_OUTPUT2_DS18))
    ds18O2.begin(config.get(KEY_PIN_OUTPUT2_DS18).toInt());

  // Electricity: ZCD
  if (config.getBool(KEY_ENABLE_ZCD))
    Mycila::ZCD.begin(config.get(KEY_PIN_ZCD).toInt(), config.get(KEY_GRID_FREQUENCY).toInt() == 60 ? 60 : 50);

  // Electricity: Dimmers
  if (Mycila::ZCD.isEnabled()) {
    if (config.getBool(KEY_ENABLE_OUTPUT1))
      dimmerO1.begin(config.get(KEY_PIN_OUTPUT1_DIMMER).toInt());
    if (config.getBool(KEY_ENABLE_OUTPUT2))
      dimmerO2.begin(config.get(KEY_PIN_OUTPUT2_DIMMER).toInt());
  } else {
    logger.error(TAG, "Dimmers cannot be enabled because ZCD is not enabled");
  }

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
    jsy.begin(YASOLR_JSY_SERIAL, config.get(KEY_PIN_JSY_RX).toInt(), config.get(KEY_PIN_JSY_RT).toInt());
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
  Mycila::TaskMonitor.addTask("async_tcp");                 // AsyncTCP
  Mycila::TaskMonitor.addTask("mqtt_task");                 // MQTT
  Mycila::TaskMonitor.addTask(ioTaskManager.getName());     // YaSolR
  Mycila::TaskMonitor.addTask(jsyTaskManager.getName());    // YaSolR
  Mycila::TaskMonitor.addTask(loopTaskManager.getName());   // Arduino
  Mycila::TaskMonitor.addTask(pzemO1TaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(pzemO2TaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(routerTaskManager.getName()); // YaSolR

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
  WebSerial.begin(webServer, "/console", YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));
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
