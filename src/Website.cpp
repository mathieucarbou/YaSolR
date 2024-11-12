// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolRWebsite.h>

#include <unordered_map>

#define HIDDEN_PWD "********"

#ifdef APP_MODEL_PRO
static const ChartSize chartSize = {.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12};
#endif

void YaSolR::WebsiteClass::initLayout() {
  logger.debug(TAG, "Initializing layout");

  for (int i = 0; i < YASOLR_GRAPH_POINTS; i++)
    _historyX[i] = i - YASOLR_GRAPH_POINTS;

  // overview

  _gridPowerHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);

  _outputBypassSwitch(_output1Bypass, output1);
  _outputDimmerSlider(_output1DimmerSlider, output1);

  _outputBypassSwitch(_output2Bypass, output2);
  _outputDimmerSlider(_output2DimmerSlider, output2);

  _relaySwitch(_relay1Switch, routerRelay1);
  _relaySwitch(_relay2Switch, routerRelay2);

  _output1PZEMSync.attachCallback([this](int32_t value) {
    pzemO1PairingTask.resume();
    _output1PZEMSync.update(!pzemO1PairingTask.isPaused());
    dashboard.refreshCard(&_output1PZEMSync);
  });

  _output2PZEMSync.attachCallback([this](int32_t value) {
    pzemO2PairingTask.resume();
    _output2PZEMSync.update(!pzemO2PairingTask.isPaused());
    dashboard.refreshCard(&_output2PZEMSync);
  });

  _resistanceCalibration.attachCallback([this](int32_t value) {
    config.set(KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_FALSE, false);
    config.set(KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_FALSE, false);
    config.set(KEY_OUTPUT1_DIMMER_LIMIT, "100", false);
    config.set(KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_FALSE, false);
    config.set(KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_FALSE, false);
    config.set(KEY_OUTPUT2_DIMMER_LIMIT, "100", false);

    router.beginCalibration([]() {
      config.set(KEY_OUTPUT1_RESISTANCE, String(router.getOutputs()[0]->config.calibratedResistance).c_str());
      config.set(KEY_OUTPUT2_RESISTANCE, String(router.getOutputs()[1]->config.calibratedResistance).c_str());
    });

    YaSolR::Website.initCards();
    mqttPublishConfigTask.resume();
    mqttPublishTask.requestEarlyRun();

    _resistanceCalibration.update(router.isCalibrationRunning());
    dashboard.refreshCard(&_resistanceCalibration);
  });

#ifdef APP_MODEL_PRO
  dashboard.setChartAnimations(false);

  // overview graphs

  _gridPowerHistory.setSize(chartSize);
  _routedPowerHistory.setSize(chartSize);
  _routerTHDiHistory.setSize(chartSize);

  // PID

  _pidInputHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _pidOutputHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _pidErrorHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _pidPTermHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _pidITermHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _pidDTermHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _pidSumHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);

  _pidInputHistory.setSize(chartSize);
  _pidOutputHistory.setSize(chartSize);
  _pidPTermHistory.setSize(chartSize);
  _pidITermHistory.setSize(chartSize);
  _pidDTermHistory.setSize(chartSize);
  _pidErrorHistory.setSize(chartSize);
  _pidSumHistory.setSize(chartSize);

  // output 1

  _output1State.setTab(&_output1Tab);
  _output1DS18State.setTab(&_output1Tab);
  _output1DimmerSlider.setTab(&_output1Tab);
  _output1DimmerSliderRO.setTab(&_output1Tab);
  _output1Bypass.setTab(&_output1Tab);
  _output1BypassRO.setTab(&_output1Tab);

  _output1Power.setTab(&_output1Tab);
  _output1PowerFactor.setTab(&_output1Tab);
  _output1THDi.setTab(&_output1Tab);
  _output1Energy.setTab(&_output1Tab);

  _output1ApparentPower.setTab(&_output1Tab);
  _output1Voltage.setTab(&_output1Tab);
  _output1Current.setTab(&_output1Tab);
  _output1Resistance.setTab(&_output1Tab);

  _output1BypassAuto.setTab(&_output1Tab);
  _output1DimmerAuto.setTab(&_output1Tab);
  _output1AutoStartTemp.setTab(&_output1Tab);
  _output1AutoStartTime.setTab(&_output1Tab);
  _output1AutoStartWDays.setTab(&_output1Tab);
  _output1AutoStoptTemp.setTab(&_output1Tab);
  _output1AutoStoptTime.setTab(&_output1Tab);
  _output1DimmerReservedExcess.setTab(&_output1Tab);
  _output1DimmerDutyLimiter.setTab(&_output1Tab);
  _output1DimmerTempLimiter.setTab(&_output1Tab);

  _boolConfig(_output1BypassAuto, KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  _boolConfig(_output1DimmerAuto, KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  _daysConfig(_output1AutoStartWDays, KEY_OUTPUT1_DAYS);
  _numConfig(_output1AutoStartTemp, KEY_OUTPUT1_TEMPERATURE_START);
  _numConfig(_output1AutoStoptTemp, KEY_OUTPUT1_TEMPERATURE_STOP);
  _numConfig(_output1DimmerTempLimiter, KEY_OUTPUT1_DIMMER_STOP_TEMP);
  _percentageSlider(_output1DimmerDutyLimiter, KEY_OUTPUT1_DIMMER_LIMIT);
  _percentageSlider(_output1DimmerReservedExcess, KEY_OUTPUT1_RESERVED_EXCESS);
  _textConfig(_output1AutoStartTime, KEY_OUTPUT1_TIME_START);
  _textConfig(_output1AutoStoptTime, KEY_OUTPUT1_TIME_STOP);

  // output 2

  _output2State.setTab(&_output2Tab);
  _output2DS18State.setTab(&_output2Tab);
  _output2DimmerSlider.setTab(&_output2Tab);
  _output2DimmerSliderRO.setTab(&_output2Tab);
  _output2Bypass.setTab(&_output2Tab);
  _output2BypassRO.setTab(&_output2Tab);

  _output2Power.setTab(&_output2Tab);
  _output2PowerFactor.setTab(&_output2Tab);
  _output2THDi.setTab(&_output2Tab);
  _output2Energy.setTab(&_output2Tab);

  _output2ApparentPower.setTab(&_output2Tab);
  _output2Voltage.setTab(&_output2Tab);
  _output2Current.setTab(&_output2Tab);
  _output2Resistance.setTab(&_output2Tab);

  _output2BypassAuto.setTab(&_output2Tab);
  _output2DimmerAuto.setTab(&_output2Tab);
  _output2AutoStartTemp.setTab(&_output2Tab);
  _output2AutoStartTime.setTab(&_output2Tab);
  _output2AutoStartWDays.setTab(&_output2Tab);
  _output2AutoStoptTemp.setTab(&_output2Tab);
  _output2AutoStoptTime.setTab(&_output2Tab);
  _output2DimmerReservedExcess.setTab(&_output2Tab);
  _output2DimmerDutyLimiter.setTab(&_output2Tab);
  _output2DimmerTempLimiter.setTab(&_output2Tab);

  _boolConfig(_output2BypassAuto, KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  _boolConfig(_output2DimmerAuto, KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  _daysConfig(_output2AutoStartWDays, KEY_OUTPUT2_DAYS);
  _numConfig(_output2AutoStartTemp, KEY_OUTPUT2_TEMPERATURE_START);
  _numConfig(_output2AutoStoptTemp, KEY_OUTPUT2_TEMPERATURE_STOP);
  _numConfig(_output2DimmerTempLimiter, KEY_OUTPUT2_DIMMER_STOP_TEMP);
  _percentageSlider(_output2DimmerDutyLimiter, KEY_OUTPUT2_DIMMER_LIMIT);
  _percentageSlider(_output2DimmerReservedExcess, KEY_OUTPUT2_RESERVED_EXCESS);
  _textConfig(_output2AutoStartTime, KEY_OUTPUT2_TIME_START);
  _textConfig(_output2AutoStoptTime, KEY_OUTPUT2_TIME_STOP);

  // relays

  _relay1Switch.setTab(&_relaysTab);
  _relay1SwitchRO.setTab(&_relaysTab);
  _relay2Switch.setTab(&_relaysTab);
  _relay2SwitchRO.setTab(&_relaysTab);

  // management

  _configBackup.setTab(&_managementTab);
  _configRestore.setTab(&_managementTab);
  _consoleLink.setTab(&_managementTab);
  _debugInfo.setTab(&_managementTab);
  _debugMode.setTab(&_managementTab);
  _safeBoot.setTab(&_managementTab);
  _reset.setTab(&_managementTab);
  _restart.setTab(&_managementTab);
  _energyReset.setTab(&_managementTab);

  _boolConfig(_debugMode, KEY_ENABLE_DEBUG);

  _energyReset.attachCallback([]() {
    jsy.resetEnergy();
    pzemO1.resetEnergy();
    pzemO2.resetEnergy();
  });
  _reset.attachCallback([]() { resetTask.resume(); });
  _restart.attachCallback([]() { restartTask.resume(); });
  _safeBoot.attachCallback([]() { safeBootTask.resume(); });

  // network (config)

  _adminPwd.setTab(&_networkConfigTab);
  _apMode.setTab(&_networkConfigTab);
  _ntpServer.setTab(&_networkConfigTab);
  _ntpSync.setTab(&_networkConfigTab);
  _ntpTimezone.setTab(&_networkConfigTab);
  _wifiPwd.setTab(&_networkConfigTab);
  _wifiSSID.setTab(&_networkConfigTab);
  _staticIP.setTab(&_networkConfigTab);
  _subnetMask.setTab(&_networkConfigTab);
  _gateway.setTab(&_networkConfigTab);
  _dnsServer.setTab(&_networkConfigTab);

  _boolConfig(_apMode, KEY_ENABLE_AP_MODE);
  _passwordConfig(_adminPwd, KEY_ADMIN_PASSWORD);
  _passwordConfig(_wifiPwd, KEY_WIFI_PASSWORD);
  _textConfig(_ntpServer, KEY_NTP_SERVER);
  _textConfig(_ntpTimezone, KEY_NTP_TIMEZONE);
  _textConfig(_wifiSSID, KEY_WIFI_SSID);
  _textConfig(_staticIP, KEY_NET_IP);
  _textConfig(_subnetMask, KEY_NET_SUBNET);
  _textConfig(_gateway, KEY_NET_GATEWAY);
  _textConfig(_dnsServer, KEY_NET_DNS);

  _ntpSync.attachCallback([](const char* value) {
    const String str = String(value);
    const size_t len = str.length();
    const timeval tv = {str.substring(0, len - 3).toInt(), str.substring(len - 3).toInt()};
    Mycila::NTP.sync(tv);
  });

  // mqtt (config)

  _haDiscovery.setTab(&_mqttConfigTab);
  _haDiscoveryTopic.setTab(&_mqttConfigTab);
  _mqttGridPower.setTab(&_mqttConfigTab);
  _mqttGridVoltage.setTab(&_mqttConfigTab);
  _mqttPort.setTab(&_mqttConfigTab);
  _mqttPublishInterval.setTab(&_mqttConfigTab);
  _mqttPwd.setTab(&_mqttConfigTab);
  _mqttSecured.setTab(&_mqttConfigTab);
  _mqttServer.setTab(&_mqttConfigTab);
  _mqttServerCert.setTab(&_mqttConfigTab);
  _mqttServerCertDelete.setTab(&_mqttConfigTab);
  _mqttTopic.setTab(&_mqttConfigTab);
  _mqttUser.setTab(&_mqttConfigTab);
  _mqttTempO1.setTab(&_mqttConfigTab);
  _mqttTempO2.setTab(&_mqttConfigTab);

  _boolConfig(_haDiscovery, KEY_ENABLE_HA_DISCOVERY);
  _boolConfig(_mqttSecured, KEY_MQTT_SECURED);
  _numConfig(_mqttPort, KEY_MQTT_PORT);
  _passwordConfig(_mqttPwd, KEY_MQTT_PASSWORD);
  _sliderConfig(_mqttPublishInterval, KEY_MQTT_PUBLISH_INTERVAL);
  _textConfig(_haDiscoveryTopic, KEY_HA_DISCOVERY_TOPIC);
  _textConfig(_mqttGridPower, KEY_GRID_POWER_MQTT_TOPIC);
  _textConfig(_mqttGridVoltage, KEY_GRID_VOLTAGE_MQTT_TOPIC);
  _textConfig(_mqttTempO1, KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  _textConfig(_mqttTempO2, KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  _textConfig(_mqttServer, KEY_MQTT_SERVER);
  _textConfig(_mqttTopic, KEY_MQTT_TOPIC);
  _textConfig(_mqttUser, KEY_MQTT_USERNAME);

  _mqttServerCertDelete.attachCallback([this]() {
    if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE) && LittleFS.remove(YASOLR_MQTT_SERVER_CERT_FILE)) {
      logger.warn(TAG, "MQTT server certificate deleted successfully!");
      initCards();
      dashboardTask.requestEarlyRun();
    }
  });

  // GPIO (configuration)

  _pinDimmerO1.setTab(&_pinConfigTab);
  _pinDimmerO2.setTab(&_pinConfigTab);
  _pinDisplayClock.setTab(&_pinConfigTab);
  _pinDisplayData.setTab(&_pinConfigTab);
  _pinDS18O1.setTab(&_pinConfigTab);
  _pinDS18O2.setTab(&_pinConfigTab);
  _pinDS18Router.setTab(&_pinConfigTab);
  _pinJsyRX.setTab(&_pinConfigTab);
  _pinJsyTX.setTab(&_pinConfigTab);
  _pinLEDGreen.setTab(&_pinConfigTab);
  _pinLEDRed.setTab(&_pinConfigTab);
  _pinLEDYellow.setTab(&_pinConfigTab);
  _pinPZEMRX.setTab(&_pinConfigTab);
  _pinPZEMTX.setTab(&_pinConfigTab);
  _pinRelay1.setTab(&_pinConfigTab);
  _pinRelay2.setTab(&_pinConfigTab);
  _pinRelayO1.setTab(&_pinConfigTab);
  _pinRelayO2.setTab(&_pinConfigTab);
  _pinZCD.setTab(&_pinConfigTab);

  _pinConfig(_pinDimmerO1, KEY_PIN_OUTPUT1_DIMMER);
  _pinConfig(_pinDimmerO2, KEY_PIN_OUTPUT2_DIMMER);
  _pinConfig(_pinDisplayClock, KEY_PIN_DISPLAY_SCL);
  _pinConfig(_pinDisplayData, KEY_PIN_DISPLAY_SDA);
  _pinConfig(_pinDS18O1, KEY_PIN_OUTPUT1_DS18);
  _pinConfig(_pinDS18O2, KEY_PIN_OUTPUT2_DS18);
  _pinConfig(_pinDS18Router, KEY_PIN_ROUTER_DS18);
  _pinConfig(_pinJsyRX, KEY_PIN_JSY_RX);
  _pinConfig(_pinJsyTX, KEY_PIN_JSY_TX);
  _pinConfig(_pinLEDGreen, KEY_PIN_LIGHTS_GREEN);
  _pinConfig(_pinLEDRed, KEY_PIN_LIGHTS_RED);
  _pinConfig(_pinLEDYellow, KEY_PIN_LIGHTS_YELLOW);
  _pinConfig(_pinPZEMRX, KEY_PIN_PZEM_RX);
  _pinConfig(_pinPZEMTX, KEY_PIN_PZEM_TX);
  _pinConfig(_pinRelay1, KEY_PIN_RELAY1);
  _pinConfig(_pinRelay2, KEY_PIN_RELAY2);
  _pinConfig(_pinRelayO1, KEY_PIN_OUTPUT1_RELAY);
  _pinConfig(_pinRelayO2, KEY_PIN_OUTPUT2_RELAY);
  _pinConfig(_pinZCD, KEY_PIN_ZCD);

  // Hardware

  _display.setTab(&_hardwareEnableTab);
  _jsy.setTab(&_hardwareEnableTab);
  _led.setTab(&_hardwareEnableTab);
  _mqtt.setTab(&_hardwareEnableTab);
  _output1Dimmer.setTab(&_hardwareEnableTab);
  _output1PZEM.setTab(&_hardwareEnableTab);
  _output1Relay.setTab(&_hardwareEnableTab);
  _output1DS18.setTab(&_hardwareEnableTab);
  _output2Dimmer.setTab(&_hardwareEnableTab);
  _output2PZEM.setTab(&_hardwareEnableTab);
  _output2Relay.setTab(&_hardwareEnableTab);
  _output2DS18.setTab(&_hardwareEnableTab);
  _relay1.setTab(&_hardwareEnableTab);
  _relay2.setTab(&_hardwareEnableTab);
  _routerDS18.setTab(&_hardwareEnableTab);
  _zcd.setTab(&_hardwareEnableTab);

  _boolConfig(_display, KEY_ENABLE_DISPLAY);
  _boolConfig(_jsy, KEY_ENABLE_JSY);
  _boolConfig(_led, KEY_ENABLE_LIGHTS);
  _boolConfig(_mqtt, KEY_ENABLE_MQTT);
  _boolConfig(_output1Dimmer, KEY_ENABLE_OUTPUT1_DIMMER);
  _boolConfig(_output1DS18, KEY_ENABLE_OUTPUT1_DS18);
  _boolConfig(_output1PZEM, KEY_ENABLE_OUTPUT1_PZEM);
  _boolConfig(_output1Relay, KEY_ENABLE_OUTPUT1_RELAY);
  _boolConfig(_output2Dimmer, KEY_ENABLE_OUTPUT2_DIMMER);
  _boolConfig(_output2DS18, KEY_ENABLE_OUTPUT2_DS18);
  _boolConfig(_output2PZEM, KEY_ENABLE_OUTPUT2_PZEM);
  _boolConfig(_output2Relay, KEY_ENABLE_OUTPUT2_RELAY);
  _boolConfig(_relay1, KEY_ENABLE_RELAY1);
  _boolConfig(_relay2, KEY_ENABLE_RELAY2);
  _boolConfig(_routerDS18, KEY_ENABLE_DS18_SYSTEM);
  _boolConfig(_zcd, KEY_ENABLE_ZCD);

  // Hardware (config)

  _gridFreq.setTab(&_hardwareConfigTab);
  _displayRotation.setTab(&_hardwareConfigTab);
  _displayType.setTab(&_hardwareConfigTab);
  _displaySpeed.setTab(&_hardwareConfigTab);
  _output1PZEMSync.setTab(&_hardwareConfigTab);
  _output2PZEMSync.setTab(&_hardwareConfigTab);
  _output1RelayType.setTab(&_hardwareConfigTab);
  _output2RelayType.setTab(&_hardwareConfigTab);
  _relay1Type.setTab(&_hardwareConfigTab);
  _relay2Type.setTab(&_hardwareConfigTab);
  _relay1Load.setTab(&_hardwareConfigTab);
  _relay2Load.setTab(&_hardwareConfigTab);
  _output1ResistanceInput.setTab(&_hardwareConfigTab);
  _output2ResistanceInput.setTab(&_hardwareConfigTab);
  _output1DimmerMapper.setTab(&_hardwareConfigTab);
  _output2DimmerMapper.setTab(&_hardwareConfigTab);
  _resistanceCalibration.setTab(&_hardwareConfigTab);

  _numConfig(_gridFreq, KEY_GRID_FREQUENCY);
  _numConfig(_displayRotation, KEY_DISPLAY_ROTATION);
  _numConfig(_relay1Load, KEY_RELAY1_LOAD);
  _numConfig(_relay2Load, KEY_RELAY2_LOAD);
  _textConfig(_displayType, KEY_DISPLAY_TYPE);
  _floatConfig(_output1ResistanceInput, KEY_OUTPUT1_RESISTANCE);
  _floatConfig(_output2ResistanceInput, KEY_OUTPUT2_RESISTANCE);
  _textConfig(_output1RelayType, KEY_OUTPUT1_RELAY_TYPE);
  _textConfig(_output2RelayType, KEY_OUTPUT2_RELAY_TYPE);
  _textConfig(_relay1Type, KEY_RELAY1_TYPE);
  _textConfig(_relay2Type, KEY_RELAY2_TYPE);
  _sliderConfig(_displaySpeed, KEY_DISPLAY_SPEED);

  _output1DimmerMapper.attachCallback([this](const char* value) {
    const char* comma = strchr(value, ',');
    if (comma != nullptr) {
      config.set(KEY_OUTPUT1_DIMMER_MIN, String(value).substring(0, comma - value).c_str());
      config.set(KEY_OUTPUT1_DIMMER_MAX, comma + 1);
    }
    _output1DimmerMapper.update(value);
    dashboard.refreshCard(&_output1DimmerMapper);
  });
  _output2DimmerMapper.attachCallback([this](const char* value) {
    const char* comma = strchr(value, ',');
    if (comma != nullptr) {
      config.set(KEY_OUTPUT2_DIMMER_MIN, String(value).substring(0, comma - value).c_str());
      config.set(KEY_OUTPUT2_DIMMER_MAX, comma + 1);
    }
    _output2DimmerMapper.update(value);
    dashboard.refreshCard(&_output2DimmerMapper);
  });

  // PID

  _pidView.setTab(&_pidTab);
  _pidPMode.setTab(&_pidTab);
  _pidDMode.setTab(&_pidTab);
  _pidICMode.setTab(&_pidTab);
  _pidSetpoint.setTab(&_pidTab);
  _pidKp.setTab(&_pidTab);
  _pidKi.setTab(&_pidTab);
  _pidKd.setTab(&_pidTab);
  _pidOutMin.setTab(&_pidTab);
  _pidOutMax.setTab(&_pidTab);

  _pidInputHistory.setTab(&_pidTab);
  _pidOutputHistory.setTab(&_pidTab);
  _pidErrorHistory.setTab(&_pidTab);
  _pidSumHistory.setTab(&_pidTab);
  _pidPTermHistory.setTab(&_pidTab);
  _pidITermHistory.setTab(&_pidTab);
  _pidDTermHistory.setTab(&_pidTab);
  _pidReset.setTab(&_pidTab);

  _pidReset.attachCallback([this]() {
    resetPID();
    updatePID();
  });

  _boolConfig(_pidView, KEY_ENABLE_PID_VIEW);
  _numConfig(_pidPMode, KEY_PID_P_MODE);
  _numConfig(_pidDMode, KEY_PID_D_MODE);
  _numConfig(_pidICMode, KEY_PID_IC_MODE);
  _numConfig(_pidSetpoint, KEY_PID_SETPOINT);
  _floatConfig(_pidKp, KEY_PID_KP);
  _floatConfig(_pidKi, KEY_PID_KI);
  _floatConfig(_pidKd, KEY_PID_KD);
  _numConfig(_pidOutMin, KEY_PID_OUT_MIN);
  _numConfig(_pidOutMax, KEY_PID_OUT_MAX);
#endif
}

void YaSolR::WebsiteClass::initCards() {
  logger.debug(TAG, "Initializing cards");

  // Statistics

  _appManufacturer.set(Mycila::AppInfo.manufacturer.c_str());
  _appModel.set((Mycila::AppInfo.model.c_str()));
  _appName.set((Mycila::AppInfo.name.c_str()));
  _appVersion.set(Mycila::AppInfo.version.c_str());
  _deviceBootCount.set(String(Mycila::System::getBootCount()).c_str());
  _deviceBootReason.set(Mycila::System::getLastRebootReason());
  _deviceCores.set(String(ESP.getChipCores()).c_str());
  _deviceModel.set(ESP.getChipModel());
  _deviceRev.set(String(ESP.getChipRevision()).c_str());
  _deviceID.set(Mycila::AppInfo.id.c_str());
  _firmwareBuildHash.set(Mycila::AppInfo.buildHash.c_str());
  _firmwareBuildTimestamp.set(Mycila::AppInfo.buildDate.c_str());
  _firmwareFilename.set(Mycila::AppInfo.firmware.c_str());
  _networkAPMAC.set(espConnect.getMACAddress(Mycila::ESPConnect::Mode::AP).c_str());
  _networkEthMAC.set(espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH).empty() ? "N/A" : espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH).c_str());
  _networkHostname.set(Mycila::AppInfo.defaultHostname.c_str());
  _networkWiFiMAC.set(espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA).c_str());

#ifdef APP_MODEL_PRO
  const bool jsyEnabled = config.getBool(KEY_ENABLE_JSY);

  // output 1 (control)

  const bool dimmer1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_DIMMER);
  const bool output1RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT1_RELAY);
  const bool bypass1Possible = dimmer1Enabled || output1RelayEnabled;
  const bool autoDimmer1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  const bool autoBypass1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  const bool output1TempEnabled = config.getBool(KEY_ENABLE_OUTPUT1_DS18) || config.isEmpty(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  const bool pzem1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_PZEM);

  _output1DimmerAuto.update(autoDimmer1Activated);
  _output1DimmerReservedExcess.update(config.getInt(KEY_OUTPUT1_RESERVED_EXCESS));
  _output1DimmerDutyLimiter.update(config.getInt(KEY_OUTPUT1_DIMMER_LIMIT));
  _output1DimmerTempLimiter.update(config.get(KEY_OUTPUT1_DIMMER_STOP_TEMP));
  _output1BypassAuto.update(autoBypass1Activated);
  _output1AutoStartWDays.update(config.get(KEY_OUTPUT1_DAYS));
  _output1AutoStartTemp.update(config.get(KEY_OUTPUT1_TEMPERATURE_START));
  _output1AutoStartTime.update(config.get(KEY_OUTPUT1_TIME_START));
  _output1AutoStoptTemp.update(config.get(KEY_OUTPUT1_TEMPERATURE_STOP));
  _output1AutoStoptTime.update(config.get(KEY_OUTPUT1_TIME_STOP));

  _output1Tab.setDisplay(dimmer1Enabled || output1TempEnabled || output1RelayEnabled);
  _output1DimmerSlider.setDisplay(dimmer1Enabled && !autoDimmer1Activated);
  _output1DimmerSliderRO.setDisplay(dimmer1Enabled && autoDimmer1Activated);
  _output1Bypass.setDisplay(bypass1Possible && !autoBypass1Activated);
  _output1BypassRO.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1Power.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1ApparentPower.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1PowerFactor.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1THDi.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Voltage.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Current.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Resistance.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Energy.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1DimmerAuto.setDisplay(dimmer1Enabled);
  _output1DimmerReservedExcess.setDisplay(dimmer1Enabled && autoDimmer1Activated);
  _output1DimmerDutyLimiter.setDisplay(dimmer1Enabled);
  _output1DimmerTempLimiter.setDisplay(dimmer1Enabled && output1TempEnabled);
  _output1BypassAuto.setDisplay(bypass1Possible);
  _output1AutoStartWDays.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1AutoStartTime.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1AutoStoptTime.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1AutoStartTemp.setDisplay(bypass1Possible && autoBypass1Activated && output1TempEnabled);
  _output1AutoStoptTemp.setDisplay(bypass1Possible && autoBypass1Activated && output1TempEnabled);

  // output 2 (control)

  const bool dimmer2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_DIMMER);
  const bool output2RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT2_RELAY);
  const bool bypass2Possible = dimmer2Enabled || output2RelayEnabled;
  const bool autoDimmer2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  const bool autoBypass2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  const bool output2TempEnabled = config.getBool(KEY_ENABLE_OUTPUT2_DS18) || !config.isEmpty(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  const bool pzem2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_PZEM);

  _output2DimmerAuto.update(autoDimmer2Activated);
  _output2DimmerReservedExcess.update(config.getInt(KEY_OUTPUT2_RESERVED_EXCESS));
  _output2DimmerDutyLimiter.update(config.getInt(KEY_OUTPUT2_DIMMER_LIMIT));
  _output2DimmerTempLimiter.update(config.get(KEY_OUTPUT2_DIMMER_STOP_TEMP));
  _output2BypassAuto.update(autoBypass2Activated);
  _output2AutoStartWDays.update(config.get(KEY_OUTPUT2_DAYS));
  _output2AutoStartTemp.update(config.get(KEY_OUTPUT2_TEMPERATURE_START));
  _output2AutoStartTime.update(config.get(KEY_OUTPUT2_TIME_START));
  _output2AutoStoptTemp.update(config.get(KEY_OUTPUT2_TEMPERATURE_STOP));
  _output2AutoStoptTime.update(config.get(KEY_OUTPUT2_TIME_STOP));

  _output2Tab.setDisplay(dimmer2Enabled || output2TempEnabled || output2RelayEnabled);
  _output2DimmerSlider.setDisplay(dimmer2Enabled && !autoDimmer2Activated);
  _output2DimmerSliderRO.setDisplay(dimmer2Enabled && autoDimmer2Activated);
  _output2Bypass.setDisplay(bypass2Possible && !autoBypass2Activated);
  _output2BypassRO.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2Power.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2ApparentPower.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2PowerFactor.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2THDi.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Voltage.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Current.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Resistance.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Energy.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2DimmerAuto.setDisplay(dimmer2Enabled);
  _output2DimmerReservedExcess.setDisplay(dimmer2Enabled && autoDimmer2Activated);
  _output2DimmerDutyLimiter.setDisplay(dimmer2Enabled);
  _output2DimmerTempLimiter.setDisplay(dimmer2Enabled && output2TempEnabled);
  _output2BypassAuto.setDisplay(bypass2Possible);
  _output2AutoStartWDays.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2AutoStartTime.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2AutoStoptTime.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2AutoStartTemp.setDisplay(bypass2Possible && autoBypass2Activated && output2TempEnabled);
  _output2AutoStoptTemp.setDisplay(bypass2Possible && autoBypass2Activated && output2TempEnabled);

  // relays (control)

  const int load1 = config.getInt(KEY_RELAY1_LOAD);
  const int load2 = config.getInt(KEY_RELAY2_LOAD);
  const bool relay1Enabled = config.getBool(KEY_ENABLE_RELAY1);
  const bool relay2Enabled = config.getBool(KEY_ENABLE_RELAY2);
  _relaysTab.setDisplay(relay1Enabled || relay2Enabled);
  _relay1Switch.setDisplay(relay1Enabled && load1 <= 0);
  _relay1SwitchRO.setDisplay(relay1Enabled && load1 > 0);
  _relay2Switch.setDisplay(relay2Enabled && load2 <= 0);
  _relay2SwitchRO.setDisplay(relay2Enabled && load2 > 0);

  // management

  _configBackup.update("/api/config/backup");
  _configRestore.update("/api/config/restore");
  _consoleLink.update("/console");
  _debugInfo.update("/api/debug");
  _debugMode.update(config.getBool(KEY_ENABLE_DEBUG));
  _energyReset.setDisplay(jsyEnabled || pzem1Enabled || pzem2Enabled);
  _debugInfo.setDisplay(config.getBool(KEY_ENABLE_DEBUG));

  // network (config)

  _adminPwd.update(config.isEmpty(KEY_ADMIN_PASSWORD) ? "" : HIDDEN_PWD);
  _apMode.update(config.getBool(KEY_ENABLE_AP_MODE));
  _ntpServer.update(config.get(KEY_NTP_SERVER));
  _ntpTimezone.update(config.get(KEY_NTP_TIMEZONE), "/timezones");
  _wifiPwd.update(config.isEmpty(KEY_WIFI_PASSWORD) ? "" : HIDDEN_PWD);
  _wifiSSID.update(config.get(KEY_WIFI_SSID));
  _staticIP.update(config.get(KEY_NET_IP));
  _subnetMask.update(config.get(KEY_NET_SUBNET));
  _gateway.update(config.get(KEY_NET_GATEWAY));
  _dnsServer.update(config.get(KEY_NET_DNS));

  // mqtt (config)

  _haDiscovery.update(config.getBool(KEY_ENABLE_HA_DISCOVERY));
  _haDiscoveryTopic.update(config.get(KEY_HA_DISCOVERY_TOPIC));
  _mqttGridPower.update(config.get(KEY_GRID_POWER_MQTT_TOPIC));
  _mqttGridVoltage.update(config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC));
  _mqttTempO1.update(config.get(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC));
  _mqttTempO2.update(config.get(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC));
  _mqttPort.update(config.get(KEY_MQTT_PORT));
  _mqttPublishInterval.update(config.get(KEY_MQTT_PUBLISH_INTERVAL));
  _mqttPwd.update(config.isEmpty(KEY_MQTT_PASSWORD) ? "" : HIDDEN_PWD);
  _mqttSecured.update(config.getBool(KEY_MQTT_SECURED));
  _mqttServer.update(config.get(KEY_MQTT_SERVER));
  _mqttServerCert.update("/api/config/mqttServerCertificate");
  _mqttTopic.update(config.get(KEY_MQTT_TOPIC));
  _mqttUser.update(config.get(KEY_MQTT_USERNAME));

  const bool serverCertExists = LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE);
  _mqttConfigTab.setDisplay(config.getBool(KEY_ENABLE_MQTT));
  _mqttServerCert.setDisplay(!serverCertExists);
  _mqttServerCertDelete.setDisplay(serverCertExists);

  // GPIO

  std::unordered_map<int32_t, Card*> pinout = {};
  _pinout(_pinDimmerO1, config.getLong(KEY_PIN_OUTPUT1_DIMMER), pinout);
  _pinout(_pinDimmerO2, config.getLong(KEY_PIN_OUTPUT2_DIMMER), pinout);
  _pinout(_pinDisplayClock, config.getLong(KEY_PIN_DISPLAY_SCL), pinout);
  _pinout(_pinDisplayData, config.getLong(KEY_PIN_DISPLAY_SDA), pinout);
  _pinout(_pinDS18O1, config.getLong(KEY_PIN_OUTPUT1_DS18), pinout);
  _pinout(_pinDS18O2, config.getLong(KEY_PIN_OUTPUT2_DS18), pinout);
  _pinout(_pinDS18Router, config.getLong(KEY_PIN_ROUTER_DS18), pinout);
  _pinout(_pinJsyRX, config.getLong(KEY_PIN_JSY_RX), pinout);
  _pinout(_pinJsyTX, config.getLong(KEY_PIN_JSY_TX), pinout);
  _pinout(_pinLEDGreen, config.getLong(KEY_PIN_LIGHTS_GREEN), pinout);
  _pinout(_pinLEDRed, config.getLong(KEY_PIN_LIGHTS_RED), pinout);
  _pinout(_pinLEDYellow, config.getLong(KEY_PIN_LIGHTS_YELLOW), pinout);
  _pinout(_pinPZEMRX, config.getLong(KEY_PIN_PZEM_RX), pinout);
  _pinout(_pinPZEMTX, config.getLong(KEY_PIN_PZEM_TX), pinout);
  _pinout(_pinRelay1, config.getLong(KEY_PIN_RELAY1), pinout);
  _pinout(_pinRelay2, config.getLong(KEY_PIN_RELAY2), pinout);
  _pinout(_pinRelayO1, config.getLong(KEY_PIN_OUTPUT1_RELAY), pinout);
  _pinout(_pinRelayO2, config.getLong(KEY_PIN_OUTPUT2_RELAY), pinout);
  _pinout(_pinZCD, config.getLong(KEY_PIN_ZCD), pinout);
  pinout.clear();

  // Hardware

  _status(_display, KEY_ENABLE_DISPLAY, display.isEnabled());
  _status(_led, KEY_ENABLE_LIGHTS, lights.isEnabled());
  _status(_output1Relay, KEY_ENABLE_OUTPUT1_RELAY, bypassRelayO1.isEnabled());
  _status(_output2Relay, KEY_ENABLE_OUTPUT2_RELAY, bypassRelayO2.isEnabled());
  _status(_relay1, KEY_ENABLE_RELAY1, relay1.isEnabled());
  _status(_relay2, KEY_ENABLE_RELAY2, relay2.isEnabled());

  // Hardware (config)

  switch (config.getLong(KEY_GRID_FREQUENCY)) {
    case 50:
      _gridFreq.update("50 Hz", "Auto-detect,50 Hz,60 Hz");
      break;
    case 60:
      _gridFreq.update("60 Hz", "Auto-detect,50 Hz,60 Hz");
      break;
    default:
      _gridFreq.update("Auto-detect", "Auto-detect,50 Hz,60 Hz");
      break;
  }

  _displayType.update(config.get(KEY_DISPLAY_TYPE), "SH1106,SH1107,SSD1306");
  _displaySpeed.update(config.getInt(KEY_DISPLAY_SPEED));
  _displayRotation.update((String(config.get(KEY_DISPLAY_ROTATION))) + "°", "0°,90°,180°,270°");
  _output1RelayType.update(config.get(KEY_OUTPUT1_RELAY_TYPE), "NO,NC");
  _output2RelayType.update(config.get(KEY_OUTPUT2_RELAY_TYPE), "NO,NC");
  _relay1Type.update(config.get(KEY_RELAY1_TYPE), "NO,NC");
  _relay2Type.update(config.get(KEY_RELAY2_TYPE), "NO,NC");
  _relay1Load.update(load1);
  _relay2Load.update(load2);
  _output1ResistanceInput.update(config.get(KEY_OUTPUT1_RESISTANCE), config.getFloat(KEY_OUTPUT1_RESISTANCE) == 0 ? DASH_STATUS_DANGER : DASH_STATUS_SUCCESS);
  _output2ResistanceInput.update(config.get(KEY_OUTPUT2_RESISTANCE), config.getFloat(KEY_OUTPUT2_RESISTANCE) == 0 ? DASH_STATUS_DANGER : DASH_STATUS_SUCCESS);
  _output1DimmerMapper.update(String(config.get(KEY_OUTPUT1_DIMMER_MIN)) + "," + config.get(KEY_OUTPUT1_DIMMER_MAX));
  _output2DimmerMapper.update(String(config.get(KEY_OUTPUT2_DIMMER_MIN)) + "," + config.get(KEY_OUTPUT2_DIMMER_MAX));

  _displayType.setDisplay(config.getBool(KEY_ENABLE_DISPLAY));
  _displaySpeed.setDisplay(config.getBool(KEY_ENABLE_DISPLAY));
  _displayRotation.setDisplay(config.getBool(KEY_ENABLE_DISPLAY));
  _relay1Load.setDisplay(relay1Enabled);
  _relay2Load.setDisplay(relay2Enabled);
  _output1ResistanceInput.setDisplay(dimmer1Enabled);
  _output2ResistanceInput.setDisplay(dimmer2Enabled);
  _output1PZEMSync.setDisplay(pzem1Enabled);
  _output2PZEMSync.setDisplay(pzem2Enabled);
  _output1RelayType.setDisplay(config.getBool(KEY_ENABLE_OUTPUT1_RELAY));
  _output2RelayType.setDisplay(config.getBool(KEY_ENABLE_OUTPUT2_RELAY));
  _relay1Type.setDisplay(relay1Enabled);
  _relay2Type.setDisplay(relay2Enabled);
  _output1DimmerMapper.setDisplay(dimmer1Enabled);
  _output2DimmerMapper.setDisplay(dimmer2Enabled);
  _resistanceCalibration.setDisplay((dimmer1Enabled && jsyEnabled) ||
                                    (dimmer1Enabled && pzem1Enabled) ||
                                    (dimmer2Enabled && jsyEnabled) ||
                                    (dimmer2Enabled && pzem2Enabled));

  // PID

  const bool pidViewEnabled = config.getBool(KEY_ENABLE_PID_VIEW);
  _pidView.update(pidViewEnabled);
  switch (config.getLong(KEY_PID_P_MODE)) {
    case 1:
      _pidPMode.update(YASOLR_PID_P_MODE_1, YASOLR_PID_P_MODE_1 "," YASOLR_PID_P_MODE_2 "," YASOLR_PID_P_MODE_3);
      break;
    case 2:
      _pidPMode.update(YASOLR_PID_P_MODE_2, YASOLR_PID_P_MODE_1 "," YASOLR_PID_P_MODE_2 "," YASOLR_PID_P_MODE_3);
      break;
    case 3:
      _pidPMode.update(YASOLR_PID_P_MODE_3, YASOLR_PID_P_MODE_1 "," YASOLR_PID_P_MODE_2 "," YASOLR_PID_P_MODE_3);
      break;
    default:
      _pidPMode.update("", YASOLR_PID_P_MODE_1 "," YASOLR_PID_P_MODE_2 "," YASOLR_PID_P_MODE_3);
      break;
  }
  switch (config.getLong(KEY_PID_D_MODE)) {
    case 1:
      _pidDMode.update(YASOLR_PID_D_MODE_1, YASOLR_PID_D_MODE_1 "," YASOLR_PID_D_MODE_2);
      break;
    case 2:
      _pidDMode.update(YASOLR_PID_D_MODE_2, YASOLR_PID_D_MODE_1 "," YASOLR_PID_D_MODE_2);
      break;
    default:
      _pidDMode.update("", YASOLR_PID_D_MODE_1 "," YASOLR_PID_D_MODE_2);
      break;
  }
  switch (config.getLong(KEY_PID_IC_MODE)) {
    case 0:
      _pidICMode.update(YASOLR_PID_IC_MODE_0, YASOLR_PID_IC_MODE_0 "," YASOLR_PID_IC_MODE_1 "," YASOLR_PID_IC_MODE_2);
      break;
    case 1:
      _pidICMode.update(YASOLR_PID_IC_MODE_1, YASOLR_PID_IC_MODE_0 "," YASOLR_PID_IC_MODE_1 "," YASOLR_PID_IC_MODE_2);
      break;
    case 2:
      _pidICMode.update(YASOLR_PID_IC_MODE_2, YASOLR_PID_IC_MODE_0 "," YASOLR_PID_IC_MODE_1 "," YASOLR_PID_IC_MODE_2);
      break;
    default:
      _pidICMode.update("", YASOLR_PID_IC_MODE_0 "," YASOLR_PID_IC_MODE_1 "," YASOLR_PID_IC_MODE_2);
      break;
  }
  _pidSetpoint.update(config.get(KEY_PID_SETPOINT));
  _pidKp.update(config.get(KEY_PID_KP));
  _pidKi.update(config.get(KEY_PID_KI));
  _pidKd.update(config.get(KEY_PID_KD));
  _pidOutMin.update(config.get(KEY_PID_OUT_MIN));
  _pidOutMax.update(config.get(KEY_PID_OUT_MAX));

  _pidInputHistory.setDisplay(pidViewEnabled);
  _pidOutputHistory.setDisplay(pidViewEnabled);
  _pidErrorHistory.setDisplay(pidViewEnabled);
  _pidSumHistory.setDisplay(pidViewEnabled);
  _pidPTermHistory.setDisplay(pidViewEnabled);
  _pidITermHistory.setDisplay(pidViewEnabled);
  _pidDTermHistory.setDisplay(pidViewEnabled);
#endif
}

void YaSolR::WebsiteClass::updateCards() {
  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);

  Mycila::Router::Metrics routerMetrics;
  router.getMeasurements(routerMetrics);

  Mycila::RouterOutput::Metrics output1Measurements;
  output1.getMeasurements(output1Measurements);

  Mycila::RouterOutput::Metrics output2Measurements;
  output2.getMeasurements(output2Measurements);

  // stats
  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);
  Mycila::ESPConnect::Mode mode = espConnect.getMode();
  _output1RelaySwitchCount.set(String(bypassRelayO1.getSwitchCount()).c_str());
  _output2RelaySwitchCount.set(String(bypassRelayO2.getSwitchCount()).c_str());
  _deviceHeapTotal.set((String(memory.total) + " bytes").c_str());
  _deviceHeapUsed.set((String(memory.used) + " bytes").c_str());
  _deviceHeapUsage.set((String(memory.usage) + " %").c_str());
  _gridEnergy.set((String(gridMetrics.energy, 3) + " kWh").c_str());
  _gridEnergyReturned.set((String(gridMetrics.energyReturned, 3) + " kWh").c_str());
  _gridFrequency.set((String(detectGridFrequency(), 0) + " Hz").c_str());
  _networkAPIP.set(espConnect.getIPAddress(Mycila::ESPConnect::Mode::AP).toString().c_str());
  _networkEthIP.set(espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  _networkInterface.set(mode == Mycila::ESPConnect::Mode::AP ? "AP" : (mode == Mycila::ESPConnect::Mode::STA ? "WiFi" : (mode == Mycila::ESPConnect::Mode::ETH ? "Ethernet" : "")));
  _networkWiFiIP.set(espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  _networkWiFiRSSI.set((String(espConnect.getWiFiRSSI()) + " dBm").c_str());
  _networkWiFiSignal.set((String(espConnect.getWiFiSignalQuality()) + " %").c_str());
  _networkWiFiSSID.set(espConnect.getWiFiSSID().c_str());
  _relay1SwitchCount.set(String(relay1.getSwitchCount()).c_str());
  _relay2SwitchCount.set(String(relay2.getSwitchCount()).c_str());
  _udpMessageRateBuffer.set((String(udpMessageRateBuffer.rate()) + " msg/s").c_str());
  _time.set(Mycila::Time::getLocalStr().c_str());
  _uptime.set(Mycila::Time::toDHHMMSS(Mycila::System::getUptime()).c_str());
#ifdef APP_MODEL_TRIAL
  _trialRemainingTime.set(Mycila::Time::toDHHMMSS(Mycila::Trial.getRemaining()).c_str());
#endif

  // home

  _routerPower.update(routerMetrics.power);
  _routerApparentPower.update(routerMetrics.apparentPower);
  _routerPowerFactor.update(routerMetrics.powerFactor);
  _routerTHDi.update(routerMetrics.thdi * 100);
  _routerVoltage.update(gridMetrics.voltage);
  _routerCurrent.update(routerMetrics.current);
  _routerResistance.update(routerMetrics.resistance);
  _routerEnergy.update(routerMetrics.energy);

  _gridPower.update(gridMetrics.power);
  _temperature(_routerDS18State, ds18Sys);

  // output 1

  switch (output1.getState()) {
    case Mycila::RouterOutput::State::OUTPUT_DISABLED:
    case Mycila::RouterOutput::State::OUTPUT_IDLE:
      _output1State.update(output1.getStateName(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutput::State::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutput::State::OUTPUT_BYPASS_MANUAL:
      _output1State.update(output1.getStateName(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutput::State::OUTPUT_ROUTING:
      _output1State.update(output1.getStateName(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output1State.update(YASOLR_LBL_109, DASH_STATUS_DANGER);
      break;
  }
  _temperature(_output1DS18State, output1);
  _output1DimmerSlider.update(dimmerO1.getDutyCycle() * 100);
  _output1Bypass.update(output1.isBypassOn());

  // output 2

  switch (output2.getState()) {
    case Mycila::RouterOutput::State::OUTPUT_DISABLED:
    case Mycila::RouterOutput::State::OUTPUT_IDLE:
      _output2State.update(output2.getStateName(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutput::State::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutput::State::OUTPUT_BYPASS_MANUAL:
      _output2State.update(output2.getStateName(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutput::State::OUTPUT_ROUTING:
      _output2State.update(output2.getStateName(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output2State.update(YASOLR_LBL_109, DASH_STATUS_DANGER);
      break;
  }
  _temperature(_output2DS18State, output2);
  _output2DimmerSlider.update(dimmerO2.getDutyCycle() * 100);
  _output2Bypass.update(output2.isBypassOn());

  // relay

  _relay1Switch.update(relay1.isOn());
  _relay2Switch.update(relay2.isOn());

  // Hardware (config)

  _output1PZEMSync.update(!pzemO1PairingTask.isPaused());
  _output2PZEMSync.update(!pzemO2PairingTask.isPaused());
  _resistanceCalibration.update(router.isCalibrationRunning());

#ifdef APP_MODEL_PRO
  // Output 1

  _output1DimmerSliderRO.update(dimmerO1.getDutyCycle() * 100);
  _output1BypassRO.update(YASOLR_STATE(output1.isBypassOn()), output1.isBypassOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);
  _output1Power.update(output1Measurements.power);
  _output1ApparentPower.update(output1Measurements.apparentPower);
  _output1PowerFactor.update(output1Measurements.powerFactor);
  _output1THDi.update(output1Measurements.thdi * 100);
  _output1Voltage.update(output1Measurements.dimmedVoltage);
  _output1Current.update(output1Measurements.current);
  _output1Resistance.update(output1Measurements.resistance);
  _output1Energy.update(output1Measurements.energy);

  // output 2

  _output2DimmerSliderRO.update(dimmerO2.getDutyCycle() * 100);
  _output2BypassRO.update(YASOLR_STATE(output2.isBypassOn()), output2.isBypassOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);
  _output2Power.update(output2Measurements.power);
  _output2ApparentPower.update(output2Measurements.apparentPower);
  _output2PowerFactor.update(output2Measurements.powerFactor);
  _output2THDi.update(output2Measurements.thdi * 100);
  _output2Voltage.update(output2Measurements.dimmedVoltage);
  _output2Current.update(output2Measurements.current);
  _output2Resistance.update(output2Measurements.resistance);
  _output2Energy.update(output2Measurements.energy);

  // relays

  _relay1SwitchRO.update(YASOLR_STATE(relay1.isOn()), relay1.isOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);
  _relay2SwitchRO.update(YASOLR_STATE(relay2.isOn()), relay2.isOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);

  // Hardware (status)

  _status(_jsy, KEY_ENABLE_JSY, jsy.isEnabled(), jsy.isConnected(), YASOLR_LBL_110);
  _status(_mqtt, KEY_ENABLE_MQTT, mqtt.isEnabled(), mqtt.isConnected(), mqtt.getLastError() ? mqtt.getLastError() : YASOLR_LBL_113);
  _status(_output1Dimmer, KEY_ENABLE_OUTPUT1_DIMMER, dimmerO1.isEnabled(), pulseAnalyzer.isOnline(), pulseAnalyzer.isEnabled() ? YASOLR_LBL_110 : YASOLR_LBL_179);
  _status(_output1DS18, KEY_ENABLE_OUTPUT1_DS18, ds18O1.isEnabled(), ds18O1.getLastTime() > 0, YASOLR_LBL_114);
  _status(_output1PZEM, KEY_ENABLE_OUTPUT1_PZEM, pzemO1.isEnabled(), pzemO1.isConnected() && pzemO1.getDeviceAddress() == YASOLR_PZEM_ADDRESS_OUTPUT1, pzemO1.isConnected() ? YASOLR_LBL_180 : YASOLR_LBL_110);
  _status(_output2Dimmer, KEY_ENABLE_OUTPUT2_DIMMER, dimmerO2.isEnabled(), pulseAnalyzer.isOnline(), pulseAnalyzer.isEnabled() ? YASOLR_LBL_110 : YASOLR_LBL_179);
  _status(_output2DS18, KEY_ENABLE_OUTPUT2_DS18, ds18O2.isEnabled(), ds18O2.getLastTime() > 0, YASOLR_LBL_114);
  _status(_output2PZEM, KEY_ENABLE_OUTPUT2_PZEM, pzemO2.isEnabled(), pzemO2.isConnected() && pzemO2.getDeviceAddress() == YASOLR_PZEM_ADDRESS_OUTPUT2, pzemO2.isConnected() ? YASOLR_LBL_180 : YASOLR_LBL_110);
  _status(_routerDS18, KEY_ENABLE_DS18_SYSTEM, ds18Sys.isEnabled(), ds18Sys.getLastTime() > 0, YASOLR_LBL_114);
  _status(_zcd, KEY_ENABLE_ZCD, pulseAnalyzer.isEnabled(), pulseAnalyzer.isOnline(), YASOLR_LBL_110);
#endif
}

void YaSolR::WebsiteClass::updateCharts() {
  // read last metrics
  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);

  Mycila::Router::Metrics routerMetrics;
  router.getMeasurements(routerMetrics);

  // shift array
  constexpr size_t shift = sizeof(_gridPowerHistoryY) - sizeof(*_gridPowerHistoryY);
  memmove(&_gridPowerHistoryY[0], &_gridPowerHistoryY[1], shift);
  memmove(&_routedPowerHistoryY[0], &_routedPowerHistoryY[1], shift);
  memmove(&_routerTHDiHistoryY[0], &_routerTHDiHistoryY[1], shift);

  // set new value
  _gridPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = round(gridMetrics.power);
  _routedPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = round(routerMetrics.power);
  _routerTHDiHistoryY[YASOLR_GRAPH_POINTS - 1] = round(routerMetrics.thdi * 100);

  // update charts
  _gridPowerHistory.updateY(_gridPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.updateY(_routedPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.updateY(_routerTHDiHistoryY, YASOLR_GRAPH_POINTS);
}

void YaSolR::WebsiteClass::updatePID() {
#ifdef APP_MODEL_PRO
  // shift array
  constexpr size_t shift = sizeof(_pidInputHistoryY) - sizeof(*_pidInputHistoryY);
  memmove(&_pidInputHistoryY[0], &_pidInputHistoryY[1], shift);
  memmove(&_pidOutputHistoryY[0], &_pidOutputHistoryY[1], shift);
  memmove(&_pidErrorHistoryY[0], &_pidErrorHistoryY[1], shift);
  memmove(&_pidSumHistoryY[0], &_pidSumHistoryY[1], shift);
  memmove(&_pidPTermHistoryY[0], &_pidPTermHistoryY[1], shift);
  memmove(&_pidITermHistoryY[0], &_pidITermHistoryY[1], shift);
  memmove(&_pidDTermHistoryY[0], &_pidDTermHistoryY[1], shift);

  // set new values
  _pidInputHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getInput());
  _pidOutputHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getOutput());
  _pidErrorHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getError());
  _pidSumHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getSum());
  _pidPTermHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getPTerm());
  _pidITermHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getITerm());
  _pidDTermHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getDTerm());

  // update charts
  _pidInputHistory.updateY(_pidInputHistoryY, YASOLR_GRAPH_POINTS);
  _pidOutputHistory.updateY(_pidOutputHistoryY, YASOLR_GRAPH_POINTS);
  _pidErrorHistory.updateY(_pidErrorHistoryY, YASOLR_GRAPH_POINTS);
  _pidSumHistory.updateY(_pidSumHistoryY, YASOLR_GRAPH_POINTS);
  _pidPTermHistory.updateY(_pidPTermHistoryY, YASOLR_GRAPH_POINTS);
  _pidITermHistory.updateY(_pidITermHistoryY, YASOLR_GRAPH_POINTS);
  _pidDTermHistory.updateY(_pidDTermHistoryY, YASOLR_GRAPH_POINTS);
#endif
}

void YaSolR::WebsiteClass::resetPID() {
#ifdef APP_MODEL_PRO
  memset(_pidOutputHistoryY, 0, sizeof(_pidOutputHistoryY));
  memset(_pidInputHistoryY, 0, sizeof(_pidInputHistoryY));
  memset(_pidErrorHistoryY, 0, sizeof(_pidErrorHistoryY));
  memset(_pidSumHistoryY, 0, sizeof(_pidSumHistoryY));
  memset(_pidPTermHistoryY, 0, sizeof(_pidPTermHistoryY));
  memset(_pidITermHistoryY, 0, sizeof(_pidITermHistoryY));
  memset(_pidDTermHistoryY, 0, sizeof(_pidDTermHistoryY));
#endif
}

void YaSolR::WebsiteClass::_sliderConfig(Card& card, const char* key) {
  card.attachCallback([key, &card](int value) {
    config.set(key, String(value).c_str());
    card.update(config.getInt(key));
    dashboard.refreshCard(&card);
  });
}

void YaSolR::WebsiteClass::_percentageSlider(Card& card, const char* key) {
  card.attachCallback([key, &card](int value) {
    config.set(key, String(value).c_str());
    card.update(value);
    dashboard.refreshCard(&card);
  });
}

void YaSolR::WebsiteClass::_floatConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    if (strlen(value) == 0) {
      config.unset(key);
    } else {
      config.set(key, value);
    }
    card.update(config.get(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_numConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    if (strlen(value) == 0) {
      config.unset(key);
    } else {
      config.set(key, String(strtol(value, nullptr, 10)).c_str());
    }
    card.update(config.getInt(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_pinConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card, this](const char* value) {
    if (strlen(value) == 0) {
      config.unset(key);
    } else {
      config.set(key, String(strtol(value, nullptr, 10)).c_str());
    }
    initCards();
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_boolConfig(Card& card, const char* key) {
  card.attachCallback([key, &card, this](int value) {
    config.setBool(key, value);
    card.update(config.getBool(key) ? 1 : 0);
    dashboard.refreshCard(&card);
  });
}

void YaSolR::WebsiteClass::_textConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    config.set(key, value);
    card.update(config.get(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_daysConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card, this](const char* value) {
    config.set(key, strlen(value) == 0 ? "none" : value);
    card.update(config.get(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_passwordConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card, this](const char* value) {
    if (strlen(value) == 0) {
      config.unset(key);
    } else {
      config.set(key, value);
    }
    card.update(config.isEmpty(key) ? "" : HIDDEN_PWD);
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_relaySwitch(Card& card, Mycila::RouterRelay& relay) {
  card.attachCallback([&card, &relay, this](int value) {
    relay.tryRelayState(value);
    card.update(relay.isOn());
    dashboard.refreshCard(&card);
  });
}

void YaSolR::WebsiteClass::_outputBypassSwitch(Card& card, Mycila::RouterOutput& output) {
  card.attachCallback([&card, &output, this](int value) {
    if (output.isBypassEnabled()) {
      output.setBypass(value);
    }
    card.update(output.isBypassOn());
    dashboard.refreshCard(&card);
    dashboardTask.requestEarlyRun();
  });
}

void YaSolR::WebsiteClass::_outputDimmerSlider(Card& card, Mycila::RouterOutput& output) {
  card.attachCallbackF([&card, &output, this](float value) {
    if (output.isDimmerEnabled()) {
      output.setDimmerDutyCycle(value / 100);
    }
    card.update(output.getDimmerDutyCycle() * 100);
    dashboard.refreshCard(&card);
    dashboardTask.requestEarlyRun();
  });
}

void YaSolR::WebsiteClass::_temperature(Card& card, Mycila::DS18& sensor) {
  if (!sensor.isEnabled()) {
    card.update(YASOLR_LBL_115, "");
  } else if (!sensor.isValid()) {
    card.update(YASOLR_LBL_123, "");
  } else {
    card.update(sensor.getTemperature().value_or(0), "°C");
  }
}

void YaSolR::WebsiteClass::_temperature(Card& card, Mycila::RouterOutput& output) {
  if (output.temperature().neverUpdated()) {
    card.update(YASOLR_LBL_115, "");
  } else if (output.temperature().isAbsent()) {
    card.update(YASOLR_LBL_123, "");
  } else {
    card.update(output.temperature().get(), "°C");
  }
}

void YaSolR::WebsiteClass::_status(Card& card, const char* key, bool enabled, bool active, const char* err) {
  const bool configEnabled = config.getBool(key);
  if (!configEnabled)
    card.update(config.getBool(key), DASH_STATUS_IDLE "," YASOLR_LBL_115);
  else if (!enabled)
    card.update(config.getBool(key), DASH_STATUS_DANGER "," YASOLR_LBL_124);
  else if (!active)
    card.update(config.getBool(key), (String(DASH_STATUS_WARNING) + "," + err).c_str());
  else
    card.update(config.getBool(key), DASH_STATUS_SUCCESS "," YASOLR_LBL_130);
}

void YaSolR::WebsiteClass::_pinout(Card& card, int32_t pin, std::unordered_map<int32_t, Card*>& pinout) {
  if (pin == GPIO_NUM_NC) {
    card.update(YASOLR_LBL_115, DASH_STATUS_IDLE);
  } else if (pinout.find(pin) != pinout.end()) {
    String v = String(pin) + " (" YASOLR_LBL_153 ")";
    pinout[pin]->update(v, DASH_STATUS_DANGER);
    card.update(v, DASH_STATUS_DANGER);
  } else if (!GPIO_IS_VALID_GPIO(pin)) {
    pinout[pin] = &card;
    card.update(String(pin) + " (" YASOLR_LBL_154 ")", DASH_STATUS_DANGER);
  } else if (!GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    pinout[pin] = &card;
    card.update(String(pin) + " (" YASOLR_LBL_155 ")", DASH_STATUS_WARNING);
  } else {
    pinout[pin] = &card;
    card.update(String(pin) + " (" YASOLR_LBL_156 ")", DASH_STATUS_SUCCESS);
  }
}

namespace YaSolR {
  WebsiteClass Website;
} // namespace YaSolR
