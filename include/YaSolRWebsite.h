// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <YaSolR.h>

#include <map>

#ifdef APP_VERSION_PRO
#define PUSH_BUTTON_CARD_CB ()
#else
#define ENERGY_CARD GENERIC_CARD
#define PUSH_BUTTON_CARD BUTTON_CARD
#define PUSH_BUTTON_CARD_CB (int32_t value)
#endif

namespace YaSolR {
  class WebsiteClass {
    public:
      void init();
      void update() { _update(false); }

    private:
      Statistic _firmName = Statistic(&dashboard, "Application");
      Statistic _firmModel = Statistic(&dashboard, "Application Model");
      Statistic _firmVersionStat = Statistic(&dashboard, "Application Version");
      Statistic _firmManufacturerStat = Statistic(&dashboard, "Application Manufacturer");

      Statistic _firmFilenameStat = Statistic(&dashboard, "Firmware Filename");
      Statistic _firmHashStat = Statistic(&dashboard, "Firmware Build Hash");
      Statistic _firmTimeStat = Statistic(&dashboard, "Firmware Build Timestamp");

      Statistic _deviceIdStat = Statistic(&dashboard, "Device Id");
      Statistic _cpuModelStat = Statistic(&dashboard, "Device CPU Model");
      Statistic _cpuCoresStat = Statistic(&dashboard, "Device CPU Cores");
      Statistic _bootCountStat = Statistic(&dashboard, "Device Boot Count");
      Statistic _heapMemoryTotalStat = Statistic(&dashboard, "Device Heap Memory Total");
      Statistic _heapMemoryUsageStat = Statistic(&dashboard, "Device Heap Memory Usage");
      Statistic _heapMemoryUsedStat = Statistic(&dashboard, "Device Heap Memory Used");

      Statistic _netModeStat = Statistic(&dashboard, "Network Mode Preferred");
      Statistic _apIPStat = Statistic(&dashboard, "Access Point IP Address");
      Statistic _apMACStat = Statistic(&dashboard, "Access Point MAC Address");
      Statistic _ethIPStat = Statistic(&dashboard, "Ethernet IP Address");
      Statistic _ethMACStat = Statistic(&dashboard, "Ethernet MAC Address");
      Statistic _wifiIPStat = Statistic(&dashboard, "WiFi IP Address");
      Statistic _wifiMACStat = Statistic(&dashboard, "WiFi MAC Address");
      Statistic _wifiSSIDStat = Statistic(&dashboard, "WiFi SSID");
      Statistic _wifiRSSIStat = Statistic(&dashboard, "WiFi RSSI");
      Statistic _wifiSignalStat = Statistic(&dashboard, "WiFi Signal");

      Statistic _gridVoltStat = Statistic(&dashboard, "Grid Voltage");
      Statistic _gridFreqStat = Statistic(&dashboard, "Grid Frequency");

      Statistic _output1SwitchCountStat = Statistic(&dashboard, "Output 1 Switch Count");
      Statistic _output2SwitchCountStat = Statistic(&dashboard, "Output 2 Switch Count");
      Statistic _relay1SwitchCountStat = Statistic(&dashboard, "Relay 1 Switch Count");
      Statistic _relay2SwitchCountStat = Statistic(&dashboard, "Relay 2 Switch Count");

      Statistic _systemTempStat = Statistic(&dashboard, "System Temperature");

      Statistic _timeStat = Statistic(&dashboard, "Time");
      Statistic _uptimeStat = Statistic(&dashboard, "Uptime");
#ifdef APP_VERSION_TRIAL
      Statistic _trialRemainingStat = Statistic(&dashboard, "Remaining Trial Time");
#endif

      // home

      Card _gridPower = Card(&dashboard, ENERGY_CARD, "Grid Power", "W");
      Card _routerPower = Card(&dashboard, ENERGY_CARD, "Routed Power", "W");
      Card _routerPowerFactor = Card(&dashboard, ENERGY_CARD, "Router Power Factor", "%");
      Card _routerTHDi = Card(&dashboard, ENERGY_CARD, "Router THDi", "%");

      Card _output1State = Card(&dashboard, STATUS_CARD, "Output 1 Status", DASH_STATUS_IDLE);
      Card _output1TempState = Card(&dashboard, TEMPERATURE_CARD, "Output 1 Temperature", "°C");
      Card _output1DimmerSlider = Card(&dashboard, SLIDER_CARD, "Output 1 Dimmer", "%", 0, 100, 1);
      Card _output1BypassSwitch = Card(&dashboard, BUTTON_CARD, "Output 1 Bypass Relay");

      Card _output2State = Card(&dashboard, STATUS_CARD, "Output 2 Status", DASH_STATUS_IDLE);
      Card _output2TempState = Card(&dashboard, TEMPERATURE_CARD, "Output 2 Temperature", "°C");
      Card _output2DimmerSlider = Card(&dashboard, SLIDER_CARD, "Output 2 Dimmer", "%", 0, 100, 1);
      Card _output2BypassSwitch = Card(&dashboard, BUTTON_CARD, "Output 2 Bypass Relay");

      Card _relay1Switch = Card(&dashboard, BUTTON_CARD, "Relay 1");
      Card _relay2Switch = Card(&dashboard, BUTTON_CARD, "Relay 2");

      Card _routerEnergy = Card(&dashboard, ENERGY_CARD, "Total Routed Energy", "kWh");

#ifdef APP_VERSION_PRO
      // health
      Card _stateBuzzer = Card(&dashboard, STATUS_CARD, "Buzzer", DASH_STATUS_IDLE);
      Card _stateDisplay = Card(&dashboard, STATUS_CARD, "Display", DASH_STATUS_IDLE);
      Card _stateGridPowerMQTT = Card(&dashboard, STATUS_CARD, "Grid Power from MQTT", DASH_STATUS_IDLE);
      Card _stateJSY = Card(&dashboard, STATUS_CARD, "JSY", DASH_STATUS_IDLE);
      Card _stateMQTT = Card(&dashboard, STATUS_CARD, "MQTT", DASH_STATUS_IDLE);
      Card _stateNTP = Card(&dashboard, STATUS_CARD, "NTP", DASH_STATUS_IDLE);
      Card _stateOutput1AutoBypass = Card(&dashboard, STATUS_CARD, "Output 1 Auto Bypass", DASH_STATUS_IDLE);
      Card _stateOutput1AutoDimmer = Card(&dashboard, STATUS_CARD, "Output 1 Auto Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput1Bypass = Card(&dashboard, STATUS_CARD, "Output 1 Bypass Relay", DASH_STATUS_IDLE);
      Card _stateOutput1Dimmer = Card(&dashboard, STATUS_CARD, "Output 1 Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput1Temp = Card(&dashboard, STATUS_CARD, "Output 1 Temperature", DASH_STATUS_IDLE);
      Card _stateOutput2AutoBypass = Card(&dashboard, STATUS_CARD, "Output 2 Auto Bypass", DASH_STATUS_IDLE);
      Card _stateOutput2AutoDimmer = Card(&dashboard, STATUS_CARD, "Output 2 Auto Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput2Bypass = Card(&dashboard, STATUS_CARD, "Output 2 Bypass Relay", DASH_STATUS_IDLE);
      Card _stateOutput2Dimmer = Card(&dashboard, STATUS_CARD, "Output 2 Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput2Temp = Card(&dashboard, STATUS_CARD, "Output 2 Temperature", DASH_STATUS_IDLE);
      Card _stateLEDs = Card(&dashboard, STATUS_CARD, "Physical LEDs", DASH_STATUS_IDLE);
      Card _statePushButton = Card(&dashboard, STATUS_CARD, "Push Button", DASH_STATUS_IDLE);
      Card _stateRelay1 = Card(&dashboard, STATUS_CARD, "Relay 1", DASH_STATUS_IDLE);
      Card _stateRelay2 = Card(&dashboard, STATUS_CARD, "Relay 2", DASH_STATUS_IDLE);
      Card _stateSystemTemp = Card(&dashboard, STATUS_CARD, "System Temperature", DASH_STATUS_IDLE);
      Card _stateZCD = Card(&dashboard, STATUS_CARD, "ZCD", DASH_STATUS_IDLE);

      // pinout live
      Card _pinLiveBuzzer = Card(&dashboard, STATUS_CARD, "Buzzer Pin", DASH_STATUS_IDLE);
      Card _pinLiveDisplayClock = Card(&dashboard, STATUS_CARD, "Display Clock Pin", DASH_STATUS_IDLE);
      Card _pinLiveDisplayData = Card(&dashboard, STATUS_CARD, "Display Data Pin", DASH_STATUS_IDLE);
      Card _pinLiveJsyRX = Card(&dashboard, STATUS_CARD, "JSY RX Pin", DASH_STATUS_IDLE);
      Card _pinLiveJsyTX = Card(&dashboard, STATUS_CARD, "JSY TX Pin", DASH_STATUS_IDLE);
      Card _pinLiveLEDGreen = Card(&dashboard, STATUS_CARD, "LED Green Pin", DASH_STATUS_IDLE);
      Card _pinLiveLEDYellow = Card(&dashboard, STATUS_CARD, "LED Yellow Pin", DASH_STATUS_IDLE);
      Card _pinLiveLEDRed = Card(&dashboard, STATUS_CARD, "LED Red Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput1Bypass = Card(&dashboard, STATUS_CARD, "Output 1 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput1Dimmer = Card(&dashboard, STATUS_CARD, "Output 1 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput1Temp = Card(&dashboard, STATUS_CARD, "Output 1 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput2Bypass = Card(&dashboard, STATUS_CARD, "Output 2 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput2Dimmer = Card(&dashboard, STATUS_CARD, "Output 2 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinLiveOutput2Temp = Card(&dashboard, STATUS_CARD, "Output 2 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLivePushButton = Card(&dashboard, STATUS_CARD, "Push Button Pin", DASH_STATUS_IDLE);
      Card _pinLiveRelay1 = Card(&dashboard, STATUS_CARD, "Relay 1 Pin", DASH_STATUS_IDLE);
      Card _pinLiveRelay2 = Card(&dashboard, STATUS_CARD, "Relay 2 Pin", DASH_STATUS_IDLE);
      Card _pinLiveSystemTemp = Card(&dashboard, STATUS_CARD, "System Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLiveZCD = Card(&dashboard, STATUS_CARD, "ZCD Sync Pin", DASH_STATUS_IDLE);

      // pinout config
      Card _pinConfigBuzzer = Card(&dashboard, STATUS_CARD, "Buzzer Pin", DASH_STATUS_IDLE);
      Card _pinConfigDisplayClock = Card(&dashboard, STATUS_CARD, "Display Clock Pin", DASH_STATUS_IDLE);
      Card _pinConfigDisplayData = Card(&dashboard, STATUS_CARD, "Display Data Pin", DASH_STATUS_IDLE);
      Card _pinConfigJsyRX = Card(&dashboard, STATUS_CARD, "JSY RX Pin", DASH_STATUS_IDLE);
      Card _pinConfigJsyTX = Card(&dashboard, STATUS_CARD, "JSY TX Pin", DASH_STATUS_IDLE);
      Card _pinConfigLEDGreen = Card(&dashboard, STATUS_CARD, "LED Green Pin", DASH_STATUS_IDLE);
      Card _pinConfigLEDYellow = Card(&dashboard, STATUS_CARD, "LED Yellow Pin", DASH_STATUS_IDLE);
      Card _pinConfigLEDRed = Card(&dashboard, STATUS_CARD, "LED Red Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput1Bypass = Card(&dashboard, STATUS_CARD, "Output 1 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput1Dimmer = Card(&dashboard, STATUS_CARD, "Output 1 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput1Temp = Card(&dashboard, STATUS_CARD, "Output 1 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput2Bypass = Card(&dashboard, STATUS_CARD, "Output 2 Bypass Relay Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput2Dimmer = Card(&dashboard, STATUS_CARD, "Output 2 Dimmer Pin", DASH_STATUS_IDLE);
      Card _pinConfigOutput2Temp = Card(&dashboard, STATUS_CARD, "Output 2 Temperature Pin", DASH_STATUS_IDLE);
      Card _pinConfigPushButton = Card(&dashboard, STATUS_CARD, "Push Button Pin", DASH_STATUS_IDLE);
      Card _pinConfigRelay1 = Card(&dashboard, STATUS_CARD, "Relay 1 Pin", DASH_STATUS_IDLE);
      Card _pinConfigRelay2 = Card(&dashboard, STATUS_CARD, "Relay 2 Pin", DASH_STATUS_IDLE);
      Card _pinConfigSystemTemp = Card(&dashboard, STATUS_CARD, "System Temperature Pin", DASH_STATUS_IDLE);
      Card _pinConfigZCD = Card(&dashboard, STATUS_CARD, "ZCD Sync Pin", DASH_STATUS_IDLE);

      // management tab
      Card _otaLink = Card(&dashboard, LINK_CARD, "OTA Firmware Update");
      Card _consoleLink = Card(&dashboard, LINK_CARD, "Web Console");
      Card _configBackup = Card(&dashboard, LINK_CARD, "Configuration Backup");
      Card _configRestore = Card(&dashboard, FILE_UPLOAD_CARD, "Configuration Restore", ".txt");
      Card _debugMode = Card(&dashboard, BUTTON_CARD, "Debug Logging");
      Card _restart = Card(&dashboard, PUSH_BUTTON_CARD, "Restart");
      Card _reset = Card(&dashboard, PUSH_BUTTON_CARD, "Factory Reset");

      // display configuration
      Card _display = Card(&dashboard, BUTTON_CARD, "Display");
      Card _displayClockPin = Card(&dashboard, TEXT_INPUT_CARD, "Display Clock Pin");
      Card _displayDataPin = Card(&dashboard, TEXT_INPUT_CARD, "Display Data Pin");
      Card _displayType = Card(&dashboard, DROPDOWN_CARD, "Display Type");
      Card _displayRotation = Card(&dashboard, DROPDOWN_CARD, "Display Rotation");
      Card _displayPowerSaveDelay = Card(&dashboard, TEXT_INPUT_CARD, "Display Power Save Delay");

      // Electricity meter configuration
      Card _jsy = Card(&dashboard, BUTTON_CARD, "JSY");
      Card _jsyRXPin = Card(&dashboard, TEXT_INPUT_CARD, "JSY RX Pin (Serial TX)");
      Card _jsyTXPin = Card(&dashboard, TEXT_INPUT_CARD, "JSY TX Pin  (Serial RX)");
      Card _jsyReset = Card(&dashboard, PUSH_BUTTON_CARD, "JSY Reset Energy");
      Card _zcd = Card(&dashboard, BUTTON_CARD, "ZCD");
      Card _zcdPin = Card(&dashboard, TEXT_INPUT_CARD, "ZCD Sync Pin");
      Card _gridFreq = Card(&dashboard, DROPDOWN_CARD, "Grid Frequency");
      Card _gridPowerMQTT = Card(&dashboard, TEXT_INPUT_CARD, "Grid Power MQTT Topic");

      // mqtt configuration
      Card _mqtt = Card(&dashboard, BUTTON_CARD, "MQTT");
      Card _mqttTopic = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Topic");
      Card _haDiscovery = Card(&dashboard, BUTTON_CARD, "Home Assistant Discovery");
      Card _haDiscoveryTopic = Card(&dashboard, TEXT_INPUT_CARD, "Home Assistant Discovery Topic");
      Card _mqttServer = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Server");
      Card _mqttPort = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Port");
      Card _mqttServerCert = Card(&dashboard, FILE_UPLOAD_CARD, "MQTT Server Certificate", ".pem,crt,der");
      Card _mqttSecured = Card(&dashboard, BUTTON_CARD, "MQTT Secured");
      Card _mqttUser = Card(&dashboard, TEXT_INPUT_CARD, "MQTT User");
      Card _mqttPwd = Card(&dashboard, PASSWORD_CARD, "MQTT Password");
      Card _mqttPublishInterval = Card(&dashboard, SLIDER_CARD, "MQTT Publish Interval", "s", 5, 30, 1);

      // network configuration
      Card _hostname = Card(&dashboard, TEXT_INPUT_CARD, "Hostname");
      Card _wifiSSID = Card(&dashboard, TEXT_INPUT_CARD, "WiFi SSID");
      Card _wifiPwd = Card(&dashboard, PASSWORD_CARD, "WiFi Password");
      Card _apMode = Card(&dashboard, BUTTON_CARD, "AP Mode");
      Card _adminPwd = Card(&dashboard, PASSWORD_CARD, "Admin Password");
      Card _ntpServer = Card(&dashboard, TEXT_INPUT_CARD, "NTP Server");
      Card _ntpTimezone = Card(&dashboard, ASYNC_DROPDOWN_CARD, "Timezone");
      Card _ntpSync = Card(&dashboard, TIME_SYNC_CARD, "Sync time with browser");

      // output 1 configuration
      Card _output1Dimmer = Card(&dashboard, BUTTON_CARD, "Output 1 Dimmer");
      Card _output1DimmerPin = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Dimmer Pin");
      Card _output1DimmerType = Card(&dashboard, DROPDOWN_CARD, "Output 1 Dimmer Type");
      Card _output1AutoDimmer = Card(&dashboard, BUTTON_CARD, "Output 1 Auto Dimmer");

      Card _output1Bypass = Card(&dashboard, BUTTON_CARD, "Output 1 Bypass Relay");
      Card _output1BypassPin = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Bypass Relay Pin");
      Card _output1BypassType = Card(&dashboard, DROPDOWN_CARD, "Output 1 Bypass Relay Type");
      Card _output1AutoBypass = Card(&dashboard, BUTTON_CARD, "Output 1 Auto Bypass");

      Card _output1Temp = Card(&dashboard, BUTTON_CARD, "Output 1 Temperature Sensor");
      Card _output1TempPin = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Temperature Sensor Pin");
      Card _output1DimmerPowerLimit = Card(&dashboard, SLIDER_CARD, "Output 1 Dimmer Level Limit", "%", 0, 100, 1);
      Card _output1AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, "Output 1 Auto Start Days");

      Card _output1AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Auto Start Time");
      Card _output1AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Auto Stop Time");
      Card _output1AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Auto Start Temperature", "°C");
      Card _output1AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, "Output 1 Auto Stop Temperature", "°C");

      // output 2 configuration
      Card _output2Dimmer = Card(&dashboard, BUTTON_CARD, "Output 2 Dimmer");
      Card _output2DimmerPin = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Dimmer Pin");
      Card _output2DimmerType = Card(&dashboard, DROPDOWN_CARD, "Output 2 Dimmer Type");
      Card _output2AutoDimmer = Card(&dashboard, BUTTON_CARD, "Output 2 Auto Dimmer");

      Card _output2Bypass = Card(&dashboard, BUTTON_CARD, "Output 2 Bypass Relay");
      Card _output2BypassPin = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Bypass Relay Pin");
      Card _output2BypassType = Card(&dashboard, DROPDOWN_CARD, "Output 2 Bypass Relay Type");
      Card _output2AutoBypass = Card(&dashboard, BUTTON_CARD, "Output 2 Auto Bypass");

      Card _output2Temp = Card(&dashboard, BUTTON_CARD, "Output 2 Temperature Sensor");
      Card _output2TempPin = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Temperature Sensor Pin");
      Card _output2DimmerPowerLimit = Card(&dashboard, SLIDER_CARD, "Output 2 Dimmer Power Limit", "%", 0, 100, 1);
      Card _output2AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, "Output 2 Auto Start Days");

      Card _output2AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Auto Start Time");
      Card _output2AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Auto Stop Time");
      Card _output2AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Auto Start Temperature");
      Card _output2AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, "Output 2 Auto Stop Temperature");

      // relay 1 configuration
      Card _relay1 = Card(&dashboard, BUTTON_CARD, "Relay 1");
      Card _relay1Pin = Card(&dashboard, TEXT_INPUT_CARD, "Relay 1 Pin");
      Card _relay1Type = Card(&dashboard, DROPDOWN_CARD, "Relay 1 Type");
      Card _relay1Power = Card(&dashboard, SLIDER_CARD, "Relay 1 Threshold", "W", 0, 3000, 50);

      // relay 2 configuration
      Card _relay2 = Card(&dashboard, BUTTON_CARD, "Relay 2");
      Card _relay2Pin = Card(&dashboard, TEXT_INPUT_CARD, "Relay 2 Pin");
      Card _relay2Type = Card(&dashboard, DROPDOWN_CARD, "Relay 2 Type");
      Card _relay2Power = Card(&dashboard, SLIDER_CARD, "Relay 2 Threshold", "W", 0, 3000, 50);

      // system configuration
      Card _systemTemp = Card(&dashboard, BUTTON_CARD, "System Temperature Sensor");
      Card _systemTempPin = Card(&dashboard, TEXT_INPUT_CARD, "System Temperature Sensor Pin");
      Card _buzzer = Card(&dashboard, BUTTON_CARD, "Buzzer");
      Card _buzzerPin = Card(&dashboard, TEXT_INPUT_CARD, "Buzzer Pin");
      Card _led = Card(&dashboard, BUTTON_CARD, "LED");
      Card _ledGreenPin = Card(&dashboard, TEXT_INPUT_CARD, "LED Green Pin");
      Card _ledYellowPin = Card(&dashboard, TEXT_INPUT_CARD, "LED Yellow Pin");
      Card _ledRedPin = Card(&dashboard, TEXT_INPUT_CARD, "LED Red Pin");
      Card _pushButton = Card(&dashboard, BUTTON_CARD, "Push Button");
      Card _pushButtonPin = Card(&dashboard, TEXT_INPUT_CARD, "Push Button Pin");
      Card _pushButtonAction = Card(&dashboard, DROPDOWN_CARD, "Push Button Action");

      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Miscellaneous_Symbols
      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Dingbats
      // tabs
      Tab _stateTab = Tab(&dashboard, "\u2764 Health");
      Tab _pinLiveTab = Tab(&dashboard, "\u26A1 Pinout Live");
      Tab _pinConfigTab = Tab(&dashboard, "\u21C6 Pinout Configured");
      Tab _managementTab = Tab(&dashboard, "\u2757 Management");
      // config tabs
      Tab _displayTab = Tab(&dashboard, "\u2699 Display");
      Tab _electricityTab = Tab(&dashboard, "\u2699 Electricity");
      Tab _mqttTab = Tab(&dashboard, "\u2699 MQTT");
      Tab _networkTab = Tab(&dashboard, "\u2699 Network");
      Tab _output1Tab = Tab(&dashboard, "\u2699 Output 1");
      Tab _output2Tab = Tab(&dashboard, "\u2699 Output 2");
      Tab _relay1Tab = Tab(&dashboard, "\u2699 Relay 1");
      Tab _relay2Tab = Tab(&dashboard, "\u2699 Relay 2");
      Tab _systemTab = Tab(&dashboard, "\u2699 System");
#endif

    private:
      void _update(bool skipWebSocketPush);

      void _boolConfig(Card* card, const char* key);
      void _daysConfig(Card* card, const char* key);
      void _numConfig(Card* card, const char* key);
      void _passwordConfig(Card* card, const char* key);
      void _sliderConfig(Card* card, const char* key);
      void _textConfig(Card* card, const char* key);

      void _outputDimmerSlider(Card* card, Mycila::RouterOutput* output);
      void _outputRelaySwitch(Card* card, Mycila::RouterOutput* output);
      void _relaySwitch(Card* card, Mycila::Relay* relay);

      void _pinout(Card* card, int32_t pin, std::map<int32_t, Card*>* pinout);
      void _status(Card* card, const char* key, bool enabled, bool state = true, const char* err = "");
      void _temperature(Card* card, Mycila::TemperatureSensor* sensor);
  };

  extern WebsiteClass Website;
} // namespace YaSolR
