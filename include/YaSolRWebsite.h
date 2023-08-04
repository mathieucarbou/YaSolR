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

      Statistic _timeStat = Statistic(&dashboard, "Time");
      Statistic _uptimeStat = Statistic(&dashboard, "Uptime");
#ifdef APP_VERSION_TRIAL
      Statistic _trialRemainingStat = Statistic(&dashboard, "Remaining Trial Time");
#endif

      // home

      Card _relay1Switch = Card(&dashboard, BUTTON_CARD, "Relay 1");
      Card _relay2Switch = Card(&dashboard, BUTTON_CARD, "Relay 2");
      Card _gridPower = Card(&dashboard, ENERGY_CARD, "Grid Power", "W");
      Card _systemTempState = Card(&dashboard, TEMPERATURE_CARD, "System Temperature", "°C");

      Card _output1BypassSwitch = Card(&dashboard, BUTTON_CARD, "Output 1 Bypass Relay");
      Card _output1DimmerSlider = Card(&dashboard, SLIDER_CARD, "Output 1 Dimmer", "", 0, MYCILA_DIMMER_MAX_LEVEL, 1);
      Card _output1State = Card(&dashboard, STATUS_CARD, "Output 1 Status", DASH_STATUS_IDLE);
      Card _output1TempState = Card(&dashboard, TEMPERATURE_CARD, "Output 1 Temperature", "°C");

      Card _output2BypassSwitch = Card(&dashboard, BUTTON_CARD, "Output 2 Bypass Relay");
      Card _output2DimmerSlider = Card(&dashboard, SLIDER_CARD, "Output 2 Dimmer", "", 0, MYCILA_DIMMER_MAX_LEVEL, 1);
      Card _output2State = Card(&dashboard, STATUS_CARD, "Output 2 Status", DASH_STATUS_IDLE);
      Card _output2TempState = Card(&dashboard, TEMPERATURE_CARD, "Output 2 Temperature", "°C");

      Card _routerPower = Card(&dashboard, ENERGY_CARD, "Total Routed Power", "W");
      Card _routerPowerFactor = Card(&dashboard, ENERGY_CARD, "Router Power Factor");
      Card _routerTHDi = Card(&dashboard, ENERGY_CARD, "Router THDi", "%");
      Card _routerEnergy = Card(&dashboard, ENERGY_CARD, "Total Routed Energy", "kWh");

      Card _output1Power = Card(&dashboard, ENERGY_CARD, "Output 1 Power", "W");
      Card _output1PowerFactor = Card(&dashboard, ENERGY_CARD, "Output 1 Power Factor");
      Card _output1THDi = Card(&dashboard, ENERGY_CARD, "Output 1 THDi", "%");
      Card _output1Energy = Card(&dashboard, ENERGY_CARD, "Output 1 Energy", "kWh");

      Card _output2Power = Card(&dashboard, ENERGY_CARD, "Output 2 Power", "W");
      Card _output2PowerFactor = Card(&dashboard, ENERGY_CARD, "Output 2 Power Factor");
      Card _output2THDi = Card(&dashboard, ENERGY_CARD, "Output 2 THDi", "%");
      Card _output2Energy = Card(&dashboard, ENERGY_CARD, "Output 2 Energy", "kWh");

      Card _output1ApparentPower = Card(&dashboard, ENERGY_CARD, "Output 1 Apparent Power", "VA");
      Card _output1Voltage = Card(&dashboard, ENERGY_CARD, "Output 1 Voltage", "V");
      Card _output1Current = Card(&dashboard, ENERGY_CARD, "Output 1 Current", "A");
      Card _output1Resistance = Card(&dashboard, ENERGY_CARD, "Output 1 Resistance", "Ω");

      Card _output2ApparentPower = Card(&dashboard, ENERGY_CARD, "Output 2 Apparent Power", "VA");
      Card _output2Voltage = Card(&dashboard, ENERGY_CARD, "Output 2 Voltage", "V");
      Card _output2Current = Card(&dashboard, ENERGY_CARD, "Output 2 Current", "A");
      Card _output2Resistance = Card(&dashboard, ENERGY_CARD, "Output 2 Resistance", "Ω");

#ifdef APP_VERSION_PRO
      // health
      Card _stateBuzzer = Card(&dashboard, STATUS_CARD, "Buzzer", DASH_STATUS_IDLE);
      Card _stateDisplay = Card(&dashboard, STATUS_CARD, "Display", DASH_STATUS_IDLE);
      Card _stateGridDataMQTT = Card(&dashboard, STATUS_CARD, "Grid data from MQTT", DASH_STATUS_IDLE);
      Card _stateJSY = Card(&dashboard, STATUS_CARD, "JSY", DASH_STATUS_IDLE);
      Card _stateMQTT = Card(&dashboard, STATUS_CARD, "MQTT", DASH_STATUS_IDLE);
      Card _stateNTP = Card(&dashboard, STATUS_CARD, "NTP", DASH_STATUS_IDLE);
      Card _stateOutput1AutoBypass = Card(&dashboard, STATUS_CARD, "Output 1 Auto Bypass", DASH_STATUS_IDLE);
      Card _stateOutput1AutoDimmer = Card(&dashboard, STATUS_CARD, "Output 1 Auto Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput1Bypass = Card(&dashboard, STATUS_CARD, "Output 1 Bypass Relay", DASH_STATUS_IDLE);
      Card _stateOutput1Dimmer = Card(&dashboard, STATUS_CARD, "Output 1 Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput1PZEM = Card(&dashboard, STATUS_CARD, "Output 1 PZEM", DASH_STATUS_IDLE);
      Card _stateOutput1Temp = Card(&dashboard, STATUS_CARD, "Output 1 Temperature", DASH_STATUS_IDLE);
      Card _stateOutput2AutoBypass = Card(&dashboard, STATUS_CARD, "Output 2 Auto Bypass", DASH_STATUS_IDLE);
      Card _stateOutput2AutoDimmer = Card(&dashboard, STATUS_CARD, "Output 2 Auto Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput2Bypass = Card(&dashboard, STATUS_CARD, "Output 2 Bypass Relay", DASH_STATUS_IDLE);
      Card _stateOutput2Dimmer = Card(&dashboard, STATUS_CARD, "Output 2 Dimmer", DASH_STATUS_IDLE);
      Card _stateOutput2PZEM = Card(&dashboard, STATUS_CARD, "Output 2 PZEM", DASH_STATUS_IDLE);
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
      Card _pinLiveJsyRX = Card(&dashboard, STATUS_CARD, "JSY RX Pin (Serial TX)", DASH_STATUS_IDLE);
      Card _pinLiveJsyTX = Card(&dashboard, STATUS_CARD, "JSY TX Pin (Serial RX)", DASH_STATUS_IDLE);
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
      Card _pinLivePZEMRX = Card(&dashboard, STATUS_CARD, "PZEM RX Pin (Serial TX)", DASH_STATUS_IDLE);
      Card _pinLivePZEMTX = Card(&dashboard, STATUS_CARD, "PZEM TX Pin (Serial RX)", DASH_STATUS_IDLE);
      Card _pinLiveRelay1 = Card(&dashboard, STATUS_CARD, "Relay 1 Pin", DASH_STATUS_IDLE);
      Card _pinLiveRelay2 = Card(&dashboard, STATUS_CARD, "Relay 2 Pin", DASH_STATUS_IDLE);
      Card _pinLiveSystemTemp = Card(&dashboard, STATUS_CARD, "System Temperature Pin", DASH_STATUS_IDLE);
      Card _pinLiveZCD = Card(&dashboard, STATUS_CARD, "ZCD Sync Pin", DASH_STATUS_IDLE);

      // pinout config
      Card _pinConfigBuzzer = Card(&dashboard, STATUS_CARD, "Buzzer Pin", DASH_STATUS_IDLE);
      Card _pinConfigDisplayClock = Card(&dashboard, STATUS_CARD, "Display Clock Pin", DASH_STATUS_IDLE);
      Card _pinConfigDisplayData = Card(&dashboard, STATUS_CARD, "Display Data Pin", DASH_STATUS_IDLE);
      Card _pinConfigJsyRX = Card(&dashboard, STATUS_CARD, "JSY RX Pin (Serial TX)", DASH_STATUS_IDLE);
      Card _pinConfigJsyTX = Card(&dashboard, STATUS_CARD, "JSY TX Pin (Serial RX)", DASH_STATUS_IDLE);
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
      Card _pinConfigPZEMRX = Card(&dashboard, STATUS_CARD, "PZEM RX Pin (Serial TX)", DASH_STATUS_IDLE);
      Card _pinConfigPZEMTX = Card(&dashboard, STATUS_CARD, "PZEM TX Pin (Serial RX)", DASH_STATUS_IDLE);
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
      Card _jsyTXPin = Card(&dashboard, TEXT_INPUT_CARD, "JSY TX Pin (Serial RX)");
      Card _jsyReset = Card(&dashboard, PUSH_BUTTON_CARD, "JSY Reset Energy");
      Card _zcd = Card(&dashboard, BUTTON_CARD, "ZCD");
      Card _zcdPin = Card(&dashboard, TEXT_INPUT_CARD, "ZCD Sync Pin");
      Card _gridFreq = Card(&dashboard, DROPDOWN_CARD, "Grid Frequency");
      Card _gridVoltageMQTT = Card(&dashboard, TEXT_INPUT_CARD, "Grid Voltage MQTT Topic");
      Card _gridPowerMQTT = Card(&dashboard, TEXT_INPUT_CARD, "Grid Power MQTT Topic");

      // mqtt configuration
      Card _mqtt = Card(&dashboard, BUTTON_CARD, "MQTT");
      Card _mqttTopic = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Topic");
      Card _haDiscovery = Card(&dashboard, BUTTON_CARD, "Home Assistant Discovery");
      Card _haDiscoveryTopic = Card(&dashboard, TEXT_INPUT_CARD, "Home Assistant Discovery Topic");
      Card _mqttServer = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Server");
      Card _mqttPort = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Port");
      Card _mqttSecured = Card(&dashboard, BUTTON_CARD, "MQTT Secured");
      Card _mqttServerCert = Card(&dashboard, FILE_UPLOAD_CARD, "MQTT Server Certificate", ".pem,crt,der");
      Card _mqttUser = Card(&dashboard, TEXT_INPUT_CARD, "MQTT Username");
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
      Card _output1Dimmer = Card(&dashboard, BUTTON_CARD, "Dimmer");
      Card _output1DimmerPin = Card(&dashboard, TEXT_INPUT_CARD, "Dimmer Pin");
      Card _output1DimmerType = Card(&dashboard, DROPDOWN_CARD, "Dimmer Type");
      Card _output1DimmerPowerLimit = Card(&dashboard, SLIDER_CARD, "Dimmer Level Limit", "", 0, MYCILA_DIMMER_MAX_LEVEL, 1);
      Card _output1AutoDimmer = Card(&dashboard, BUTTON_CARD, "Auto Dimmer");

      Card _output1Bypass = Card(&dashboard, BUTTON_CARD, "Bypass Relay");
      Card _output1BypassPin = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Relay Pin");
      Card _output1BypassType = Card(&dashboard, DROPDOWN_CARD, "Bypass Relay Type");

      Card _output1Temp = Card(&dashboard, BUTTON_CARD, "Temperature Sensor");
      Card _output1TempPin = Card(&dashboard, TEXT_INPUT_CARD, "Temperature Sensor Pin");
      Card _output1AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, "Auto Start Temperature", "°C");
      Card _output1AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, "Auto Stop Temperature", "°C");

      Card _output1AutoBypass = Card(&dashboard, BUTTON_CARD, "Auto Bypass");
      Card _output1AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, "Auto Start Days");
      Card _output1AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, "Auto Start Time");
      Card _output1AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, "Auto Stop Time");

      Card _output1PZEM = Card(&dashboard, BUTTON_CARD, "PZEM-004T v3");
      Card _output1PZEMRXPin = Card(&dashboard, TEXT_INPUT_CARD, "PZEM RX Pin (Serial TX)");
      Card _output1PZEMTXPin = Card(&dashboard, TEXT_INPUT_CARD, "PZEM TX Pin (Serial RX)");
      Card _output1PZEMSync = Card(&dashboard, PUSH_BUTTON_CARD, "PZEM Pairing");
      Card _output1PZEMReset = Card(&dashboard, PUSH_BUTTON_CARD, "PZEM Reset Energy");

      // output 2 configuration
      Card _output2Dimmer = Card(&dashboard, BUTTON_CARD, "Dimmer");
      Card _output2DimmerPin = Card(&dashboard, TEXT_INPUT_CARD, "Dimmer Pin");
      Card _output2DimmerType = Card(&dashboard, DROPDOWN_CARD, "Dimmer Type");
      Card _output2DimmerPowerLimit = Card(&dashboard, SLIDER_CARD, "Dimmer Power Limit", "", 0, MYCILA_DIMMER_MAX_LEVEL, 1);
      Card _output2AutoDimmer = Card(&dashboard, BUTTON_CARD, "Auto Dimmer");

      Card _output2Bypass = Card(&dashboard, BUTTON_CARD, "Bypass Relay");
      Card _output2BypassPin = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Relay Pin");
      Card _output2BypassType = Card(&dashboard, DROPDOWN_CARD, "Bypass Relay Type");

      Card _output2Temp = Card(&dashboard, BUTTON_CARD, "Temperature Sensor");
      Card _output2TempPin = Card(&dashboard, TEXT_INPUT_CARD, "Temperature Sensor Pin");
      Card _output2AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, "Auto Start Temperature");
      Card _output2AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, "Auto Stop Temperature");

      Card _output2AutoBypass = Card(&dashboard, BUTTON_CARD, "Auto Bypass");
      Card _output2AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, "Auto Start Days");
      Card _output2AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, "Auto Start Time");
      Card _output2AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, "Auto Stop Time");

      Card _output2PZEM = Card(&dashboard, BUTTON_CARD, "PZEM-004T v3");
      Card _output2PZEMRXPin = Card(&dashboard, TEXT_INPUT_CARD, "PZEM RX Pin (Serial TX)");
      Card _output2PZEMTXPin = Card(&dashboard, TEXT_INPUT_CARD, "PZEM TX Pin (Serial RX)");
      Card _output2PZEMSync = Card(&dashboard, PUSH_BUTTON_CARD, "PZEM Pairing");
      Card _output2PZEMReset = Card(&dashboard, PUSH_BUTTON_CARD, "PZEM Reset Energy");

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
      Tab _electricityTab = Tab(&dashboard, "\u2699 Grid Electricity");
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
      void _relaySwitch(Card* card, const char* relayName);

      void _pinout(Card* card, int32_t pin, std::map<int32_t, Card*>* pinout);
      void _status(Card* card, const char* key, bool enabled, bool state = true, const char* err = "");
      void _temperature(Card* card, Mycila::TemperatureSensor* sensor);
  };

  extern WebsiteClass Website;
} // namespace YaSolR
