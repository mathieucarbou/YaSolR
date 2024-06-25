// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolRWebsite.h>

#define HIDDEN_PWD "********"

#ifdef APP_MODEL_PRO
static const ChartSize chartSize = {.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12};
#endif

void YaSolR::WebsiteClass::initLayout() {
  logger.debug(TAG, "Initializing layout...");

  for (int i = 0; i < YASOLR_GRAPH_POINTS; i++)
    _historyX[i] = i - YASOLR_GRAPH_POINTS;

  _gridPowerHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.updateX(_historyX, YASOLR_GRAPH_POINTS);

#ifdef APP_MODEL_PRO
  dashboard.setChartAnimations(false);

  // graphs
  _gridPowerHistory.setSize(chartSize);
  _routedPowerHistory.setSize(chartSize);
  _routerTHDiHistory.setSize(chartSize);

  // output 1 (status)
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

  // output 2 (status)
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

  // output 1 (control)
  _output1BypassAuto.setTab(&_output1Tab);
  _output1DimmerAuto.setTab(&_output1Tab);
  _output1AutoStartTemp.setTab(&_output1Tab);
  _output1AutoStartTime.setTab(&_output1Tab);
  _output1AutoStartWDays.setTab(&_output1Tab);
  _output1AutoStoptTemp.setTab(&_output1Tab);
  _output1AutoStoptTime.setTab(&_output1Tab);
  _output1DimmerRatio.setTab(&_output1Tab);
  _output1DimmerDutyLimiter.setTab(&_output1Tab);
  _output1DimmerTempLimiter.setTab(&_output1Tab);

  _boolConfig(_output1BypassAuto, KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  _boolConfig(_output1DimmerAuto, KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  _daysConfig(_output1AutoStartWDays, KEY_OUTPUT1_DAYS);
  _numConfig(_output1AutoStartTemp, KEY_OUTPUT1_TEMPERATURE_START);
  _numConfig(_output1AutoStoptTemp, KEY_OUTPUT1_TEMPERATURE_STOP);
  _numConfig(_output1DimmerTempLimiter, KEY_OUTPUT1_DIMMER_MAX_TEMP);
  _sliderConfig(_output1DimmerDutyLimiter, KEY_OUTPUT1_DIMMER_MAX_DUTY);
  _textConfig(_output1AutoStartTime, KEY_OUTPUT1_TIME_START);
  _textConfig(_output1AutoStoptTime, KEY_OUTPUT1_TIME_STOP);
  _outputBypassSwitch(_output1Bypass, output1);
  _outputDimmerSlider(_output1DimmerSlider, output1);
  _output1DimmerRatio.attachCallback([this](int value) {
    config.set(KEY_OUTPUT_SPLIT, String(value));
    _output1DimmerRatio.update(static_cast<int>(config.get(KEY_OUTPUT_SPLIT).toInt()));
    _output2DimmerRatio.update(100 - static_cast<int>(config.get(KEY_OUTPUT_SPLIT).toInt()));
    dashboard.refreshCard(&_output1DimmerRatio);
    dashboard.refreshCard(&_output2DimmerRatio);
  });

  // output 2 (control)
  _output2BypassAuto.setTab(&_output2Tab);
  _output2DimmerAuto.setTab(&_output2Tab);
  _output2AutoStartTemp.setTab(&_output2Tab);
  _output2AutoStartTime.setTab(&_output2Tab);
  _output2AutoStartWDays.setTab(&_output2Tab);
  _output2AutoStoptTemp.setTab(&_output2Tab);
  _output2AutoStoptTime.setTab(&_output2Tab);
  _output2DimmerRatio.setTab(&_output2Tab);
  _output2DimmerDutyLimiter.setTab(&_output2Tab);
  _output2DimmerTempLimiter.setTab(&_output2Tab);

  _boolConfig(_output2BypassAuto, KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  _boolConfig(_output2DimmerAuto, KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  _daysConfig(_output2AutoStartWDays, KEY_OUTPUT2_DAYS);
  _numConfig(_output2AutoStartTemp, KEY_OUTPUT2_TEMPERATURE_START);
  _numConfig(_output2AutoStoptTemp, KEY_OUTPUT2_TEMPERATURE_STOP);
  _numConfig(_output2DimmerTempLimiter, KEY_OUTPUT2_DIMMER_MAX_TEMP);
  _sliderConfig(_output2DimmerDutyLimiter, KEY_OUTPUT2_DIMMER_MAX_DUTY);
  _textConfig(_output2AutoStartTime, KEY_OUTPUT2_TIME_START);
  _textConfig(_output2AutoStoptTime, KEY_OUTPUT2_TIME_STOP);
  _outputBypassSwitch(_output2Bypass, output2);
  _outputDimmerSlider(_output2DimmerSlider, output2);
  _output2DimmerRatio.attachCallback([this](int value) {
    config.set(KEY_OUTPUT_SPLIT, String(100 - value));
    _output1DimmerRatio.update(100 - static_cast<int>(config.get(KEY_OUTPUT_SPLIT).toInt()));
    _output2DimmerRatio.update(static_cast<int>(config.get(KEY_OUTPUT_SPLIT).toInt()));
    dashboard.refreshCard(&_output1DimmerRatio);
    dashboard.refreshCard(&_output2DimmerRatio);
  });

  // relays (control)
  _relay1Load.setTab(&_relaysTab);
  _relay1Switch.setTab(&_relaysTab);
  _relay1SwitchRO.setTab(&_relaysTab);
  _relay2Load.setTab(&_relaysTab);
  _relay2Switch.setTab(&_relaysTab);
  _relay2SwitchRO.setTab(&_relaysTab);

  _relaySwitch(_relay1Switch, routerRelay1);
  _relaySwitch(_relay2Switch, routerRelay2);

  // management
  _configBackup.setTab(&_managementTab);
  _configRestore.setTab(&_managementTab);
  _consoleLink.setTab(&_managementTab);
  _debugMode.setTab(&_managementTab);
  _otaLink.setTab(&_managementTab);
  _reset.setTab(&_managementTab);
  _restart.setTab(&_managementTab);
  _energyReset.setTab(&_managementTab);

  _boolConfig(_debugMode, KEY_ENABLE_DEBUG);

  _energyReset.attachCallback([] PUSH_BUTTON_CARD_CB {
    jsy.resetEnergy();
    pzemO1.resetEnergy();
    pzemO2.resetEnergy();
  });
  _reset.attachCallback([] PUSH_BUTTON_CARD_CB { resetTask.resume(); });
  _restart.attachCallback([] PUSH_BUTTON_CARD_CB { restartTask.resume(); });

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
  _output1ResistanceInput.setTab(&_hardwareConfigTab);
  _output2ResistanceInput.setTab(&_hardwareConfigTab);
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

  _numConfig(_gridFreq, KEY_GRID_FREQUENCY);
  _numConfig(_displayRotation, KEY_DISPLAY_ROTATION);
  _numConfig(_relay1Load, KEY_RELAY1_LOAD);
  _numConfig(_relay2Load, KEY_RELAY2_LOAD);
  _textConfig(_displayType, KEY_DISPLAY_TYPE);
  _numConfig(_output1ResistanceInput, KEY_OUTPUT1_RESISTANCE);
  _numConfig(_output2ResistanceInput, KEY_OUTPUT2_RESISTANCE);
  _textConfig(_output1RelayType, KEY_OUTPUT1_RELAY_TYPE);
  _textConfig(_output2RelayType, KEY_OUTPUT2_RELAY_TYPE);
  _textConfig(_relay1Type, KEY_RELAY1_TYPE);
  _textConfig(_relay2Type, KEY_RELAY2_TYPE);
  _sliderConfig(_displaySpeed, KEY_DISPLAY_SPEED);

  _output1PZEMSync.attachCallback([] PUSH_BUTTON_CARD_CB { pzemO1PairingTask.resume(); });
  _output2PZEMSync.attachCallback([] PUSH_BUTTON_CARD_CB { pzemO2PairingTask.resume(); });

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
  _mqttTopic.setTab(&_mqttConfigTab);
  _mqttUser.setTab(&_mqttConfigTab);

  _boolConfig(_haDiscovery, KEY_ENABLE_HA_DISCOVERY);
  _boolConfig(_mqttSecured, KEY_MQTT_SECURED);
  _numConfig(_mqttPort, KEY_MQTT_PORT);
  _passwordConfig(_mqttPwd, KEY_MQTT_PASSWORD);
  _sliderConfig(_mqttPublishInterval, KEY_MQTT_PUBLISH_INTERVAL);
  _textConfig(_haDiscoveryTopic, KEY_HA_DISCOVERY_TOPIC);
  _textConfig(_mqttGridPower, KEY_GRID_POWER_MQTT_TOPIC);
  _textConfig(_mqttGridVoltage, KEY_GRID_VOLTAGE_MQTT_TOPIC);
  _textConfig(_mqttServer, KEY_MQTT_SERVER);
  _textConfig(_mqttTopic, KEY_MQTT_TOPIC);
  _textConfig(_mqttUser, KEY_MQTT_USERNAME);

  // network (config)
  _adminPwd.setTab(&_networkConfigTab);
  _apMode.setTab(&_networkConfigTab);
  _ntpServer.setTab(&_networkConfigTab);
  _ntpSync.setTab(&_networkConfigTab);
  _ntpTimezone.setTab(&_networkConfigTab);
  _wifiPwd.setTab(&_networkConfigTab);
  _wifiSSID.setTab(&_networkConfigTab);

  _boolConfig(_apMode, KEY_ENABLE_AP_MODE);
  _passwordConfig(_adminPwd, KEY_ADMIN_PASSWORD);
  _passwordConfig(_wifiPwd, KEY_WIFI_PASSWORD);
  _textConfig(_ntpServer, KEY_NTP_SERVER);
  _textConfig(_ntpTimezone, KEY_NTP_TIMEZONE);
  _textConfig(_wifiSSID, KEY_WIFI_SSID);

  _ntpSync.attachCallback([](const char* value) {
    const String str = String(value);
    const size_t len = str.length();
    const timeval tv = {str.substring(0, len - 3).toInt(), str.substring(len - 3).toInt()};
    Mycila::NTP.sync(tv);
  });
#endif
}

void YaSolR::WebsiteClass::initCards() {
  logger.debug(TAG, "Initializing cards...");

  // Statistics
  _appManufacturer.set(Mycila::AppInfo.manufacturer.c_str());
  _appModel.set((Mycila::AppInfo.model.c_str()));
  _appName.set((Mycila::AppInfo.name.c_str()));
  _appVersion.set(Mycila::AppInfo.version.c_str());
  _deviceBootCount.set(String(Mycila::System.getBootCount()).c_str());
  _deviceCores.set(String(ESP.getChipCores()).c_str());
  _deviceModel.set(ESP.getChipModel());
  _deviceRev.set(String(ESP.getChipRevision()).c_str());
  _deviceHeapTotal.set((String(ESP.getHeapSize()) + " bytes").c_str());
  _deviceID.set(Mycila::AppInfo.id.c_str());
  _firmwareBuildHash.set(Mycila::AppInfo.buildHash.c_str());
  _firmwareBuildTimestamp.set(Mycila::AppInfo.buildDate.c_str());
  _firmwareFilename.set(Mycila::AppInfo.firmware.c_str());
  _networkAPMAC.set(ESPConnect.getMACAddress(ESPConnectMode::AP).c_str());
  _networkEthMAC.set(ESPConnect.getMACAddress(ESPConnectMode::ETH).isEmpty() ? "N/A" : ESPConnect.getMACAddress(ESPConnectMode::ETH).c_str());
  _networkHostname.set(Mycila::AppInfo.defaultHostname.c_str());
  _networkWiFiMAC.set(ESPConnect.getMACAddress(ESPConnectMode::STA).c_str());

#ifdef APP_MODEL_PRO
  // output 1 (control)
  bool dimmer1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_DIMMER);
  bool output1RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT1_RELAY);
  bool bypass1Possible = dimmer1Enabled || output1RelayEnabled;
  bool autoDimmerO1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  bool autoBypassActivated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  bool output1DS18Enabled = config.getBool(KEY_ENABLE_OUTPUT1_DS18);

  _output1DimmerAuto.update(autoDimmerO1Activated);
  _output1DimmerRatio.update(static_cast<int>(config.get(KEY_OUTPUT_SPLIT).toInt()));
  _output1DimmerDutyLimiter.update(static_cast<int>(config.get(KEY_OUTPUT1_DIMMER_MAX_DUTY).toInt()));
  _output1DimmerTempLimiter.update(config.get(KEY_OUTPUT1_DIMMER_MAX_TEMP));
  _output1BypassAuto.update(autoBypassActivated);
  _output1AutoStartWDays.update(config.get(KEY_OUTPUT1_DAYS));
  _output1AutoStartTemp.update(config.get(KEY_OUTPUT1_TEMPERATURE_START));
  _output1AutoStartTime.update(config.get(KEY_OUTPUT1_TIME_START));
  _output1AutoStoptTemp.update(config.get(KEY_OUTPUT1_TEMPERATURE_STOP));
  _output1AutoStoptTime.update(config.get(KEY_OUTPUT1_TIME_STOP));

  _output1Tab.setDisplay(dimmer1Enabled || output1DS18Enabled || output1RelayEnabled);
  _output1DimmerSlider.setDisplay(dimmer1Enabled && !autoDimmerO1Activated);
  _output1DimmerSliderRO.setDisplay(dimmer1Enabled && autoDimmerO1Activated);
  _output1Bypass.setDisplay(bypass1Possible && !autoBypassActivated);
  _output1BypassRO.setDisplay(bypass1Possible && autoBypassActivated);
  _output1Power.setDisplay(dimmer1Enabled);
  _output1ApparentPower.setDisplay(dimmer1Enabled);
  _output1PowerFactor.setDisplay(dimmer1Enabled);
  _output1THDi.setDisplay(dimmer1Enabled);
  _output1Voltage.setDisplay(dimmer1Enabled);
  _output1Current.setDisplay(dimmer1Enabled);
  _output1Resistance.setDisplay(dimmer1Enabled);
  _output1Energy.setDisplay(dimmer1Enabled);
  _output1DimmerAuto.setDisplay(dimmer1Enabled);
  _output1DimmerRatio.setDisplay(dimmer1Enabled && autoDimmerO1Activated);
  _output1DimmerDutyLimiter.setDisplay(dimmer1Enabled);
  _output1DimmerTempLimiter.setDisplay(dimmer1Enabled && output1DS18Enabled);
  _output1BypassAuto.setDisplay(bypass1Possible);
  _output1AutoStartWDays.setDisplay(bypass1Possible && autoBypassActivated);
  _output1AutoStartTime.setDisplay(bypass1Possible && autoBypassActivated);
  _output1AutoStoptTime.setDisplay(bypass1Possible && autoBypassActivated);
  _output1AutoStartTemp.setDisplay(bypass1Possible && autoBypassActivated && output1DS18Enabled);
  _output1AutoStoptTemp.setDisplay(bypass1Possible && autoBypassActivated && output1DS18Enabled);

  // output 2 (control)
  bool dimmer2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_DIMMER);
  bool output2RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT2_RELAY);
  bool bypass2Possible = dimmer2Enabled || output2RelayEnabled;
  bool autoDimmerO2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  bool autoBypassO2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  bool output2DS18Enabled = config.getBool(KEY_ENABLE_OUTPUT2_DS18);

  _output2DimmerAuto.update(autoDimmerO2Activated);
  _output2DimmerRatio.update(100 - static_cast<int>(config.get(KEY_OUTPUT_SPLIT).toInt()));
  _output2DimmerDutyLimiter.update(static_cast<int>(config.get(KEY_OUTPUT2_DIMMER_MAX_DUTY).toInt()));
  _output2DimmerTempLimiter.update(config.get(KEY_OUTPUT2_DIMMER_MAX_TEMP));
  _output2BypassAuto.update(autoBypassO2Activated);
  _output2AutoStartWDays.update(config.get(KEY_OUTPUT2_DAYS));
  _output2AutoStartTemp.update(config.get(KEY_OUTPUT2_TEMPERATURE_START));
  _output2AutoStartTime.update(config.get(KEY_OUTPUT2_TIME_START));
  _output2AutoStoptTemp.update(config.get(KEY_OUTPUT2_TEMPERATURE_STOP));
  _output2AutoStoptTime.update(config.get(KEY_OUTPUT2_TIME_STOP));

  _output2Tab.setDisplay(dimmer2Enabled || output2DS18Enabled || output2RelayEnabled);
  _output2DimmerSlider.setDisplay(dimmer2Enabled && !autoDimmerO2Activated);
  _output2DimmerSliderRO.setDisplay(dimmer2Enabled && autoDimmerO2Activated);
  _output2Bypass.setDisplay(bypass2Possible && !autoBypassO2Activated);
  _output2BypassRO.setDisplay(bypass2Possible && autoBypassO2Activated);
  _output2Power.setDisplay(dimmer2Enabled);
  _output2ApparentPower.setDisplay(dimmer2Enabled);
  _output2PowerFactor.setDisplay(dimmer2Enabled);
  _output2THDi.setDisplay(dimmer2Enabled);
  _output2Voltage.setDisplay(dimmer2Enabled);
  _output2Current.setDisplay(dimmer2Enabled);
  _output2Resistance.setDisplay(dimmer2Enabled);
  _output2Energy.setDisplay(dimmer2Enabled);
  _output2DimmerAuto.setDisplay(dimmer2Enabled);
  _output2DimmerRatio.setDisplay(dimmer2Enabled && autoDimmerO2Activated);
  _output2DimmerDutyLimiter.setDisplay(dimmer2Enabled);
  _output2DimmerTempLimiter.setDisplay(dimmer2Enabled && output2DS18Enabled);
  _output2BypassAuto.setDisplay(bypass2Possible);
  _output2AutoStartWDays.setDisplay(bypass2Possible && autoBypassO2Activated);
  _output2AutoStartTime.setDisplay(bypass2Possible && autoBypassO2Activated);
  _output2AutoStoptTime.setDisplay(bypass2Possible && autoBypassO2Activated);
  _output2AutoStartTemp.setDisplay(bypass2Possible && autoBypassO2Activated && output2DS18Enabled);
  _output2AutoStoptTemp.setDisplay(bypass2Possible && autoBypassO2Activated && output2DS18Enabled);

  // relays (control)
  int32_t load1 = config.get(KEY_RELAY1_LOAD).toInt();
  int32_t load2 = config.get(KEY_RELAY2_LOAD).toInt();
  _relay1Load.update(static_cast<int>(load1));
  _relay2Load.update(static_cast<int>(load2));
  _relaysTab.setDisplay(config.getBool(KEY_ENABLE_RELAY1) || config.getBool(KEY_ENABLE_RELAY2));
  _relay1Load.setDisplay(config.getBool(KEY_ENABLE_RELAY1));
  _relay1Switch.setDisplay(config.getBool(KEY_ENABLE_RELAY1) && load1 <= 0);
  _relay1SwitchRO.setDisplay(config.getBool(KEY_ENABLE_RELAY1) && load1 > 0);
  _relay2Load.setDisplay(config.getBool(KEY_ENABLE_RELAY2));
  _relay2Switch.setDisplay(config.getBool(KEY_ENABLE_RELAY2) && load2 <= 0);
  _relay2SwitchRO.setDisplay(config.getBool(KEY_ENABLE_RELAY2) && load2 > 0);

  // management
  _configBackup.update("/api/config/backup");
  _configRestore.update("/api/config/restore");
  _consoleLink.update("/console");
  _debugMode.update(config.getBool(KEY_ENABLE_DEBUG));
  _otaLink.update("/update");
  _energyReset.setDisplay(config.getBool(KEY_ENABLE_JSY) || config.getBool(KEY_ENABLE_OUTPUT1_PZEM) || config.getBool(KEY_ENABLE_OUTPUT2_PZEM));

  // GPIO

  std::map<int32_t, Card*> pinout = {};
  _pinout(_pinDimmerO1, config.get(KEY_PIN_OUTPUT1_DIMMER).toInt(), pinout);
  _pinout(_pinDimmerO2, config.get(KEY_PIN_OUTPUT2_DIMMER).toInt(), pinout);
  _pinout(_pinDisplayClock, config.get(KEY_PIN_DISPLAY_SCL).toInt(), pinout);
  _pinout(_pinDisplayData, config.get(KEY_PIN_DISPLAY_SDA).toInt(), pinout);
  _pinout(_pinDS18O1, config.get(KEY_PIN_OUTPUT1_DS18).toInt(), pinout);
  _pinout(_pinDS18O2, config.get(KEY_PIN_OUTPUT2_DS18).toInt(), pinout);
  _pinout(_pinDS18Router, config.get(KEY_PIN_ROUTER_DS18).toInt(), pinout);
  _pinout(_pinJsyRX, config.get(KEY_PIN_JSY_RX).toInt(), pinout);
  _pinout(_pinJsyTX, config.get(KEY_PIN_JSY_TX).toInt(), pinout);
  _pinout(_pinLEDGreen, config.get(KEY_PIN_LIGHTS_GREEN).toInt(), pinout);
  _pinout(_pinLEDRed, config.get(KEY_PIN_LIGHTS_RED).toInt(), pinout);
  _pinout(_pinLEDYellow, config.get(KEY_PIN_LIGHTS_YELLOW).toInt(), pinout);
  _pinout(_pinPZEMRX, config.get(KEY_PIN_PZEM_RX).toInt(), pinout);
  _pinout(_pinPZEMTX, config.get(KEY_PIN_PZEM_TX).toInt(), pinout);
  _pinout(_pinRelay1, config.get(KEY_PIN_RELAY1).toInt(), pinout);
  _pinout(_pinRelay2, config.get(KEY_PIN_RELAY2).toInt(), pinout);
  _pinout(_pinRelayO1, config.get(KEY_PIN_OUTPUT1_RELAY).toInt(), pinout);
  _pinout(_pinRelayO2, config.get(KEY_PIN_OUTPUT2_RELAY).toInt(), pinout);
  _pinout(_pinZCD, config.get(KEY_PIN_ZCD).toInt(), pinout);
  pinout.clear();

  // Hardware
  _status(_display, KEY_ENABLE_DISPLAY, display.isEnabled());
  _status(_led, KEY_ENABLE_LIGHTS, lights.isEnabled());
  _status(_output1Relay, KEY_ENABLE_OUTPUT1_RELAY, bypassRelayO1.isEnabled());
  _status(_output2Relay, KEY_ENABLE_OUTPUT2_RELAY, bypassRelayO2.isEnabled());
  _status(_relay1, KEY_ENABLE_RELAY1, relay1.isEnabled());
  _status(_relay2, KEY_ENABLE_RELAY2, relay2.isEnabled());

  // Hardware (config)
  _gridFreq.update(config.get(KEY_GRID_FREQUENCY).toInt() == 60 ? "60 Hz" : "50 Hz", "50 Hz,60 Hz");
  _output1RelayType.update(config.get(KEY_OUTPUT1_RELAY_TYPE), "NO,NC");
  _output2RelayType.update(config.get(KEY_OUTPUT2_RELAY_TYPE), "NO,NC");
  _relay1Type.update(config.get(KEY_RELAY1_TYPE), "NO,NC");
  _relay2Type.update(config.get(KEY_RELAY2_TYPE), "NO,NC");
  _displayType.update(config.get(KEY_DISPLAY_TYPE), "SH1106,SH1107,SSD1306");
  _displaySpeed.update(static_cast<int>(config.get(KEY_DISPLAY_SPEED).toInt()));
  _displayRotation.update(config.get(KEY_DISPLAY_ROTATION) + "°", "0°,90°,180°,270°");
  _output1ResistanceInput.update(config.get(KEY_OUTPUT1_RESISTANCE));
  _output2ResistanceInput.update(config.get(KEY_OUTPUT2_RESISTANCE));

  _displayType.setDisplay(config.getBool(KEY_ENABLE_DISPLAY));
  _displaySpeed.setDisplay(config.getBool(KEY_ENABLE_DISPLAY));
  _displayRotation.setDisplay(config.getBool(KEY_ENABLE_DISPLAY));
  _output1ResistanceInput.setDisplay(dimmer1Enabled);
  _output2ResistanceInput.setDisplay(dimmer2Enabled);
  _output1PZEMSync.setDisplay(config.getBool(KEY_ENABLE_OUTPUT1_PZEM));
  _output2PZEMSync.setDisplay(config.getBool(KEY_ENABLE_OUTPUT2_PZEM));
  _output1RelayType.setDisplay(config.getBool(KEY_ENABLE_OUTPUT1_RELAY));
  _output2RelayType.setDisplay(config.getBool(KEY_ENABLE_OUTPUT2_RELAY));
  _relay1Type.setDisplay(config.getBool(KEY_ENABLE_RELAY1));
  _relay2Type.setDisplay(config.getBool(KEY_ENABLE_RELAY2));

  // mqtt (config)
  _haDiscovery.update(config.getBool(KEY_ENABLE_HA_DISCOVERY));
  _haDiscoveryTopic.update(config.get(KEY_HA_DISCOVERY_TOPIC));
  _mqttGridPower.update(config.get(KEY_GRID_POWER_MQTT_TOPIC));
  _mqttGridVoltage.update(config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC));
  _mqttPort.update(config.get(KEY_MQTT_PORT));
  _mqttPublishInterval.update(config.get(KEY_MQTT_PUBLISH_INTERVAL));
  _mqttPwd.update(config.get(KEY_MQTT_PASSWORD).isEmpty() ? "" : HIDDEN_PWD);
  _mqttSecured.update(config.getBool(KEY_MQTT_SECURED));
  _mqttServer.update(config.get(KEY_MQTT_SERVER));
  _mqttServerCert.update("/api/config/mqttServerCertificate");
  _mqttTopic.update(config.get(KEY_MQTT_TOPIC));
  _mqttUser.update(config.get(KEY_MQTT_USERNAME));
  _mqttConfigTab.setDisplay(config.getBool(KEY_ENABLE_MQTT));

  // network (config)
  _adminPwd.update(config.get(KEY_ADMIN_PASSWORD).isEmpty() ? "" : HIDDEN_PWD);
  _apMode.update(config.getBool(KEY_ENABLE_AP_MODE));
  _ntpServer.update(config.get(KEY_NTP_SERVER));
  _ntpTimezone.update(config.get(KEY_NTP_TIMEZONE), "/timezones");
  _wifiPwd.update(config.get(KEY_WIFI_PASSWORD).isEmpty() ? "" : HIDDEN_PWD);
  _wifiSSID.update(config.get(KEY_WIFI_SSID));
#endif
}

void YaSolR::WebsiteClass::updateCards() {
  Mycila::GridMetrics gridMetrics;
  grid.getMetrics(gridMetrics);

  Mycila::RouterMetrics routerMetrics;
  router.getMetrics(routerMetrics);

  // stats
  Mycila::SystemMemory memory = Mycila::System.getMemory();
  ESPConnectMode mode = ESPConnect.getMode();
  _output1RelaySwitchCount.set(String(bypassRelayO1.getSwitchCount()).c_str());
  _output2RelaySwitchCount.set(String(bypassRelayO2.getSwitchCount()).c_str());
  _deviceHeapUsage.set((String(memory.usage) + " %").c_str());
  _deviceHeapUsed.set((String(memory.used) + " bytes").c_str());
  _gridEnergy.set((String(gridMetrics.energy, 3) + " kWh").c_str());
  _gridEnergyReturned.set((String(gridMetrics.energyReturned, 3) + " kWh").c_str());
  _gridFrequency.set((String(gridMetrics.frequency) + " Hz").c_str());
  _networkAPIP.set(ESPConnect.getIPAddress(ESPConnectMode::AP).toString().c_str());
  _networkEthIP.set(ESPConnect.getIPAddress(ESPConnectMode::ETH).toString().c_str());
  _networkInterface.set(mode == ESPConnectMode::AP ? "AP" : (mode == ESPConnectMode::STA ? "WiFi" : (mode == ESPConnectMode::ETH ? "Ethernet" : "")));
  _networkWiFiIP.set(ESPConnect.getIPAddress(ESPConnectMode::STA).toString().c_str());
  _networkWiFiRSSI.set((String(ESPConnect.getWiFiRSSI()) + " dBm").c_str());
  _networkWiFiSignal.set((String(ESPConnect.getWiFiSignalQuality()) + " %").c_str());
  _networkWiFiSSID.set(ESPConnect.getWiFiSSID().c_str());
  _relay1SwitchCount.set(String(relay1.getSwitchCount()).c_str());
  _relay2SwitchCount.set(String(relay2.getSwitchCount()).c_str());
  _jsyRemoteUdpRate.set((String(jsyRemoteUdpRate.rate()) + " msg/s").c_str());
  _time.set(Mycila::Time::getLocalStr().c_str());
  _uptime.set(Mycila::Time::toDHHMMSS(Mycila::System.getUptime()).c_str());
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

#ifdef APP_MODEL_PRO
  // Output 1 (status)
  switch (output1.getState()) {
    case Mycila::RouterOutputState::OUTPUT_DISABLED:
    case Mycila::RouterOutputState::OUTPUT_IDLE:
      _output1State.update(output1.getStateName(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutputState::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutputState::OUTPUT_BYPASS_MANUAL:
      _output1State.update(output1.getStateName(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutputState::OUTPUT_ROUTING:
      _output1State.update(output1.getStateName(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output1State.update(YASOLR_LBL_109, DASH_STATUS_DANGER);
      break;
  }
  _temperature(_output1DS18State, ds18O1);
  _output1DimmerSlider.update(dimmerO1.getPowerDuty());
  _output1DimmerSliderRO.update(dimmerO1.getPowerDuty());
  _output1Bypass.update(output1.isBypassOn());
  _output1BypassRO.update(YASOLR_STATE(output1.isBypassOn()), output1.isBypassOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);
  _output1Power.update(routerMetrics.outputs[0].power);
  _output1ApparentPower.update(routerMetrics.outputs[0].apparentPower);
  _output1PowerFactor.update(routerMetrics.outputs[0].powerFactor);
  _output1THDi.update(routerMetrics.outputs[0].thdi * 100);
  _output1Voltage.update(routerMetrics.outputs[0].dimmedVoltage);
  _output1Current.update(routerMetrics.outputs[0].current);
  _output1Resistance.update(routerMetrics.outputs[0].resistance);
  _output1Energy.update(routerMetrics.outputs[0].energy);

  // output 2
  switch (output2.getState()) {
    case Mycila::RouterOutputState::OUTPUT_DISABLED:
    case Mycila::RouterOutputState::OUTPUT_IDLE:
      _output2State.update(output2.getStateName(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutputState::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutputState::OUTPUT_BYPASS_MANUAL:
      _output2State.update(output2.getStateName(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutputState::OUTPUT_ROUTING:
      _output2State.update(output2.getStateName(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output2State.update(YASOLR_LBL_109, DASH_STATUS_DANGER);
      break;
  }
  _temperature(_output2DS18State, ds18O2);
  _output2DimmerSlider.update(dimmerO2.getPowerDuty());
  _output2DimmerSliderRO.update(dimmerO2.getPowerDuty());
  _output2Bypass.update(output2.isBypassOn());
  _output2BypassRO.update(YASOLR_STATE(output2.isBypassOn()), output2.isBypassOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);
  _output2Power.update(routerMetrics.outputs[1].power);
  _output2ApparentPower.update(routerMetrics.outputs[1].apparentPower);
  _output2PowerFactor.update(routerMetrics.outputs[1].powerFactor);
  _output2THDi.update(routerMetrics.outputs[1].thdi * 100);
  _output2Voltage.update(routerMetrics.outputs[1].dimmedVoltage);
  _output2Current.update(routerMetrics.outputs[1].current);
  _output2Resistance.update(routerMetrics.outputs[1].resistance);
  _output2Energy.update(routerMetrics.outputs[1].energy);

  // relays
  _relay1Switch.update(relay1.isOn());
  _relay1SwitchRO.update(YASOLR_STATE(relay1.isOn()), relay1.isOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);
  _relay2Switch.update(relay2.isOn());
  _relay2SwitchRO.update(YASOLR_STATE(relay2.isOn()), relay2.isOn() ? DASH_STATUS_SUCCESS : DASH_STATUS_IDLE);

  // Hardware (status)
  _status(_jsy, KEY_ENABLE_JSY, jsy.isEnabled(), jsy.isConnected(), YASOLR_LBL_110);
  _status(_mqtt, KEY_ENABLE_MQTT, mqtt.isEnabled(), mqtt.isConnected(), mqtt.getLastError() ? mqtt.getLastError() : YASOLR_LBL_113);
  _status(_output1Dimmer, KEY_ENABLE_OUTPUT1_DIMMER, dimmerO1.isEnabled(), dimmerO1.isConnected(), YASOLR_LBL_110);
  _status(_output1DS18, KEY_ENABLE_OUTPUT1_DS18, ds18O1.isEnabled(), ds18O1.getLastTime() > 0, YASOLR_LBL_114);
  _status(_output1PZEM, KEY_ENABLE_OUTPUT1_PZEM, pzemO1.isEnabled(), pzemO1.isConnected(), YASOLR_LBL_110);
  _status(_output2Dimmer, KEY_ENABLE_OUTPUT2_DIMMER, dimmerO2.isEnabled(), dimmerO2.isConnected(), YASOLR_LBL_110);
  _status(_output2DS18, KEY_ENABLE_OUTPUT2_DS18, ds18O2.isEnabled(), ds18O2.getLastTime() > 0, YASOLR_LBL_114);
  _status(_output2PZEM, KEY_ENABLE_OUTPUT2_PZEM, pzemO2.isEnabled(), pzemO2.isConnected(), YASOLR_LBL_110);
  _status(_routerDS18, KEY_ENABLE_DS18_SYSTEM, ds18Sys.isEnabled(), ds18Sys.getLastTime() > 0, YASOLR_LBL_114);
  _status(_zcd, KEY_ENABLE_ZCD, zcd.isEnabled(), zcd.isConnected(), YASOLR_LBL_110);
#endif
}

void YaSolR::WebsiteClass::updateCharts() {
  // read last metrics
  Mycila::GridMetrics gridMetrics;
  grid.getMetrics(gridMetrics);
  Mycila::RouterMetrics routerMetrics;
  router.getMetrics(routerMetrics);

  // shift array
  for (size_t i = 0; i < YASOLR_GRAPH_POINTS - 1; i++) {
    _gridPowerHistoryY[i] = _gridPowerHistoryY[i + 1];
    _routedPowerHistoryY[i] = _routedPowerHistoryY[i + 1];
    _routerTHDiHistoryY[i] = _routerTHDiHistoryY[i + 1];
  }

  // set new value
  _gridPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = round(gridMetrics.power);
  _routedPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = round(routerMetrics.power);
  _routerTHDiHistoryY[YASOLR_GRAPH_POINTS - 1] = round(routerMetrics.thdi * 100);

  // update charts
  _gridPowerHistory.updateY(_gridPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.updateY(_routedPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.updateY(_routerTHDiHistoryY, YASOLR_GRAPH_POINTS);
}

void YaSolR::WebsiteClass::_sliderConfig(Card& card, const char* key) {
  card.attachCallback([key, &card](int value) {
    config.set(key, String(value));
    card.update(static_cast<int>(config.get(key).toInt()));
    dashboard.refreshCard(&card);
  });
}

void YaSolR::WebsiteClass::_numConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    if (strlen(value) == 0) {
      config.unset(key);
    } else {
      config.set(key, String(strtol(value, nullptr, 10)));
    }
    card.update(static_cast<int>(config.get(key).toInt()));
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
      config.set(key, String(strtol(value, nullptr, 10)));
    }
    initCards();
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::WebsiteClass::_boolConfig(Card& card, const char* key) {
  card.attachCallback([key, &card, this](int value) {
    config.setBool(key, value);
    card.update(static_cast<int>(config.getBool(key)));
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
    } else if (strlen(value) >= 8) {
      config.set(key, value);
    }
    card.update(config.get(key).isEmpty() ? "" : HIDDEN_PWD);
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
      output.tryBypassState(value);
    }
    card.update(output.isBypassOn());
    dashboard.refreshCard(&card);
    dashboardTask.requestEarlyRun();
  });
}

void YaSolR::WebsiteClass::_outputDimmerSlider(Card& card, Mycila::RouterOutput& output) {
  card.attachCallback([&card, &output, this](int value) {
    if (output.isDimmerEnabled()) {
      output.tryDimmerDuty(value);
    }
    card.update(output.getDimmerDuty());
    dashboard.refreshCard(&card);
    dashboardTask.requestEarlyRun();
  });
}

void YaSolR::WebsiteClass::_temperature(Card& card, Mycila::DS18& sensor) {
  if (!sensor.isEnabled()) {
    card.update(YASOLR_LBL_115, "");
  } else if (sensor.getLastTime() == 0) {
    card.update(YASOLR_LBL_123, "");
  } else {
    card.update(sensor.getValidTemperature(), "°C");
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

void YaSolR::WebsiteClass::_pinout(Card& card, int32_t pin, std::map<int32_t, Card*>& pinout) {
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
