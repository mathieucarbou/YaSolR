// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaHTTPd.h>
#include <MycilaTemperatureSensor.h>
#include <map>

#ifdef APP_VERSION_PRO
#include <ESPDashPro.h>
#define PUSH_BUTTON_CARD_CB ()
#else
#include <ESPDash.h>
#define ENERGY_CARD GENERIC_CARD
#define PUSH_BUTTON_CARD BUTTON_CARD
#define PUSH_BUTTON_CARD_CB (int32_t value)
#endif

namespace YaSolR {
  class Website {
    public:
      void begin();
      void update();

    private:
      ESPDash _dashboard = ESPDash(&Mycila::HTTPd.server, "/dashboard", false);

      Statistic _appFirmwareStat = Statistic(&_dashboard, "App Firmware");
      Statistic _appIdStat = Statistic(&_dashboard, "App Id");
      Statistic _appManufacturerStat = Statistic(&_dashboard, "App Manufacturer");
      Statistic _appName = Statistic(&_dashboard, "App Name");
      Statistic _appVersionStat = Statistic(&_dashboard, "App Version");
      Statistic _bootCountStat = Statistic(&_dashboard, "Boot Count");
      Statistic _cpuCoresStat = Statistic(&_dashboard, "CPU Cores");
      Statistic _cpuModelStat = Statistic(&_dashboard, "CPU Model");
      Statistic _gridFreqStat = Statistic(&_dashboard, "Grid Frequency");
      Statistic _gridVoltStat = Statistic(&_dashboard, "Grid Voltage");
      Statistic _memoryTotalStat = Statistic(&_dashboard, "Heap Memory Total");
      Statistic _memoryUsageStat = Statistic(&_dashboard, "Heap Memory Usage");
      Statistic _memoryUsedStat = Statistic(&_dashboard, "Heap Memory Used");
      Statistic _ipAddressStat = Statistic(&_dashboard, "IP Address");
      Statistic _macAddressStat = Statistic(&_dashboard, "MAC Address");
      Statistic _netModeStat = Statistic(&_dashboard, "Network Mode");
      Statistic _output1SwitchCountStat = Statistic(&_dashboard, "Output 1 Switch Count");
      Statistic _output2SwitchCountStat = Statistic(&_dashboard, "Output 2 Switch Count");
      Statistic _relay1SwitchCountStat = Statistic(&_dashboard, "Relay 1 Switch Count");
      Statistic _relay2SwitchCountStat = Statistic(&_dashboard, "Relay 2 Switch Count");
      Statistic _systemTempStat = Statistic(&_dashboard, "System Temperature");
      Statistic _timeStat = Statistic(&_dashboard, "Time");
      Statistic _uptimeStat = Statistic(&_dashboard, "Uptime");
      Statistic _wifiRSSIStat = Statistic(&_dashboard, "WiFi RSSI");
      Statistic _wifiSignalStat = Statistic(&_dashboard, "WiFi Signal");
      Statistic _wifiSSIDStat = Statistic(&_dashboard, "WiFi SSID");
#ifdef APP_VERSION_TRIAL
      Statistic _trialRemainingStat = Statistic(&_dashboard, "Trial Time Remaining");
#endif

      // home

      Card _gridPower = Card(&_dashboard, ENERGY_CARD, "Grid Power", "W");
      Card _routerPower = Card(&_dashboard, ENERGY_CARD, "Routed Power", "W");
      Card _routerPowerFactor = Card(&_dashboard, ENERGY_CARD, "Router Power Factor", "%");
      Card _routerTHDi = Card(&_dashboard, ENERGY_CARD, "Router THDi", "%");

      Card _output1State = Card(&_dashboard, STATUS_CARD, "Output 1 Status", DASH_STATUS_IDLE);
      Card _output1TempState = Card(&_dashboard, TEMPERATURE_CARD, "Output 1 Temperature", "°C");
      Card _output1DimmerSlider = Card(&_dashboard, SLIDER_CARD, "Output 1 Dimmer", "%", 0, 100, 1);
      Card _output1BypassSwitch = Card(&_dashboard, BUTTON_CARD, "Output 1 Bypass Relay");

      Card _output2State = Card(&_dashboard, STATUS_CARD, "Output 2 Status", DASH_STATUS_IDLE);
      Card _output2TempState = Card(&_dashboard, TEMPERATURE_CARD, "Output 2 Temperature", "°C");
      Card _output2DimmerSlider = Card(&_dashboard, SLIDER_CARD, "Output 2 Dimmer", "%", 0, 100, 1);
      Card _output2BypassSwitch = Card(&_dashboard, BUTTON_CARD, "Output 2 Bypass Relay");

      Card _relay1Switch = Card(&_dashboard, BUTTON_CARD, "Relay 1");
      Card _relay2Switch = Card(&_dashboard, BUTTON_CARD, "Relay 2");

      Card _routerEnergy = Card(&_dashboard, ENERGY_CARD, "Total Routed Energy", "kWh");

#ifdef APP_VERSION_PRO
      // health
      Card _stateBuzzer = Card(&_dashboard, STATUS_CARD, "Buzzer", DASH_STATUS_IDLE);
      Card _stateDisplay = Card(&_dashboard, STATUS_CARD, "Display", DASH_STATUS_IDLE);
      Card _stateJSY = Card(&_dashboard, STATUS_CARD, "JSY", DASH_STATUS_IDLE);
      Card _stateLEDs = Card(&_dashboard, STATUS_CARD, "LEDs", DASH_STATUS_IDLE);
      Card _stateMQTT = Card(&_dashboard, STATUS_CARD, "MQTT", DASH_STATUS_IDLE);
      Card _stateNTP = Card(&_dashboard, STATUS_CARD, "NTP", DASH_STATUS_IDLE);
      Card _stateOutput1AutoBypass = Card(&_dashboard, STATUS_CARD, "Output 1 Auto Bypass", DASH_STATUS_IDLE);
      Card _stateOutput1AutoDimmer = Card(&_dashboard, STATUS_CARD, "Output 1 Auto Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput1Bypass = Card(&_dashboard, STATUS_CARD, "Output 1 Bypass Relay", DASH_STATUS_IDLE);
      Card _stateOutput1Dimmer = Card(&_dashboard, STATUS_CARD, "Output 1 Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput1Temp = Card(&_dashboard, STATUS_CARD, "Output 1 Temperature", DASH_STATUS_IDLE);
      Card _stateOutput2AutoBypass = Card(&_dashboard, STATUS_CARD, "Output 2 Auto Bypass", DASH_STATUS_IDLE);
      Card _stateOutput2AutoDimmer = Card(&_dashboard, STATUS_CARD, "Output 2 Auto Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput2Bypass = Card(&_dashboard, STATUS_CARD, "Output 2 Bypass Relay", DASH_STATUS_IDLE);
      Card _stateOutput2Dimmer = Card(&_dashboard, STATUS_CARD, "Output 2 Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput2Temp = Card(&_dashboard, STATUS_CARD, "Output 2 Temperature", DASH_STATUS_IDLE);
      Card _statePushButton = Card(&_dashboard, STATUS_CARD, "Push Button", DASH_STATUS_IDLE);
      Card _stateRelay1 = Card(&_dashboard, STATUS_CARD, "Relay 1", DASH_STATUS_IDLE);
      Card _stateRelay2 = Card(&_dashboard, STATUS_CARD, "Relay 2", DASH_STATUS_IDLE);
      Card _stateSystemTemp = Card(&_dashboard, STATUS_CARD, "System Temperature", DASH_STATUS_IDLE);
      Card _stateZCD = Card(&_dashboard, STATUS_CARD, "ZCD", DASH_STATUS_IDLE);

      // pinout live
      Card _pinLiveBuzzer = Card(&_dashboard, STATUS_CARD, "Buzzer Pin", DASH_STATUS_IDLE);
      Card _pinLiveDisplayClock = Card(&_dashboard, STATUS_CARD, "Display Clock Pin", DASH_STATUS_IDLE);
      Card _pinLiveDisplayData = Card(&_dashboard, STATUS_CARD, "Display Data Pin", DASH_STATUS_IDLE);
      Card _pinLiveJsyRX = Card(&_dashboard, STATUS_CARD, "JSY (RX) Pin", DASH_STATUS_IDLE);
      Card _pinLiveJsyTX = Card(&_dashboard, STATUS_CARD, "JSY (TX) Pin", DASH_STATUS_IDLE);
      Card _pinLiveLEDGreen = Card(&_dashboard, STATUS_CARD, "LED Green Pin", DASH_STATUS_IDLE);
      Card _pinLiveLEDRed = Card(&_dashboard, STATUS_CARD, "LED Red Pin", DASH_STATUS_IDLE);
      Card _pinLiveLEDYellow = Card(&_dashboard, STATUS_CARD, "LED Yellow Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput1Bypass = Card(&_dashboard, STATUS_CARD, "Output 1 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput1Dimmer = Card(&_dashboard, STATUS_CARD, "Output 1 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput1Temp = Card(&_dashboard, STATUS_CARD, "Output 1 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput2Bypass = Card(&_dashboard, STATUS_CARD, "Output 2 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput2Dimmer = Card(&_dashboard, STATUS_CARD, "Output 2 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput2Temp = Card(&_dashboard, STATUS_CARD, "Output 2 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLivePushButton = Card(&_dashboard, STATUS_CARD, "Push Button Pin", DASH_STATUS_IDLE);
      Card _pinLiveRelay1 = Card(&_dashboard, STATUS_CARD, "Relay 1 Pin", DASH_STATUS_IDLE);
      Card _pinLiveRelay2 = Card(&_dashboard, STATUS_CARD, "Relay 2 Pin", DASH_STATUS_IDLE);
      Card _pinLiveSystemTemp = Card(&_dashboard, STATUS_CARD, "System Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLiveZCD = Card(&_dashboard, STATUS_CARD, "ZCD Sync Pin", DASH_STATUS_IDLE);

      // pinout config
      Card _pinConfigBuzzer = Card(&_dashboard, STATUS_CARD, "Buzzer Pin", DASH_STATUS_IDLE);
      Card _pinConfigDisplayClock = Card(&_dashboard, STATUS_CARD, "Display Clock Pin", DASH_STATUS_IDLE);
      Card _pinConfigDisplayData = Card(&_dashboard, STATUS_CARD, "Display Data Pin", DASH_STATUS_IDLE);
      Card _pinConfigJsyRX = Card(&_dashboard, STATUS_CARD, "JSY (RX) Pin", DASH_STATUS_IDLE);
      Card _pinConfigJsyTX = Card(&_dashboard, STATUS_CARD, "JSY (TX) Pin", DASH_STATUS_IDLE);
      Card _pinConfigLEDGreen = Card(&_dashboard, STATUS_CARD, "LED Green Pin", DASH_STATUS_IDLE);
      Card _pinConfigLEDRed = Card(&_dashboard, STATUS_CARD, "LED Red Pin", DASH_STATUS_IDLE);
      Card _pinConfigLEDYellow = Card(&_dashboard, STATUS_CARD, "LED Yellow Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput1Bypass = Card(&_dashboard, STATUS_CARD, "Output 1 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput1Dimmer = Card(&_dashboard, STATUS_CARD, "Output 1 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput1Temp = Card(&_dashboard, STATUS_CARD, "Output 1 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput2Bypass = Card(&_dashboard, STATUS_CARD, "Output 2 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput2Dimmer = Card(&_dashboard, STATUS_CARD, "Output 2 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput2Temp = Card(&_dashboard, STATUS_CARD, "Output 2 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinConfigPushButton = Card(&_dashboard, STATUS_CARD, "Push Button Pin", DASH_STATUS_IDLE);
      Card _pinConfigRelay1 = Card(&_dashboard, STATUS_CARD, "Relay 1 Pin", DASH_STATUS_IDLE);
      Card _pinConfigRelay2 = Card(&_dashboard, STATUS_CARD, "Relay 2 Pin", DASH_STATUS_IDLE);
      Card _pinConfigSystemTemp = Card(&_dashboard, STATUS_CARD, "System Temperature Pin", DASH_STATUS_IDLE);
      Card _pinConfigZCD = Card(&_dashboard, STATUS_CARD, "ZCD Sync Pin", DASH_STATUS_IDLE);

      // management tab
      Card _otaLink = Card(&_dashboard, LINK_CARD, "OTA Firmware Update");
      Card _consoleLink = Card(&_dashboard, LINK_CARD, "Web Console");
      Card _configBackup = Card(&_dashboard, LINK_CARD, "Configuration Backup");
      Card _configRestore = Card(&_dashboard, FILE_UPLOAD_CARD, "Configuration Restore", ".txt");
      Card _debugMode = Card(&_dashboard, BUTTON_CARD, "Debug Logging");
      Card _restart = Card(&_dashboard, PUSH_BUTTON_CARD, "Restart");
      Card _reset = Card(&_dashboard, PUSH_BUTTON_CARD, "Factory Reset");

      // display configuration
      Card _display = Card(&_dashboard, BUTTON_CARD, "Display");
      Card _displayClockPin = Card(&_dashboard, TEXT_INPUT_CARD, "Display Clock Pin");
      Card _displayDataPin = Card(&_dashboard, TEXT_INPUT_CARD, "Display Data Pin");
      Card _displayType = Card(&_dashboard, DROPDOWN_CARD, "Display Type");
      Card _displayRotation = Card(&_dashboard, DROPDOWN_CARD, "Display Rotation");
      Card _displayPowerSaveDelay = Card(&_dashboard, TEXT_INPUT_CARD, "Display Power Save Delay");

      // Electricity meter configuration
      Card _jsy = Card(&_dashboard, BUTTON_CARD, "JSY");
      Card _jsyRXPin = Card(&_dashboard, TEXT_INPUT_CARD, "JSY RX Pin (Serial TX)");
      Card _jsyTXPin = Card(&_dashboard, TEXT_INPUT_CARD, "JSY TX Pin  (Serial RX)");
      Card _jsyReset = Card(&_dashboard, PUSH_BUTTON_CARD, "JSY Reset Energy");
      Card _zcd = Card(&_dashboard, BUTTON_CARD, "ZCD");
      Card _zcdPin = Card(&_dashboard, TEXT_INPUT_CARD, "ZCD Sync Pin");
      Card _gridFreq = Card(&_dashboard, DROPDOWN_CARD, "Grid Frequency");
      Card _gridPowerMQTT = Card(&_dashboard, TEXT_INPUT_CARD, "Grid Power MQTT Topic");

      // mqtt configuration
      Card _mqtt = Card(&_dashboard, BUTTON_CARD, "MQTT");
      Card _mqttSecured = Card(&_dashboard, BUTTON_CARD, "MQTT Secured");
      Card _mqttServer = Card(&_dashboard, TEXT_INPUT_CARD, "MQTT Server");
      Card _mqttPort = Card(&_dashboard, TEXT_INPUT_CARD, "MQTT Port");
      Card _mqttUser = Card(&_dashboard, TEXT_INPUT_CARD, "MQTT User");
      Card _mqttPwd = Card(&_dashboard, PASSWORD_CARD, "MQTT Password");
      Card _mqttTopic = Card(&_dashboard, TEXT_INPUT_CARD, "MQTT Topic");
      Card _mqttPublishInterval = Card(&_dashboard, TEXT_INPUT_CARD, "MQTT Publish Interval");
      Card _haDiscovery = Card(&_dashboard, BUTTON_CARD, "Home Assistant Discovery");
      Card _haDiscoveryTopic = Card(&_dashboard, TEXT_INPUT_CARD, "Home Assistant Discovery Topic");

      // network configuration
      Card _hostname = Card(&_dashboard, TEXT_INPUT_CARD, "Hostname");
      Card _wifiSSID = Card(&_dashboard, TEXT_INPUT_CARD, "WiFi SSID");
      Card _wifiPwd = Card(&_dashboard, PASSWORD_CARD, "WiFi Password");
      Card _apMode = Card(&_dashboard, BUTTON_CARD, "AP Mode");
      Card _adminPwd = Card(&_dashboard, PASSWORD_CARD, "Admin Password");
      Card _ntpServer = Card(&_dashboard, TEXT_INPUT_CARD, "NTP Server");
      Card _ntpTimezone = Card(&_dashboard, ASYNC_DROPDOWN_CARD, "Timezone");
      Card _ntpSync = Card(&_dashboard, TIME_SYNC_CARD, "Sync time with browser");

      // output 1 configuration
      Card _output1Dimmer = Card(&_dashboard, BUTTON_CARD, "Output 1 Dimmer");
      Card _output1DimmerPin = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Dimmer Pin");
      Card _output1DimmerType = Card(&_dashboard, DROPDOWN_CARD, "Output 1 Dimmer Type");
      Card _output1AutoDimmer = Card(&_dashboard, BUTTON_CARD, "Output 1 Auto Dimmer");

      Card _output1Bypass = Card(&_dashboard, BUTTON_CARD, "Output 1 Bypass Relay");
      Card _output1BypassPin = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Bypass Relay Pin");
      Card _output1BypassType = Card(&_dashboard, DROPDOWN_CARD, "Output 1 Bypass Relay Type");
      Card _output1AutoBypass = Card(&_dashboard, BUTTON_CARD, "Output 1 Auto Bypass");

      Card _output1Temp = Card(&_dashboard, BUTTON_CARD, "Output 1 Temperature Sensor");
      Card _output1TempPin = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Temperature Sensor Pin");
      Card _output1DimmerPowerLimit = Card(&_dashboard, SLIDER_CARD, "Output 1 Dimmer Level Limit", "%", 0, 100, 1);
      Card _output1AutoStartWDays = Card(&_dashboard, WEEK_SELECTOR_CARD, "Output 1 Auto Start Days");

      Card _output1AutoStartTime = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Auto Start Time");
      Card _output1AutoStoptTime = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Auto Stop Time");
      Card _output1AutoStartTemp = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Auto Start Temperature", "°C");
      Card _output1AutoStoptTemp = Card(&_dashboard, TEXT_INPUT_CARD, "Output 1 Auto Stop Temperature", "°C");

      // output 2 configuration
      Card _output2Dimmer = Card(&_dashboard, BUTTON_CARD, "Output 2 Dimmer");
      Card _output2DimmerPin = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Dimmer Pin");
      Card _output2DimmerType = Card(&_dashboard, DROPDOWN_CARD, "Output 2 Dimmer Type");
      Card _output2AutoDimmer = Card(&_dashboard, BUTTON_CARD, "Output 2 Auto Dimmer");

      Card _output2Bypass = Card(&_dashboard, BUTTON_CARD, "Output 2 Bypass Relay");
      Card _output2BypassPin = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Bypass Relay Pin");
      Card _output2BypassType = Card(&_dashboard, DROPDOWN_CARD, "Output 2 Bypass Relay Type");
      Card _output2AutoBypass = Card(&_dashboard, BUTTON_CARD, "Output 2 Auto Bypass");

      Card _output2Temp = Card(&_dashboard, BUTTON_CARD, "Output 2 Temperature Sensor");
      Card _output2TempPin = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Temperature Sensor Pin");
      Card _output2DimmerPowerLimit = Card(&_dashboard, SLIDER_CARD, "Output 2 Dimmer Power Limit", "%", 0, 100, 1);
      Card _output2AutoStartWDays = Card(&_dashboard, WEEK_SELECTOR_CARD, "Output 2 Auto Start Days");

      Card _output2AutoStartTime = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Auto Start Time");
      Card _output2AutoStoptTime = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Auto Stop Time");
      Card _output2AutoStartTemp = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Auto Start Temperature");
      Card _output2AutoStoptTemp = Card(&_dashboard, TEXT_INPUT_CARD, "Output 2 Auto Stop Temperature");

      // relay 1 configuration
      Card _relay1 = Card(&_dashboard, BUTTON_CARD, "Relay 1");
      Card _relay1Pin = Card(&_dashboard, TEXT_INPUT_CARD, "Relay 1 Pin");
      Card _relay1Type = Card(&_dashboard, DROPDOWN_CARD, "Relay 1 Type");
      Card _relay1Power = Card(&_dashboard, SLIDER_CARD, "Relay 1 Threshold", "W", 0, 3000, 50);

      // relay 2 configuration
      Card _relay2 = Card(&_dashboard, BUTTON_CARD, "Relay 2");
      Card _relay2Pin = Card(&_dashboard, TEXT_INPUT_CARD, "Relay 2 Pin");
      Card _relay2Type = Card(&_dashboard, DROPDOWN_CARD, "Relay 2 Type");
      Card _relay2Power = Card(&_dashboard, SLIDER_CARD, "Relay 2 Threshold", "W", 0, 3000, 50);

      // system configuration
      Card _systemTemp = Card(&_dashboard, BUTTON_CARD, "System Temperature Sensor");
      Card _systemTempPin = Card(&_dashboard, TEXT_INPUT_CARD, "System Temperature Sensor Pin");
      Card _buzzer = Card(&_dashboard, BUTTON_CARD, "Buzzer");
      Card _buzzerPin = Card(&_dashboard, TEXT_INPUT_CARD, "Buzzer Pin");
      Card _led = Card(&_dashboard, BUTTON_CARD, "LED");
      Card _ledGreenPin = Card(&_dashboard, TEXT_INPUT_CARD, "LED Green Pin");
      Card _ledRedPin = Card(&_dashboard, TEXT_INPUT_CARD, "LED Red Pin");
      Card _ledYellowPin = Card(&_dashboard, TEXT_INPUT_CARD, "LED Yellow Pin");
      Card _pushButton = Card(&_dashboard, BUTTON_CARD, "Push Button");
      Card _pushButtonPin = Card(&_dashboard, TEXT_INPUT_CARD, "Push Button Pin");
      Card _pushButtonAction = Card(&_dashboard, DROPDOWN_CARD, "Push Button Action");

      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Miscellaneous_Symbols
      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Dingbats
      // tabs
      Tab _stateTab = Tab(&_dashboard, "\u2764 Health");
      Tab _pinLiveTab = Tab(&_dashboard, "\u26A1 Pinout Live");
      Tab _pinConfigTab = Tab(&_dashboard, "\u21C6 Pinout Configured");
      Tab _managementTab = Tab(&_dashboard, "\u2757 Management");
      // config tabs
      Tab _displayTab = Tab(&_dashboard, "\u2699 Display");
      Tab _electricityTab = Tab(&_dashboard, "\u2699 Electricity");
      Tab _mqttTab = Tab(&_dashboard, "\u2699 MQTT");
      Tab _networkTab = Tab(&_dashboard, "\u2699 Network");
      Tab _output1Tab = Tab(&_dashboard, "\u2699 Output 1");
      Tab _output2Tab = Tab(&_dashboard, "\u2699 Output 2");
      Tab _relay1Tab = Tab(&_dashboard, "\u2699 Relay 1");
      Tab _relay2Tab = Tab(&_dashboard, "\u2699 Relay 2");
      Tab _systemTab = Tab(&_dashboard, "\u2699 System");
#endif

    private:
      void _update(bool skipWebSocketPush);

      void _boolConfig(Card* card, const char* key);
      void _daysConfig(Card* card, const char* key);
      void _numConfig(Card* card, const char* key);
      void _passwordConfig(Card* card, const char* key);
      void _sliderConfig(Card* card, const char* key);
      void _textConfig(Card* card, const char* key);

      void _outputDimmerSlider(Card* card, int idx);
      void _outputRelaySwitch(Card* card, int idx);
      void _relaySwitch(Card* card, int idx);

      void _pinout(Card* card, int32_t pin, std::map<int32_t, Card*>* pinout);
      void _status(Card* card, const char* key, bool enabled, bool state = true, const char* err = "");
      void _temperature(Card* card, Mycila::TemperatureSensor* sensor);
  };
} // namespace YaSolR
