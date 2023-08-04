// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolRWebsite.h>

#include <MycilaESPConnect.h>

#include <MycilaAppInfo.h>
#include <MycilaButton.h>
#include <MycilaBuzzer.h>
#include <MycilaConfig.h>
#include <MycilaDisplay.h>
#include <MycilaGrid.h>
#include <MycilaHTTPd.h>
#include <MycilaJSY.h>
#include <MycilaLights.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaNTP.h>
#include <MycilaRouter.h>
#include <MycilaSystem.h>
#include <MycilaTime.h>
#include <MycilaZeroCrossDetection.h>

#include <YaSolR.h>

#define TAG "WEBSITE"

extern const uint8_t logo_png_gz_start[] asm("_binary__pio_data_logo_png_gz_start");
extern const uint8_t logo_png_gz_end[] asm("_binary__pio_data_logo_png_gz_end");
extern const uint8_t logo_icon_png_gz_start[] asm("_binary__pio_data_logo_icon_png_gz_start");
extern const uint8_t logo_icon_png_gz_end[] asm("_binary__pio_data_logo_icon_png_gz_end");
extern const uint8_t logo_icon_png_gz_start[] asm("_binary__pio_data_logo_icon_png_gz_start");
extern const uint8_t logo_icon_png_gz_end[] asm("_binary__pio_data_logo_icon_png_gz_end");
extern const uint8_t config_html_gz_start[] asm("_binary__pio_data_config_html_gz_start");
extern const uint8_t config_html_gz_end[] asm("_binary__pio_data_config_html_gz_end");

void YaSolR::Website::begin() {
// pro mode
#ifdef APP_VERSION_PRO
  _dashboard.setTitle((Mycila::AppInfo.name + " " + Mycila::AppInfo.model).c_str());

  // dropdowns

  _displayRotation.update("", "0°,90°,180°,270°");
  _displayType.update("", "SH1106,SH1107,SSD1306");
  _gridFreq.update("", "50 Hz,60 Hz");
  _output1BypassType.update("", "NO,NC");
  _output2BypassType.update("", "NO,NC");
  _pushButtonAction.update("", "restart,bypass");
  _relay1Type.update("", "NO,NC");
  _relay2Type.update("", "NO,NC");
  _output1DimmerType.update("", "TRIAC,SSR RANDOM,SSR ZC");
  _output2DimmerType.update("", "TRIAC,SSR RANDOM,SSR ZC");
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
  _stateJSY.setTab(&_stateTab);
  _stateLEDs.setTab(&_stateTab);
  _stateMQTT.setTab(&_stateTab);
  _stateNTP.setTab(&_stateTab);
  _stateOutput1AutoBypass.setTab(&_stateTab);
  _stateOutput1AutoDimmer.setTab(&_stateTab);
  _stateOutput1Bypass.setTab(&_stateTab);
  _stateOutput1Dimmer.setTab(&_stateTab);
  _stateOutput1Temp.setTab(&_stateTab);
  _stateOutput2AutoBypass.setTab(&_stateTab);
  _stateOutput2AutoDimmer.setTab(&_stateTab);
  _stateOutput2Bypass.setTab(&_stateTab);
  _stateOutput2Dimmer.setTab(&_stateTab);
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
  _jsy.setTab(&_electricityTab);
  _jsyRXPin.setTab(&_electricityTab);
  _jsyTXPin.setTab(&_electricityTab);
  _jsyReset.setTab(&_electricityTab);
  _zcd.setTab(&_electricityTab);
  _zcdPin.setTab(&_electricityTab);
#endif

  // logo

  Mycila::HTTPd.server.rewrite("/dash/assets/logo/mini", "/logo-icon");
  Mycila::HTTPd.server.rewrite("/dash/assets/logo/large", "/logo");
  Mycila::HTTPd.server.rewrite("/dash/assets/logo", "/logo");
  Mycila::HTTPd.server.rewrite("/ota/logo/dark", "/logo");
  Mycila::HTTPd.server.rewrite("/ota/logo/light", "/logo");
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

  _dashboard.setAuthentication(Mycila::HTTPdConfig.getUsername().c_str(), Mycila::HTTPdConfig.getPassword().c_str());
  Mycila::HTTPd.server.rewrite("/", "/dashboard").setFilter([](AsyncWebServerRequest* request) { return ESPConnect.getState() != ESPConnectState::PORTAL_STARTED; });

  // config

  Mycila::HTTPd.get("/config", [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", config_html_gz_start, config_html_gz_end - config_html_gz_start);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    });

  // app stats

  _appName.set((Mycila::AppInfo.name + " " + Mycila::AppInfo.model).c_str());
  _appManufacturerStat.set(Mycila::AppInfo.manufacturer.c_str());
  _appVersionStat.set(Mycila::AppInfo.version.c_str());
  _appFirmwareStat.set(Mycila::AppInfo.firmware.c_str());
  _appIdStat.set(Mycila::AppInfo.id.c_str());
  _cpuCoresStat.set(String(ESP.getChipCores()).c_str());
  _cpuModelStat.set(ESP.getChipModel());
  _memoryTotalStat.set((String(ESP.getHeapSize()) + " bytes").c_str());
  _macAddressStat.set(ESPConnect.getMacAddress().c_str());
  _bootCountStat.set(String(Mycila::System.getBootCount()).c_str());

  // home callbacks

  _outputRelaySwitch(&_output1BypassSwitch, 0);
  _outputRelaySwitch(&_output2BypassSwitch, 1);
  _outputDimmerSlider(&_output1DimmerSlider, 0);
  _outputDimmerSlider(&_output2DimmerSlider, 1);
  _relaySwitch(&_relay1Switch, 0);
  _relaySwitch(&_relay2Switch, 1);

#ifdef APP_VERSION_PRO
  // management

  _boolConfig(&_debugMode, KEY_DEBUG_ENABLE);
  _restart.attachCallback(PUSH_BUTTON_CARD_CB { Mycila::System.restart(); });
  _reset.attachCallback(PUSH_BUTTON_CARD_CB { Mycila::System.reset(); });
  _otaLink.update("/update");
  _consoleLink.update("/console");
  _configBackup.update("/api/config/backup");
  _configRestore.update("/api/config/restore");

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
  _jsyReset.attachCallback(PUSH_BUTTON_CARD_CB {
    Mycila::JSY.resetEnergy();
  });

  // mqtt callbacks

  _boolConfig(&_mqtt, KEY_MQTT_ENABLE);
  _boolConfig(&_mqttSecured, KEY_MQTT_SECURED);
  _textConfig(&_mqttServer, KEY_MQTT_SERVER);
  _numConfig(&_mqttPort, KEY_MQTT_PORT);
  _textConfig(&_mqttUser, KEY_MQTT_USERNAME);
  _passwordConfig(&_mqttPwd, KEY_MQTT_PASSWORD);
  _textConfig(&_mqttTopic, KEY_MQTT_TOPIC);
  _numConfig(&_mqttPublishInterval, KEY_MQTT_PUBLISH_INTERVAL);
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
  _sliderConfig(&_output1DimmerPowerLimit, KEY_OUTPUT1_DIMMER_LEVEL_LIMIT);
  _numConfig(&_output1TempPin, KEY_OUTPUT1_TEMP_PIN);
  _textConfig(&_output1AutoStartTime, KEY_OUTPUT1_AUTO_START_TIME);
  _textConfig(&_output1AutoStoptTime, KEY_OUTPUT1_AUTO_STOP_TIME);
  _textConfig(&_output1BypassType, KEY_OUTPUT1_RELAY_TYPE);
  _textConfig(&_output1DimmerType, KEY_OUTPUT1_DIMMER_TYPE);

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
  _sliderConfig(&_output2DimmerPowerLimit, KEY_OUTPUT2_DIMMER_LEVEL_LIMIT);
  _numConfig(&_output2TempPin, KEY_OUTPUT2_TEMP_PIN);
  _textConfig(&_output2AutoStartTime, KEY_OUTPUT2_AUTO_START_TIME);
  _textConfig(&_output2AutoStoptTime, KEY_OUTPUT2_AUTO_STOP_TIME);
  _textConfig(&_output2BypassType, KEY_OUTPUT2_RELAY_TYPE);
  _textConfig(&_output2DimmerType, KEY_OUTPUT2_DIMMER_TYPE);

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

  update(true);
}

void YaSolR::Website::loop() {
  if (millis() - _lastUpdate >= YASOLR_WEBSITE_UPDATE_INTERVAL * 1000) {
    update();
  }
}

void YaSolR::Website::update(bool skipWebSocketPush) {
  if (_dashboard.isAsyncAccessInProgress()) {
    return;
  }

  // const uint32_t start = millis();

  // stats

  Mycila::SystemMemory memory = Mycila::System.getMemory();
  _timeStat.set(Mycila::Time::getLocalStr().c_str());
  _gridFreqStat.set((String(Mycila::Grid.getFrequency()) + " Hz").c_str());
  _gridVoltStat.set((String(Mycila::Grid.getVoltage()) + " V").c_str());
  _ipAddressStat.set(ESPConnect.getIPAddress().toString().c_str());
  _memoryUsageStat.set((String(memory.usage) + " %").c_str());
  _memoryUsedStat.set((String(memory.used) + " bytes").c_str());
  _output1SwitchCountStat.set(String(Mycila::Router.outputs[0].relay.getSwitchCount()).c_str());
  _output2SwitchCountStat.set(String(Mycila::Router.outputs[1].relay.getSwitchCount()).c_str());
  _relay1SwitchCountStat.set(String(Mycila::RelayManager.relays[0].getSwitchCount()).c_str());
  _relay2SwitchCountStat.set(String(Mycila::RelayManager.relays[1].getSwitchCount()).c_str());
  _systemTempStat.set(_systemTempSensor->isValid() ? (String(_systemTempSensor->getTemperature()) + " °C").c_str() : "");
  _uptimeStat.set((String(Mycila::System.getUptime()) + " s").c_str());
  _wifiRSSIStat.set((String(ESPConnect.getWiFiRSSI()) + " dBm").c_str());
  _wifiSignalStat.set((String(ESPConnect.getWiFiSignalQuality()) + " %").c_str());
  _wifiSSIDStat.set(ESPConnect.getWiFiSSID().c_str());

  // home

  switch (Mycila::Router.outputs[0].getState()) {
    case Mycila::RouterOutputState::OUTPUT_DISABLED:
      _output1State.update(Mycila::Router.outputs[0].getStateString(), DASH_STATUS_DANGER);
      break;
    case Mycila::RouterOutputState::OUTPUT_IDLE:
      _output1State.update(Mycila::Router.outputs[0].getStateString(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutputState::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutputState::OUTPUT_BYPASS_MANUAL:
      _output1State.update(Mycila::Router.outputs[0].getStateString(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutputState::OUTPUT_ROUTING:
      _output1State.update(Mycila::Router.outputs[0].getStateString(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output1State.update("Unknown", DASH_STATUS_DANGER);
      break;
  }
  _temperature(&_output1TempState, &Mycila::Router.outputs[0].temperatureSensor);
  _output1BypassSwitch.update(Mycila::Router.outputs[0].isBypassRelayOn());
  _output1DimmerSlider.update(static_cast<int32_t>(Mycila::Router.outputs[0].dimmer.getLevel()));

  switch (Mycila::Router.outputs[1].getState()) {
    case Mycila::RouterOutputState::OUTPUT_DISABLED:
      _output2State.update(Mycila::Router.outputs[1].getStateString(), DASH_STATUS_DANGER);
      break;
    case Mycila::RouterOutputState::OUTPUT_IDLE:
      _output2State.update(Mycila::Router.outputs[1].getStateString(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutputState::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutputState::OUTPUT_BYPASS_MANUAL:
      _output2State.update(Mycila::Router.outputs[1].getStateString(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutputState::OUTPUT_ROUTING:
      _output2State.update(Mycila::Router.outputs[1].getStateString(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output2State.update("Unknown", DASH_STATUS_DANGER);
      break;
  }
  _temperature(&_output2TempState, &Mycila::Router.outputs[1].temperatureSensor);
  _output2BypassSwitch.update(Mycila::Router.outputs[1].isBypassRelayOn());
  _output2DimmerSlider.update(static_cast<int32_t>(Mycila::Router.outputs[1].dimmer.getLevel()));

  _relay1Switch.update(Mycila::RelayManager.relays[0].isOn());
  _relay2Switch.update(Mycila::RelayManager.relays[1].isOn());
  _gridPower.update(Mycila::Grid.getPower());
  _routerPower.update(Mycila::Router.getTotalRoutedPower());
  _routerPowerFactor.update(Mycila::Router.getTotalPowerFactor() * 100);
  _routerEnergy.update(Mycila::Router.getTotalRoutedEnergy());
  _routerTHDi.update(Mycila::Router.getTotalTHDi() * 100);

#ifdef APP_VERSION_PRO
  // management tab

  _debugMode.update(Mycila::Config.getBool(KEY_DEBUG_ENABLE));

  // health

  const bool gridOnline = Mycila::Grid.isOnline();
  _status(&_stateBuzzer, KEY_BUZZER_ENABLE, Mycila::Buzzer.isEnabled());
  _status(&_stateDisplay, KEY_DISPLAY_ENABLE, Mycila::Display.isEnabled());
  _status(&_stateJSY, KEY_JSY_ENABLE, Mycila::JSY.isEnabled(), gridOnline, "No grid voltage");
  _status(&_stateLEDs, KEY_LIGHTS_ENABLE, Mycila::Lights.isEnabled());
  _status(&_stateMQTT, KEY_MQTT_ENABLE, Mycila::MQTT.isEnabled(), Mycila::MQTT.isConnected(), "Disconnected");
  _status(&_stateOutput1AutoBypass, KEY_OUTPUT1_AUTO_BYPASS_ENABLE, true, Mycila::NTP.isSynced(), "Waiting for time sync");
  _status(&_stateOutput1AutoDimmer, KEY_OUTPUT1_DIMMER_AUTO, true, Mycila::Router.outputs[0].dimmer.isEnabled(), "Dimmer disabled");
  _status(&_stateOutput1Bypass, KEY_OUTPUT1_RELAY_ENABLE, Mycila::Router.outputs[0].isBypassRelayEnabled());
  _status(&_stateOutput1Dimmer, KEY_OUTPUT1_DIMMER_ENABLE, Mycila::Router.outputs[0].dimmer.isEnabled(), gridOnline, "No grid voltage");
  _status(&_stateOutput1Temp, KEY_OUTPUT1_TEMP_ENABLE, Mycila::Router.outputs[0].temperatureSensor.isEnabled(), Mycila::Router.outputs[0].temperatureSensor.isValid(), "Read error");
  _status(&_stateOutput2AutoBypass, KEY_OUTPUT2_AUTO_BYPASS_ENABLE, true, Mycila::NTP.isSynced(), "Waiting for time sync");
  _status(&_stateOutput2AutoDimmer, KEY_OUTPUT2_DIMMER_AUTO, true, Mycila::Router.outputs[0].dimmer.isEnabled(), "Dimmer disabled");
  _status(&_stateOutput2Bypass, KEY_OUTPUT2_RELAY_ENABLE, Mycila::Router.outputs[1].isBypassRelayEnabled());
  _status(&_stateOutput2Dimmer, KEY_OUTPUT2_DIMMER_ENABLE, Mycila::Router.outputs[1].dimmer.isEnabled(), gridOnline, "No grid voltage");
  _status(&_stateOutput2Temp, KEY_OUTPUT2_TEMP_ENABLE, Mycila::Router.outputs[1].temperatureSensor.isEnabled(), Mycila::Router.outputs[1].temperatureSensor.isValid(), "Read error");
  _status(&_statePushButton, KEY_BUTTON_ENABLE, Mycila::Button.isEnabled());
  _status(&_stateRelay1, KEY_RELAY1_ENABLE, Mycila::RelayManager.relays[0].isEnabled());
  _status(&_stateRelay2, KEY_RELAY2_ENABLE, Mycila::RelayManager.relays[1].isEnabled());
  _status(&_stateSystemTemp, KEY_SYSTEM_TEMP_ENABLE, _systemTempSensor->isEnabled(), _systemTempSensor->isValid(), "Read error");
  _status(&_stateZCD, KEY_ZCD_ENABLE, Mycila::ZCD.isEnabled(), gridOnline, "No grid voltage");
  // ntp
  if (Mycila::NTP.isSynced() && !Mycila::NTP.getTimezoneInfo().isEmpty())
    _stateNTP.update("Enabled", DASH_STATUS_SUCCESS);
  else if (Mycila::NTP.getTimezoneInfo().isEmpty())
    _stateNTP.update("Invalid timezone", DASH_STATUS_DANGER);
  else if (!Mycila::NTP.isSynced())
    _stateNTP.update("Waiting for time sync", DASH_STATUS_WARNING);

  // pinout live

  std::map<int32_t, Card*> pinout = {};
  _pinout(&_pinLiveBuzzer, Mycila::Buzzer.getPin(), &pinout);
  _pinout(&_pinLiveDisplayClock, Mycila::Display.getClockPin(), &pinout);
  _pinout(&_pinLiveDisplayData, Mycila::Display.getDataPin(), &pinout);
  _pinout(&_pinLiveJsyRX, Mycila::JSY.getRXPin(), &pinout);
  _pinout(&_pinLiveJsyTX, Mycila::JSY.getTXPin(), &pinout);
  _pinout(&_pinLiveLEDGreen, Mycila::Lights.getGreenPin(), &pinout);
  _pinout(&_pinLiveLEDRed, Mycila::Lights.getRedPin(), &pinout);
  _pinout(&_pinLiveLEDYellow, Mycila::Lights.getYellowPin(), &pinout);
  _pinout(&_pinLiveOutput1Bypass, Mycila::Router.outputs[0].relay.getPin(), &pinout);
  _pinout(&_pinLiveOutput1Dimmer, Mycila::Router.outputs[0].dimmer.getPin(), &pinout);
  _pinout(&_pinLiveOutput1Temp, Mycila::Router.outputs[0].temperatureSensor.getPin(), &pinout);
  _pinout(&_pinLiveOutput2Bypass, Mycila::Router.outputs[1].relay.getPin(), &pinout);
  _pinout(&_pinLiveOutput2Dimmer, Mycila::Router.outputs[1].dimmer.getPin(), &pinout);
  _pinout(&_pinLiveOutput2Temp, Mycila::Router.outputs[1].temperatureSensor.getPin(), &pinout);
  _pinout(&_pinLivePushButton, Mycila::Button.getPin(), &pinout);
  _pinout(&_pinLiveRelay1, Mycila::RelayManager.relays[0].getPin(), &pinout);
  _pinout(&_pinLiveRelay2, Mycila::RelayManager.relays[1].getPin(), &pinout);
  _pinout(&_pinLiveSystemTemp, _systemTempSensor->getPin(), &pinout);
  _pinout(&_pinLiveZCD, Mycila::ZCD.getPin(), &pinout);
  pinout.clear();

  // pinout config

  _pinout(&_pinConfigRelay1, Mycila::Config.get(KEY_RELAY1_PIN).toInt(), &pinout);
  _pinout(&_pinConfigOutput1Dimmer, Mycila::Config.get(KEY_OUTPUT1_DIMMER_PIN).toInt(), &pinout);
  _pinout(&_pinConfigOutput1Bypass, Mycila::Config.get(KEY_OUTPUT1_RELAY_PIN).toInt(), &pinout);
  _pinout(&_pinConfigOutput1Temp, Mycila::Config.get(KEY_OUTPUT1_TEMP_PIN).toInt(), &pinout);
  _pinout(&_pinConfigRelay2, Mycila::Config.get(KEY_RELAY2_PIN).toInt(), &pinout);
  _pinout(&_pinConfigOutput2Dimmer, Mycila::Config.get(KEY_OUTPUT2_DIMMER_PIN).toInt(), &pinout);
  _pinout(&_pinConfigOutput2Bypass, Mycila::Config.get(KEY_OUTPUT2_RELAY_PIN).toInt(), &pinout);
  _pinout(&_pinConfigOutput2Temp, Mycila::Config.get(KEY_OUTPUT2_TEMP_PIN).toInt(), &pinout);
  _pinout(&_pinConfigZCD, Mycila::Config.get(KEY_ZCD_PIN).toInt(), &pinout);
  _pinout(&_pinConfigJsyRX, Mycila::Config.get(KEY_JSY_RX_PIN).toInt(), &pinout);
  _pinout(&_pinConfigJsyTX, Mycila::Config.get(KEY_JSY_TX_PIN).toInt(), &pinout);
  _pinout(&_pinConfigSystemTemp, Mycila::Config.get(KEY_SYSTEM_TEMP_PIN).toInt(), &pinout);
  _pinout(&_pinConfigDisplayClock, Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN).toInt(), &pinout);
  _pinout(&_pinConfigDisplayData, Mycila::Config.get(KEY_DISPLAY_DATA_PIN).toInt(), &pinout);
  _pinout(&_pinConfigPushButton, Mycila::Config.get(KEY_BUTTON_PIN).toInt(), &pinout);
  _pinout(&_pinConfigBuzzer, Mycila::Config.get(KEY_BUZZER_PIN).toInt(), &pinout);
  _pinout(&_pinConfigLEDGreen, Mycila::Config.get(KEY_LIGHTS_GREEN_PIN).toInt(), &pinout);
  _pinout(&_pinConfigLEDYellow, Mycila::Config.get(KEY_LIGHTS_YELLOW_PIN).toInt(), &pinout);
  _pinout(&_pinConfigLEDRed, Mycila::Config.get(KEY_LIGHTS_RED_PIN).toInt(), &pinout);
  pinout.clear();

  // display

  _display.update(Mycila::Config.getBool(KEY_DISPLAY_ENABLE));
  _displayClockPin.update(String(Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN).toInt()));
  _displayDataPin.update(String(Mycila::Config.get(KEY_DISPLAY_DATA_PIN).toInt()));
  _displayType.update(Mycila::Config.get(KEY_DISPLAY_TYPE));
  _displayRotation.update(String(Mycila::Config.get(KEY_DISPLAY_ROTATION).toInt()) + "°");
  _displayPowerSaveDelay.update(String(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY).toInt()));

  // electricity

  _zcd.update(Mycila::Config.getBool(KEY_ZCD_ENABLE));
  _zcdPin.update(String(Mycila::Config.get(KEY_ZCD_PIN).toInt()));
  _jsy.update(Mycila::Config.getBool(KEY_JSY_ENABLE));
  _jsyRXPin.update(String(Mycila::Config.get(KEY_JSY_RX_PIN).toInt()));
  _jsyTXPin.update(String(Mycila::Config.get(KEY_JSY_TX_PIN).toInt()));
  _gridFreq.update(Mycila::Config.get(KEY_GRID_FREQ).toInt() == 60 ? "60 Hz" : "50 Hz");
  _gridPowerMQTT.update(Mycila::Config.get(KEY_GRID_POWER_MQTT_TOPIC));

  // mqtt

  _mqtt.update(Mycila::Config.getBool(KEY_MQTT_ENABLE));
  _mqttSecured.update(Mycila::Config.getBool(KEY_MQTT_SECURED));
  _mqttServer.update(Mycila::Config.get(KEY_MQTT_SERVER));
  _mqttPort.update(String(Mycila::Config.get(KEY_MQTT_PORT).toInt()));
  _mqttUser.update(Mycila::Config.get(KEY_MQTT_USERNAME));
  String pwd = Mycila::Config.get(KEY_MQTT_PASSWORD);
  _mqttPwd.update(pwd.isEmpty() ? "" : HIDDEN_PWD);
  _mqttTopic.update(Mycila::Config.get(KEY_MQTT_TOPIC));
  _mqttPublishInterval.update(String(Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt()));
  _haDiscovery.update(Mycila::Config.getBool(KEY_HA_DISCOVERY_ENABLE));
  _haDiscoveryTopic.update(Mycila::Config.get(KEY_HA_DISCOVERY_TOPIC));

  // network

  _hostname.update(Mycila::Config.get(KEY_HOSTNAME));
  pwd = Mycila::Config.get(KEY_ADMIN_PASSWORD);
  _adminPwd.update(pwd.isEmpty() ? "" : HIDDEN_PWD);
  _wifiSSID.update(Mycila::Config.get(KEY_WIFI_SSID));
  pwd = Mycila::Config.get(KEY_WIFI_PASSWORD);
  _wifiPwd.update(pwd.isEmpty() ? "" : HIDDEN_PWD);
  _apMode.update(Mycila::Config.getBool(KEY_AP_MODE_ENABLE));
  _ntpServer.update(Mycila::Config.get(KEY_NTP_SERVER));
  _ntpTimezone.update(Mycila::Config.get(KEY_NTP_TIMEZONE));

  // output1

  _output1AutoBypass.update(Mycila::Config.getBool(KEY_OUTPUT1_AUTO_BYPASS_ENABLE));
  _output1AutoDimmer.update(Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_AUTO));
  _output1AutoStartTemp.update(String(Mycila::Config.get(KEY_OUTPUT1_AUTO_START_TEMPERATURE).toInt()));
  _output1AutoStartTime.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_START_TIME));
  _output1AutoStartWDays.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_WEEK_DAYS));
  _output1AutoStoptTemp.update(String(Mycila::Config.get(KEY_OUTPUT1_AUTO_STOP_TEMPERATURE).toInt()));
  _output1AutoStoptTime.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_STOP_TIME));
  _output1Bypass.update(Mycila::Config.getBool(KEY_OUTPUT1_RELAY_ENABLE));
  _output1BypassPin.update(String(Mycila::Config.get(KEY_OUTPUT1_RELAY_PIN).toInt()));
  _output1BypassType.update(Mycila::Config.get(KEY_OUTPUT1_RELAY_TYPE));
  _output1Dimmer.update(Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_ENABLE));
  _output1DimmerPin.update(String(Mycila::Config.get(KEY_OUTPUT1_DIMMER_PIN).toInt()));
  _output1DimmerPowerLimit.update(static_cast<int32_t>(Mycila::Config.get(KEY_OUTPUT1_DIMMER_LEVEL_LIMIT).toInt()));
  _output1Temp.update(Mycila::Config.getBool(KEY_OUTPUT1_TEMP_ENABLE));
  _output1TempPin.update(String(Mycila::Config.get(KEY_OUTPUT1_TEMP_PIN).toInt()));

  // output2

  _output2AutoBypass.update(Mycila::Config.getBool(KEY_OUTPUT2_AUTO_BYPASS_ENABLE));
  _output2AutoDimmer.update(Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_AUTO));
  _output2AutoStartTemp.update(String(Mycila::Config.get(KEY_OUTPUT2_AUTO_START_TEMPERATURE).toInt()));
  _output2AutoStartTime.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_START_TIME));
  _output2AutoStartWDays.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_WEEK_DAYS));
  _output2AutoStoptTemp.update(String(Mycila::Config.get(KEY_OUTPUT2_AUTO_STOP_TEMPERATURE).toInt()));
  _output2AutoStoptTime.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_STOP_TIME));
  _output2Bypass.update(Mycila::Config.getBool(KEY_OUTPUT2_RELAY_ENABLE));
  _output2BypassPin.update(String(Mycila::Config.get(KEY_OUTPUT2_RELAY_PIN).toInt()));
  _output2BypassType.update(Mycila::Config.get(KEY_OUTPUT2_RELAY_TYPE));
  _output2Dimmer.update(Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_ENABLE));
  _output2DimmerPin.update(String(Mycila::Config.get(KEY_OUTPUT2_DIMMER_PIN).toInt()));
  _output2DimmerPowerLimit.update(static_cast<int32_t>(Mycila::Config.get(KEY_OUTPUT2_DIMMER_LEVEL_LIMIT).toInt()));
  _output2Temp.update(Mycila::Config.getBool(KEY_OUTPUT2_TEMP_ENABLE));
  _output2TempPin.update(String(Mycila::Config.get(KEY_OUTPUT2_TEMP_PIN).toInt()));

  // relay1

  _relay1.update(Mycila::Config.getBool(KEY_RELAY1_ENABLE));
  _relay1Pin.update(String(Mycila::Config.get(KEY_RELAY1_PIN).toInt()));
  _relay1Type.update(Mycila::Config.get(KEY_RELAY1_TYPE));
  _relay1Power.update(static_cast<int32_t>(Mycila::Config.get(KEY_RELAY1_POWER).toInt()));

  // relay2

  _relay2.update(Mycila::Config.getBool(KEY_RELAY2_ENABLE));
  _relay2Pin.update(String(Mycila::Config.get(KEY_RELAY2_PIN).toInt()));
  _relay2Type.update(Mycila::Config.get(KEY_RELAY2_TYPE));
  _relay2Power.update(static_cast<int32_t>(Mycila::Config.get(KEY_RELAY2_POWER).toInt()));

  // system

  _systemTemp.update(Mycila::Config.getBool(KEY_SYSTEM_TEMP_ENABLE));
  _pushButton.update(Mycila::Config.getBool(KEY_BUTTON_ENABLE));
  _led.update(Mycila::Config.getBool(KEY_LIGHTS_ENABLE));
  _buzzer.update(Mycila::Config.getBool(KEY_BUZZER_ENABLE));
  _systemTempPin.update(String(Mycila::Config.get(KEY_SYSTEM_TEMP_PIN).toInt()));
  _pushButtonPin.update(String(Mycila::Config.get(KEY_BUTTON_PIN).toInt()));
  _pushButtonAction.update(Mycila::Config.get(KEY_BUTTON_ACTION));
  _ledGreenPin.update(String(Mycila::Config.get(KEY_LIGHTS_GREEN_PIN).toInt()));
  _ledYellowPin.update(String(Mycila::Config.get(KEY_LIGHTS_YELLOW_PIN).toInt()));
  _ledRedPin.update(String(Mycila::Config.get(KEY_LIGHTS_RED_PIN).toInt()));
  _buzzerPin.update(String(Mycila::Config.get(KEY_BUZZER_PIN).toInt()));
#endif

  if (!skipWebSocketPush && _dashboard.hasClient()) {
    _dashboard.sendUpdates();
  }

  _lastUpdate = millis();
  // Mycila::Logger.debug(TAG, "Published in %u ms", (_lastUpdate - start));
}

void YaSolR::Website::_sliderConfig(Card* card, const char* key) {
  card->attachCallback([key](int value) {
    Mycila::Config.set(key, String(MAX(0, value)));
  });
}

void YaSolR::Website::_numConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key](const char* value) {
    if (strlen(value) == 0) {
      Mycila::Config.unset(key);
    } else {
      Mycila::Config.set(key, String(MAX(0, strtol(value, nullptr, 10))));
    }
  });
#endif
}

void YaSolR::Website::_boolConfig(Card* card, const char* key) {
  card->attachCallback([key, card, this](int value) {
    card->update(value);
    _dashboard.refreshCard(card);
    Mycila::Config.setBool(key, value);
  });
}

void YaSolR::Website::_textConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key](const char* value) { Mycila::Config.set(key, value); });
#endif
}

void YaSolR::Website::_daysConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key, card, this](const char* value) {
    card->update(value);
    _dashboard.refreshCard(card);
    Mycila::Config.set(key, strlen(value) == 0 ? "none" : value);
  });
#endif
}

void YaSolR::Website::_passwordConfig(Card* card, const char* key) {
#ifdef APP_VERSION_PRO
  card->attachCallback([key, card, this](const char* value) {
    if (strlen(value) == 0) {
      Mycila::Config.unset(key);
    } else if (strlen(value) >= 8) {
      Mycila::Config.set(key, value);
    } else {
      card->update(emptyString);
      _dashboard.refreshCard(card);
    }
  });
#endif
}

void YaSolR::Website::_relaySwitch(Card* card, int idx) {
  card->attachCallback([card, idx, this](int value) {
    if (Mycila::RelayManager.relays[idx].isEnabled()) {
      Mycila::RelayManager.tryRelayState(idx, value);
    }
    card->update(Mycila::RelayManager.relays[idx].isOn());
    _dashboard.refreshCard(card);
  });
}

void YaSolR::Website::_outputRelaySwitch(Card* card, int idx) {
  card->attachCallback([card, idx, this](int value) {
    if (Mycila::Router.outputs[idx].isBypassRelayEnabled()) {
      Mycila::Router.outputs[idx].tryBypassRelayState(value);
    }
    card->update(Mycila::Router.outputs[idx].isBypassRelayOn());
    _dashboard.refreshCard(card);
  });
}

void YaSolR::Website::_outputDimmerSlider(Card* card, int idx) {
  card->attachCallback([card, idx, this](int value) { // 0-100
    if (Mycila::Router.outputs[idx].dimmer.isEnabled()) {
      Mycila::Router.outputs[idx].tryDimmerLevel(value);
    }
    card->update(static_cast<int32_t>(Mycila::Router.outputs[idx].dimmer.getLevel()));
    _dashboard.refreshCard(card);
  });
}

void YaSolR::Website::_temperature(Card* card, Mycila::TemperatureSensor* sensor) {
  if (!sensor->isEnabled()) {
    card->update("Disabled", "");
  } else if (!sensor->isValid()) {
    card->update("Pending...", "");
  } else {
    card->update(sensor->getTemperature(), "°C");
  }
}

void YaSolR::Website::_status(Card* card, const char* key, bool enabled, bool active, const char* err) {
  const bool configEnabled = Mycila::Config.getBool(key);
  if (!configEnabled)
    card->update("Disabled", DASH_STATUS_DANGER);
  else if (!enabled)
    card->update("Unable to start", DASH_STATUS_WARNING);
  else if (!active)
    card->update(err, DASH_STATUS_WARNING);
  else
    card->update("Enabled", DASH_STATUS_SUCCESS);
}

void YaSolR::Website::_pinout(Card* card, int32_t pin, std::map<int32_t, Card*>* pinout) {
  if (pin == GPIO_NUM_NC) {
    card->update("Disabled", DASH_STATUS_DANGER);
  } else if (pinout->find(pin) != pinout->end()) {
    String v = String(pin) + " (Duplicate)";
    pinout->at(pin)->update(v, DASH_STATUS_WARNING);
    card->update(v, DASH_STATUS_WARNING);
  } else if (!GPIO_IS_VALID_GPIO(pin)) {
    pinout->insert(std::pair<int32_t, Card*>(pin, card));
    card->update(String(pin) + " (Invalid)", DASH_STATUS_WARNING);
  } else if (!GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    pinout->insert(std::pair<int32_t, Card*>(pin, card));
    card->update(String(pin) + " (Input Only)", DASH_STATUS_SUCCESS);
  } else {
    pinout->insert(std::pair<int32_t, Card*>(pin, card));
    card->update(String(pin) + " (I/O)", DASH_STATUS_SUCCESS);
  }
}
