// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolRWebsite.h>

#ifdef APP_VERSION_PRO
#include <ElegantOTAPro.h>
#else
#include <ElegantOTA.h>
#endif

#define TAG "WEBSITE"

extern const uint8_t logo_png_gz_start[] asm("_binary__pio_data_logo_png_gz_start");
extern const uint8_t logo_png_gz_end[] asm("_binary__pio_data_logo_png_gz_end");
extern const uint8_t logo_icon_png_gz_start[] asm("_binary__pio_data_logo_icon_png_gz_start");
extern const uint8_t logo_icon_png_gz_end[] asm("_binary__pio_data_logo_icon_png_gz_end");
extern const uint8_t config_html_gz_start[] asm("_binary__pio_data_config_html_gz_start");
extern const uint8_t config_html_gz_end[] asm("_binary__pio_data_config_html_gz_end");

void YaSolR::WebsiteClass::init() {
// pro mode
#ifdef APP_VERSION_PRO
  dashboard.setTitle((Mycila::AppInfo.name + " " + Mycila::AppInfo.model).c_str());

  // dropdowns

  _displayRotation.update("", "0°,90°,180°,270°");
  _displayType.update("", "SH1106,SH1107,SSD1306");
  _gridFreq.update("", "50 Hz,60 Hz");
  _output1BypassType.update("", "NO,NC");
  _output2BypassType.update("", "NO,NC");
  _pushButtonAction.update("", "restart,bypass");
  _relay1Type.update("", "NO,NC");
  _relay2Type.update("", "NO,NC");
  _output1DimmerType.update("", "TRIAC,SSR_RANDOM,SSR_ZC");
  _output2DimmerType.update("", "TRIAC,SSR_RANDOM,SSR_ZC");
  _ntpTimezone.update("", "/api/ntp/timezones");

  // tabs

  _configBackup.setTab(&_managementTab);
  _configRestore.setTab(&_managementTab);
  _consoleLink.setTab(&_managementTab);
  _debugMode.setTab(&_managementTab);
  _otaLink.setTab(&_managementTab);
  _reset.setTab(&_managementTab);
  _restart.setTab(&_managementTab);

  _stateBuzzer.setTab(&_stateTab);
  _stateDisplay.setTab(&_stateTab);
  _stateGridDataMQTT.setTab(&_stateTab);
  _stateJSY.setTab(&_stateTab);
  _stateLEDs.setTab(&_stateTab);
  _stateMQTT.setTab(&_stateTab);
  _stateNTP.setTab(&_stateTab);
  _stateOutput1AutoBypass.setTab(&_stateTab);
  _stateOutput1AutoDimmer.setTab(&_stateTab);
  _stateOutput1Bypass.setTab(&_stateTab);
  _stateOutput1Dimmer.setTab(&_stateTab);
  _stateOutput1PZEM.setTab(&_stateTab);
  _stateOutput1Temp.setTab(&_stateTab);
  _stateOutput2AutoBypass.setTab(&_stateTab);
  _stateOutput2AutoDimmer.setTab(&_stateTab);
  _stateOutput2Bypass.setTab(&_stateTab);
  _stateOutput2Dimmer.setTab(&_stateTab);
  _stateOutput2PZEM.setTab(&_stateTab);
  _stateOutput2Temp.setTab(&_stateTab);
  _statePushButton.setTab(&_stateTab);
  _stateRelay1.setTab(&_stateTab);
  _stateRelay2.setTab(&_stateTab);
  _stateSystemTemp.setTab(&_stateTab);
  _stateZCD.setTab(&_stateTab);

  _pinLiveBuzzer.setTab(&_pinLiveTab);
  _pinLiveDisplayClock.setTab(&_pinLiveTab);
  _pinLiveDisplayData.setTab(&_pinLiveTab);
  _pinLiveJsyRX.setTab(&_pinLiveTab);
  _pinLiveJsyTX.setTab(&_pinLiveTab);
  _pinLiveLEDGreen.setTab(&_pinLiveTab);
  _pinLiveLEDRed.setTab(&_pinLiveTab);
  _pinLiveLEDYellow.setTab(&_pinLiveTab);
  _pinLiveOutput1Bypass.setTab(&_pinLiveTab);
  _pinLiveOutput1Dimmer.setTab(&_pinLiveTab);
  _pinLiveOutput1Temp.setTab(&_pinLiveTab);
  _pinLiveOutput2Bypass.setTab(&_pinLiveTab);
  _pinLiveOutput2Dimmer.setTab(&_pinLiveTab);
  _pinLiveOutput2Temp.setTab(&_pinLiveTab);
  _pinLivePushButton.setTab(&_pinLiveTab);
  _pinLivePZEMRX.setTab(&_pinLiveTab);
  _pinLivePZEMTX.setTab(&_pinLiveTab);
  _pinLiveRelay1.setTab(&_pinLiveTab);
  _pinLiveRelay2.setTab(&_pinLiveTab);
  _pinLiveSystemTemp.setTab(&_pinLiveTab);
  _pinLiveZCD.setTab(&_pinLiveTab);

  _pinConfigBuzzer.setTab(&_pinConfigTab);
  _pinConfigDisplayClock.setTab(&_pinConfigTab);
  _pinConfigDisplayData.setTab(&_pinConfigTab);
  _pinConfigJsyRX.setTab(&_pinConfigTab);
  _pinConfigJsyTX.setTab(&_pinConfigTab);
  _pinConfigLEDGreen.setTab(&_pinConfigTab);
  _pinConfigLEDRed.setTab(&_pinConfigTab);
  _pinConfigLEDYellow.setTab(&_pinConfigTab);
  _pinConfigOutput1Bypass.setTab(&_pinConfigTab);
  _pinConfigOutput1Dimmer.setTab(&_pinConfigTab);
  _pinConfigOutput1Temp.setTab(&_pinConfigTab);
  _pinConfigOutput2Bypass.setTab(&_pinConfigTab);
  _pinConfigOutput2Dimmer.setTab(&_pinConfigTab);
  _pinConfigOutput2Temp.setTab(&_pinConfigTab);
  _pinConfigPushButton.setTab(&_pinConfigTab);
  _pinConfigPZEMRX.setTab(&_pinConfigTab);
  _pinConfigPZEMTX.setTab(&_pinConfigTab);
  _pinConfigRelay1.setTab(&_pinConfigTab);
  _pinConfigRelay2.setTab(&_pinConfigTab);
  _pinConfigSystemTemp.setTab(&_pinConfigTab);
  _pinConfigZCD.setTab(&_pinConfigTab);

  _buzzer.setTab(&_systemTab);
  _buzzerPin.setTab(&_systemTab);

  _display.setTab(&_displayTab);
  _displayClockPin.setTab(&_displayTab);
  _displayDataPin.setTab(&_displayTab);
  _displayPowerSaveDelay.setTab(&_displayTab);
  _displayRotation.setTab(&_displayTab);
  _displayType.setTab(&_displayTab);

  _mqtt.setTab(&_mqttTab);
  _mqttPort.setTab(&_mqttTab);
  _mqttPublishInterval.setTab(&_mqttTab);
  _mqttPwd.setTab(&_mqttTab);
  _mqttSecured.setTab(&_mqttTab);
  _mqttServer.setTab(&_mqttTab);
  _mqttTopic.setTab(&_mqttTab);
  _mqttUser.setTab(&_mqttTab);
  _mqttServerCert.setTab(&_mqttTab);

  _haDiscovery.setTab(&_mqttTab);
  _haDiscoveryTopic.setTab(&_mqttTab);

  _adminPwd.setTab(&_networkTab);
  _apMode.setTab(&_networkTab);
  _hostname.setTab(&_networkTab);
  _wifiPwd.setTab(&_networkTab);
  _wifiSSID.setTab(&_networkTab);

  _ntpServer.setTab(&_networkTab);
  _ntpTimezone.setTab(&_networkTab);
  _ntpSync.setTab(&_networkTab);

  _output1AutoBypass.setTab(&_output1Tab);
  _output1AutoDimmer.setTab(&_output1Tab);
  _output1AutoStartTemp.setTab(&_output1Tab);
  _output1AutoStartTime.setTab(&_output1Tab);
  _output1AutoStartWDays.setTab(&_output1Tab);
  _output1AutoStoptTemp.setTab(&_output1Tab);
  _output1AutoStoptTime.setTab(&_output1Tab);
  _output1Bypass.setTab(&_output1Tab);
  _output1BypassPin.setTab(&_output1Tab);
  _output1BypassType.setTab(&_output1Tab);
  _output1Dimmer.setTab(&_output1Tab);
  _output1DimmerPin.setTab(&_output1Tab);
  _output1DimmerType.setTab(&_output1Tab);
  _output1DimmerPowerLimit.setTab(&_output1Tab);
  _output1Temp.setTab(&_output1Tab);
  _output1TempPin.setTab(&_output1Tab);
  _output1PZEM.setTab(&_output1Tab);
  _output1PZEMRXPin.setTab(&_output1Tab);
  _output1PZEMTXPin.setTab(&_output1Tab);
  _output1PZEMReset.setTab(&_output1Tab);
  _output1PZEMSync.setTab(&_output1Tab);

  _output2AutoBypass.setTab(&_output2Tab);
  _output2AutoDimmer.setTab(&_output2Tab);
  _output2AutoStartTemp.setTab(&_output2Tab);
  _output2AutoStartTime.setTab(&_output2Tab);
  _output2AutoStartWDays.setTab(&_output2Tab);
  _output2AutoStoptTemp.setTab(&_output2Tab);
  _output2AutoStoptTime.setTab(&_output2Tab);
  _output2Bypass.setTab(&_output2Tab);
  _output2BypassPin.setTab(&_output2Tab);
  _output2BypassType.setTab(&_output2Tab);
  _output2Dimmer.setTab(&_output2Tab);
  _output2DimmerPin.setTab(&_output2Tab);
  _output2DimmerType.setTab(&_output2Tab);
  _output2DimmerPowerLimit.setTab(&_output2Tab);
  _output2Temp.setTab(&_output2Tab);
  _output2TempPin.setTab(&_output2Tab);
  _output2PZEM.setTab(&_output2Tab);
  _output2PZEMRXPin.setTab(&_output2Tab);
  _output2PZEMTXPin.setTab(&_output2Tab);
  _output2PZEMReset.setTab(&_output2Tab);
  _output2PZEMSync.setTab(&_output2Tab);

  _pushButton.setTab(&_systemTab);
  _pushButtonPin.setTab(&_systemTab);
  _pushButtonAction.setTab(&_systemTab);

  _relay1.setTab(&_relay1Tab);
  _relay1Pin.setTab(&_relay1Tab);
  _relay1Power.setTab(&_relay1Tab);
  _relay1Type.setTab(&_relay1Tab);

  _relay2.setTab(&_relay2Tab);
  _relay2Pin.setTab(&_relay2Tab);
  _relay2Power.setTab(&_relay2Tab);
  _relay2Type.setTab(&_relay2Tab);

  _systemTemp.setTab(&_systemTab);
  _systemTempPin.setTab(&_systemTab);

  _led.setTab(&_systemTab);
  _ledGreenPin.setTab(&_systemTab);
  _ledRedPin.setTab(&_systemTab);
  _ledYellowPin.setTab(&_systemTab);

  _gridFreq.setTab(&_electricityTab);
  _gridPowerMQTT.setTab(&_electricityTab);
  _gridVoltageMQTT.setTab(&_electricityTab);
  _jsy.setTab(&_electricityTab);
  _jsyRXPin.setTab(&_electricityTab);
  _jsyTXPin.setTab(&_electricityTab);
  _jsyReset.setTab(&_electricityTab);
  _zcd.setTab(&_electricityTab);
  _zcdPin.setTab(&_electricityTab);
#endif

  // logo

  webServer.rewrite("/dash/assets/logo/mini", "/logo-icon");
  webServer.rewrite("/dash/assets/logo/large", "/logo");
  webServer.rewrite("/dash/assets/logo", "/logo");
  webServer.rewrite("/ota/logo/dark", "/logo");
  webServer.rewrite("/ota/logo/light", "/logo");
  Mycila::HTTPd.get(
    "/logo-icon",
    [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse_P(200, "image/png", logo_icon_png_gz_start, logo_icon_png_gz_end - logo_icon_png_gz_start);
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Cache-Control", "public, max-age=900");
      request->send(response);
    },
    true);
  Mycila::HTTPd.get(
    "/logo",
    [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse_P(200, "image/png", logo_png_gz_start, logo_png_gz_end - logo_png_gz_start);
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Cache-Control", "public, max-age=900");
      request->send(response);
    },
    true);

  // ping

  Mycila::HTTPd.get(
    "/ping",
    [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "pong");
      request->send(response);
    },
    true);

  // dashboard

  dashboard.setAuthentication(YASOLR_ADMIN_USERNAME, Mycila::Config.get(KEY_ADMIN_PASSWORD).c_str());
  webServer.rewrite("/", "/dashboard").setFilter([](AsyncWebServerRequest* request) { return ESPConnect.getState() != ESPConnectState::PORTAL_STARTED; });

  // config

  Mycila::HTTPd.get("/config", [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", config_html_gz_start, config_html_gz_end - config_html_gz_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // ota

#ifdef APP_VERSION_PRO
  ElegantOTA.setID(Mycila::AppInfo.firmware.c_str());
  ElegantOTA.setTitle((Mycila::AppInfo.name + " Web Updater").c_str());
  ElegantOTA.setFWVersion(Mycila::AppInfo.version.c_str());
  ElegantOTA.setFirmwareMode(true);
  ElegantOTA.setFilesystemMode(false);
#endif
  ElegantOTA.setAutoReboot(false);
  ElegantOTA.onStart([this]() { otaPrepareTask.resume(); });
  ElegantOTA.onEnd([this](bool success) {
    if (success) {
      Mycila::Logger.info(TAG, "OTA Update Success! Restarting...");
    } else {
      Mycila::Logger.error(TAG, "OTA Failed! Restarting...");
    }
    restartTask.resume();
  });
  ElegantOTA.begin(&webServer, YASOLR_ADMIN_USERNAME, Mycila::Config.get(KEY_ADMIN_PASSWORD).c_str());

  // app stats

  _firmName.set((Mycila::AppInfo.name.c_str()));
  _firmModel.set((Mycila::AppInfo.model.c_str()));
  _firmManufacturerStat.set(Mycila::AppInfo.manufacturer.c_str());
  _firmVersionStat.set(Mycila::AppInfo.version.c_str());
  _firmFilenameStat.set(Mycila::AppInfo.firmware.c_str());
  _deviceIdStat.set(Mycila::AppInfo.id.c_str());
  _cpuCoresStat.set(String(ESP.getChipCores()).c_str());
  _cpuModelStat.set(ESP.getChipModel());
  _heapMemoryTotalStat.set((String(ESP.getHeapSize()) + " bytes").c_str());
  _bootCountStat.set(String(Mycila::System.getBootCount()).c_str());
  _firmTimeStat.set(Mycila::AppInfo.buildDate.c_str());
  _firmHashStat.set(Mycila::AppInfo.buildHash.c_str());

  // home callbacks

  _outputRelaySwitch(&_output1BypassSwitch, &output1);
  _outputRelaySwitch(&_output2BypassSwitch, &output2);
  _outputDimmerSlider(&_output1DimmerSlider, &output1);
  _outputDimmerSlider(&_output2DimmerSlider, &output2);
  _relaySwitch(&_relay1Switch, NAME_RELAY1);
  _relaySwitch(&_relay2Switch, NAME_RELAY2);

#ifdef APP_VERSION_PRO
  // management

  _boolConfig(&_debugMode, KEY_DEBUG_ENABLE);
  _restart.attachCallback([=] PUSH_BUTTON_CARD_CB { restartTask.resume(); });
  _reset.attachCallback([=] PUSH_BUTTON_CARD_CB { resetTask.resume(); });
  _otaLink.update("/update");
  _consoleLink.update("/console");
  _configBackup.update("/api/config/backup");
  _configRestore.update("/api/config/restore");
  _mqttServerCert.update("/api/config/mqttServerCertificate");

  // display callbacks

  _boolConfig(&_display, KEY_DISPLAY_ENABLE);
  _numConfig(&_displayClockPin, KEY_DISPLAY_CLOCK_PIN);
  _numConfig(&_displayDataPin, KEY_DISPLAY_DATA_PIN);
  _textConfig(&_displayType, KEY_DISPLAY_TYPE);
  _numConfig(&_displayRotation, KEY_DISPLAY_ROTATION);
  _numConfig(&_displayPowerSaveDelay, KEY_DISPLAY_POWER_SAVE_DELAY);

  // electricity callbacks

  _boolConfig(&_zcd, KEY_ZCD_ENABLE);
  _numConfig(&_zcdPin, KEY_ZCD_PIN);
  _boolConfig(&_jsy, KEY_JSY_ENABLE);
  _numConfig(&_jsyRXPin, KEY_JSY_RX_PIN);
  _numConfig(&_jsyTXPin, KEY_JSY_TX_PIN);
  _numConfig(&_gridFreq, KEY_GRID_FREQ);
  _textConfig(&_gridPowerMQTT, KEY_GRID_POWER_MQTT_TOPIC);
  _textConfig(&_gridVoltageMQTT, KEY_GRID_VOLTAGE_MQTT_TOPIC);
  _jsyReset.attachCallback([] PUSH_BUTTON_CARD_CB {
    if (jsy.resetEnergy()) {
      Mycila::Logger.info(TAG, "JSY Energy Reset Success");
    } else {
      Mycila::Logger.error(TAG, "JSY Energy Reset Failed!");
    }
  });

  // mqtt callbacks

  _boolConfig(&_mqtt, KEY_MQTT_ENABLE);
  _boolConfig(&_mqttSecured, KEY_MQTT_SECURED);
  _textConfig(&_mqttServer, KEY_MQTT_SERVER);
  _numConfig(&_mqttPort, KEY_MQTT_PORT);
  _textConfig(&_mqttUser, KEY_MQTT_USERNAME);
  _passwordConfig(&_mqttPwd, KEY_MQTT_PASSWORD);
  _textConfig(&_mqttTopic, KEY_MQTT_TOPIC);
  _sliderConfig(&_mqttPublishInterval, KEY_MQTT_PUBLISH_INTERVAL);
  _boolConfig(&_haDiscovery, KEY_HA_DISCOVERY_ENABLE);
  _textConfig(&_haDiscoveryTopic, KEY_HA_DISCOVERY_TOPIC);

  // network callbacks

  _textConfig(&_hostname, KEY_HOSTNAME);
  _passwordConfig(&_adminPwd, KEY_ADMIN_PASSWORD);
  _textConfig(&_wifiSSID, KEY_WIFI_SSID);
  _passwordConfig(&_wifiPwd, KEY_WIFI_PASSWORD);
  _boolConfig(&_apMode, KEY_AP_MODE_ENABLE);
  _textConfig(&_ntpServer, KEY_NTP_SERVER);
  _textConfig(&_ntpTimezone, KEY_NTP_TIMEZONE);
  _ntpSync.attachCallback([](const char* value) {
    const String str = String(value);
    const size_t len = str.length();
    const timeval tv = {str.substring(0, len - 3).toInt(), str.substring(len - 3).toInt()};
    Mycila::NTP.sync(&tv);
  });

  // output1 callbacks

  _boolConfig(&_output1AutoBypass, KEY_OUTPUT1_AUTO_BYPASS_ENABLE);
  _boolConfig(&_output1AutoDimmer, KEY_OUTPUT1_DIMMER_AUTO);
  _boolConfig(&_output1Bypass, KEY_OUTPUT1_RELAY_ENABLE);
  _boolConfig(&_output1Dimmer, KEY_OUTPUT1_DIMMER_ENABLE);
  _boolConfig(&_output1Temp, KEY_OUTPUT1_TEMP_ENABLE);
  _daysConfig(&_output1AutoStartWDays, KEY_OUTPUT1_AUTO_WEEK_DAYS);
  _numConfig(&_output1AutoStartTemp, KEY_OUTPUT1_AUTO_START_TEMPERATURE);
  _numConfig(&_output1AutoStoptTemp, KEY_OUTPUT1_AUTO_STOP_TEMPERATURE);
  _numConfig(&_output1BypassPin, KEY_OUTPUT1_RELAY_PIN);
  _numConfig(&_output1DimmerPin, KEY_OUTPUT1_DIMMER_PIN);
  _numConfig(&_output1TempPin, KEY_OUTPUT1_TEMP_PIN);
  _sliderConfig(&_output1DimmerPowerLimit, KEY_OUTPUT1_DIMMER_LEVEL_LIMIT);
  _textConfig(&_output1AutoStartTime, KEY_OUTPUT1_AUTO_START_TIME);
  _textConfig(&_output1AutoStoptTime, KEY_OUTPUT1_AUTO_STOP_TIME);
  _textConfig(&_output1BypassType, KEY_OUTPUT1_RELAY_TYPE);
  _textConfig(&_output1DimmerType, KEY_OUTPUT1_DIMMER_TYPE);
  _boolConfig(&_output1PZEM, KEY_OUTPUT1_PZEM_ENABLE);
  _numConfig(&_output1PZEMRXPin, KEY_PZEM_RX_PIN);
  _numConfig(&_output1PZEMTXPin, KEY_PZEM_TX_PIN);
  _output1PZEMSync.attachCallback([] PUSH_BUTTON_CARD_CB { assignOutput1PZEMTask.resume(); });
  _output1PZEMReset.attachCallback([] PUSH_BUTTON_CARD_CB {
    if (output1PZEM.resetEnergy()) {
      Mycila::Logger.info(TAG, "Output1 PZEM Energy Reset Success");
    } else {
      Mycila::Logger.error(TAG, "Output1 PZEM Energy Reset Failed!");
    }
  });

  // output2 callbacks

  _boolConfig(&_output2AutoBypass, KEY_OUTPUT2_AUTO_BYPASS_ENABLE);
  _boolConfig(&_output2AutoDimmer, KEY_OUTPUT2_DIMMER_AUTO);
  _boolConfig(&_output2Bypass, KEY_OUTPUT2_RELAY_ENABLE);
  _boolConfig(&_output2Dimmer, KEY_OUTPUT2_DIMMER_ENABLE);
  _boolConfig(&_output2Temp, KEY_OUTPUT2_TEMP_ENABLE);
  _daysConfig(&_output2AutoStartWDays, KEY_OUTPUT2_AUTO_WEEK_DAYS);
  _numConfig(&_output2AutoStartTemp, KEY_OUTPUT2_AUTO_START_TEMPERATURE);
  _numConfig(&_output2AutoStoptTemp, KEY_OUTPUT2_AUTO_STOP_TEMPERATURE);
  _numConfig(&_output2BypassPin, KEY_OUTPUT2_RELAY_PIN);
  _numConfig(&_output2DimmerPin, KEY_OUTPUT2_DIMMER_PIN);
  _numConfig(&_output2TempPin, KEY_OUTPUT2_TEMP_PIN);
  _sliderConfig(&_output2DimmerPowerLimit, KEY_OUTPUT2_DIMMER_LEVEL_LIMIT);
  _textConfig(&_output2AutoStartTime, KEY_OUTPUT2_AUTO_START_TIME);
  _textConfig(&_output2AutoStoptTime, KEY_OUTPUT2_AUTO_STOP_TIME);
  _textConfig(&_output2BypassType, KEY_OUTPUT2_RELAY_TYPE);
  _textConfig(&_output2DimmerType, KEY_OUTPUT2_DIMMER_TYPE);
  _boolConfig(&_output2PZEM, KEY_OUTPUT2_PZEM_ENABLE);
  _numConfig(&_output2PZEMRXPin, KEY_PZEM_RX_PIN);
  _numConfig(&_output2PZEMTXPin, KEY_PZEM_TX_PIN);
  _output2PZEMSync.attachCallback([] PUSH_BUTTON_CARD_CB { assignOutput2PZEMTask.resume(); });
  _output2PZEMReset.attachCallback([] PUSH_BUTTON_CARD_CB {
    if (output2PZEM.resetEnergy()) {
      Mycila::Logger.info(TAG, "Output2 PZEM Energy Reset Success");
    } else {
      Mycila::Logger.error(TAG, "Output2 PZEM Energy Reset Failed!");
    }
  });

  // relay1 callbacks

  _boolConfig(&_relay1, KEY_RELAY1_ENABLE);
  _numConfig(&_relay1Pin, KEY_RELAY1_PIN);
  _textConfig(&_relay1Type, KEY_RELAY1_TYPE);
  _sliderConfig(&_relay1Power, KEY_RELAY1_POWER);

  // relay2 callbacks

  _boolConfig(&_relay2, KEY_RELAY2_ENABLE);
  _numConfig(&_relay2Pin, KEY_RELAY2_PIN);
  _textConfig(&_relay2Type, KEY_RELAY2_TYPE);
  _sliderConfig(&_relay2Power, KEY_RELAY2_POWER);

  // system callbacks

  _boolConfig(&_systemTemp, KEY_SYSTEM_TEMP_ENABLE);
  _boolConfig(&_pushButton, KEY_BUTTON_ENABLE);
  _boolConfig(&_led, KEY_LIGHTS_ENABLE);
  _boolConfig(&_buzzer, KEY_BUZZER_ENABLE);
  _numConfig(&_systemTempPin, KEY_SYSTEM_TEMP_PIN);
  _numConfig(&_pushButtonPin, KEY_BUTTON_PIN);
  _textConfig(&_pushButtonAction, KEY_BUTTON_ACTION);
  _numConfig(&_ledGreenPin, KEY_LIGHTS_GREEN_PIN);
  _numConfig(&_ledYellowPin, KEY_LIGHTS_YELLOW_PIN);
  _numConfig(&_ledRedPin, KEY_LIGHTS_RED_PIN);
  _numConfig(&_buzzerPin, KEY_BUZZER_PIN);
#endif

  _update(true);
}

void YaSolR::WebsiteClass::_sliderConfig(Card* card, const char* key) {
  card->attachCallback([key](int value) {
    Mycila::Config.set(key, String(MAX(0, value)));
  });
}

void YaSolR::WebsiteClass::_numConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key](const char* value) {
    if (strlen(value) == 0) {
      Mycila::Config.unset(key);
    } else {
      Mycila::Config.set(key, String(strtol(value, nullptr, 10)));
    }
  });
#endif
}

void YaSolR::WebsiteClass::_boolConfig(Card* card, const char* key) {
  card->attachCallback([key, card, this](int value) {
    card->update(value);
    dashboard.refreshCard(card);
    Mycila::Config.setBool(key, value);
  });
}

void YaSolR::WebsiteClass::_textConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key](const char* value) { Mycila::Config.set(key, value); });
#endif
}

void YaSolR::WebsiteClass::_daysConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key, card, this](const char* value) {
    card->update(value);
    dashboard.refreshCard(card);
    Mycila::Config.set(key, strlen(value) == 0 ? "none" : value);
  });
#endif
}

void YaSolR::WebsiteClass::_passwordConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key, card, this](const char* value) {
    if (strlen(value) == 0) {
      Mycila::Config.unset(key);
    } else if (strlen(value) >= 8) {
      Mycila::Config.set(key, value);
    } else {
      card->update(emptyString);
      dashboard.refreshCard(card);
    }
  });
#endif
}

void YaSolR::WebsiteClass::_relaySwitch(Card* card, const char* relayName) {
  card->attachCallback([card, relayName, this](int value) {
    const Mycila::Relay* relay = Mycila::RelayManager.getRelay(relayName);
    if (relay->isEnabled()) {
      Mycila::RelayManager.tryRelayState(relayName, value);
    }
    card->update(relay->isOn());
    dashboard.refreshCard(card);
  });
}

void YaSolR::WebsiteClass::_outputRelaySwitch(Card* card, Mycila::RouterOutput* output) {
  card->attachCallback([card, output, this](int value) {
    if (output->isBypassRelayEnabled()) {
      output->tryBypassRelayState(value);
    }
    card->update(output->isBypassRelayOn());
    dashboard.refreshCard(card);
  });
}

void YaSolR::WebsiteClass::_outputDimmerSlider(Card* card, Mycila::RouterOutput* output) {
  card->attachCallback([card, output, this](int value) { // 0-100
    if (output->getDimmer()->isEnabled()) {
      output->tryDimmerLevel(value);
    }
    card->update(static_cast<int>(output->getDimmer()->getLevel()));
    dashboard.refreshCard(card);
  });
}

namespace YaSolR {
  WebsiteClass Website;
} // namespace YaSolR
