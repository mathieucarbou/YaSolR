// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <YaSolR.h>

#include <map>

#ifdef APP_MODEL_PRO
  #define PUSH_BUTTON_CARD_CB ()
#else
  #define ENERGY_CARD         GENERIC_CARD
  #define PUSH_BUTTON_CARD    BUTTON_CARD
  #define PUSH_BUTTON_CARD_CB (int32_t value)
#endif

namespace YaSolR {
  class WebsiteClass {
    public:
      void initLayout();
      void initCards();
      void updateCards();

    private:
      // statistics
      Statistic _appName = Statistic(&dashboard, "Application: Name");
      Statistic _appModel = Statistic(&dashboard, "Application: Model");
      Statistic _appVersion = Statistic(&dashboard, "Application: Version");
      Statistic _appManufacturer = Statistic(&dashboard, "Application: Manufacturer");

      Statistic _deviceBootCount = Statistic(&dashboard, "Device: Boot Count");
      Statistic _deviceCores = Statistic(&dashboard, "Device: Cores");
      Statistic _deviceHeapTotal = Statistic(&dashboard, "Device: Heap Memory Total");
      Statistic _deviceHeapUsage = Statistic(&dashboard, "Device: Heap Memory Usage");
      Statistic _deviceHeapUsed = Statistic(&dashboard, "Device: Heap Memory Used");
      Statistic _deviceID = Statistic(&dashboard, "Device: ID");
      Statistic _deviceModel = Statistic(&dashboard, "Device: Model");
      Statistic _deviceRev = Statistic(&dashboard, "Device: Revision");

      Statistic _firmwareBuildHash = Statistic(&dashboard, "Firmware: Build Hash");
      Statistic _firmwareBuildTimestamp = Statistic(&dashboard, "Firmware: Build Timestamp");
      Statistic _firmwareFilename = Statistic(&dashboard, "Firmware: Filename");

      Statistic _gridEnergy = Statistic(&dashboard, "Grid: Energy");
      Statistic _gridEnergyReturned = Statistic(&dashboard, "Grid: Energy Returned");
      Statistic _gridFrequency = Statistic(&dashboard, "Grid: Frequency");
      Statistic _gridVoltage = Statistic(&dashboard, "Grid: Voltage");

      Statistic _networkHostname = Statistic(&dashboard, "Network: Hostname");
      Statistic _networkInterface = Statistic(&dashboard, "Network: Interface");
      Statistic _networkAPIP = Statistic(&dashboard, "Network: Access Point IP Address");
      Statistic _networkAPMAC = Statistic(&dashboard, "Network: Access Point MAC Address");
      Statistic _networkEthIP = Statistic(&dashboard, "Network: Ethernet IP Address");
      Statistic _networkEthMAC = Statistic(&dashboard, "Network: Ethernet MAC Address");
      Statistic _networkWiFiIP = Statistic(&dashboard, "Network: WiFi IP Address");
      Statistic _networkWiFiMAC = Statistic(&dashboard, "Network: WiFi MAC Address");
      Statistic _networkWiFiSSID = Statistic(&dashboard, "Network: WiFi SSID");
      Statistic _networkWiFiRSSI = Statistic(&dashboard, "Network: WiFi RSSI");
      Statistic _networkWiFiSignal = Statistic(&dashboard, "Network: WiFi Signal");

      Statistic _output1RelaySwitchCount = Statistic(&dashboard, "Relay: Output 1 Bypass Switch Count");
      Statistic _output2RelaySwitchCount = Statistic(&dashboard, "Relay: Output 2 Bypass Switch Count");
      Statistic _relay1SwitchCount = Statistic(&dashboard, "Relay: Relay 1 Switch Count");
      Statistic _relay2SwitchCount = Statistic(&dashboard, "Relay: Relay 2 Switch Count");

      Statistic _time = Statistic(&dashboard, "Time");
      Statistic _uptime = Statistic(&dashboard, "Uptime");
#ifdef APP_MODEL_TRIAL
      Statistic _trialRemainingTime = Statistic(&dashboard, "Trial Remaining Time");
#endif

      // home
      Card _routerEnergy = Card(&dashboard, ENERGY_CARD, "Total Routed Energy", "kWh");
      Card _routerPower = Card(&dashboard, ENERGY_CARD, "Total Routed Power", "W");
      Card _routerPowerFactor = Card(&dashboard, ENERGY_CARD, "Router Power Factor");
      Card _routerTHDi = Card(&dashboard, ENERGY_CARD, "Router THDi", "%");
      Card _gridPower = Card(&dashboard, ENERGY_CARD, "Grid Power", "W");
      Card _routerDS18State = Card(&dashboard, TEMPERATURE_CARD, "System Temperature", "°C");

#ifdef APP_MODEL_PRO
      // tabs icons:
      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Miscellaneous_Symbols
      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Dingbats

      Tab _output1Tab = Tab(&dashboard, "\u2600 Output 1");
      Card _output1State = Card(&dashboard, STATUS_CARD, "Status", DASH_STATUS_IDLE);
      Card _output1DS18State = Card(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
      Card _output1DimmerSlider = Card(&dashboard, SLIDER_CARD, "Dimmer Level Manual Control", "", 0, YASOLR_DIMMER_MAX_LEVEL, 1);
      Card _output1DimmerSliderRO = Card(&dashboard, PROGRESS_CARD, "Dimmer Level", "", 0, YASOLR_DIMMER_MAX_LEVEL, 1);
      Card _output1Bypass = Card(&dashboard, BUTTON_CARD, "Bypass Manual Control");
      Card _output1BypassRO = Card(&dashboard, STATUS_CARD, "Bypass");
      Card _output1Power = Card(&dashboard, ENERGY_CARD, "Power", "W");
      Card _output1ApparentPower = Card(&dashboard, ENERGY_CARD, "Apparent Power", "VA");
      Card _output1PowerFactor = Card(&dashboard, ENERGY_CARD, "Power Factor");
      Card _output1THDi = Card(&dashboard, ENERGY_CARD, "THDi", "%");
      Card _output1Voltage = Card(&dashboard, ENERGY_CARD, "Voltage", "V");
      Card _output1Current = Card(&dashboard, ENERGY_CARD, "Current", "A");
      Card _output1Resistance = Card(&dashboard, ENERGY_CARD, "Resistance", "Ω");
      Card _output1Energy = Card(&dashboard, ENERGY_CARD, "Energy", "kWh");
      Card _output1DimmerAuto = Card(&dashboard, BUTTON_CARD, "Dimmer Automatic Control");
      Card _output1DimmerLimiter = Card(&dashboard, SLIDER_CARD, "Dimmer Limiter", "", 0, YASOLR_DIMMER_MAX_LEVEL, 1);
      Card _output1AutoBypass = Card(&dashboard, BUTTON_CARD, "Bypass Automatic Control");
      Card _output1AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, "Bypass Week Days");
      Card _output1AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Start Time");
      Card _output1AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Stop Time");
      Card _output1AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Start Temperature", "°C");
      Card _output1AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Stop Temperature", "°C");

      Tab _output2Tab = Tab(&dashboard, "\u2600 Output 2");
      Card _output2State = Card(&dashboard, STATUS_CARD, "Status", DASH_STATUS_IDLE);
      Card _output2DS18State = Card(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
      Card _output2DimmerSlider = Card(&dashboard, SLIDER_CARD, "Dimmer Level Manual Control", "", 0, YASOLR_DIMMER_MAX_LEVEL, 1);
      Card _output2DimmerSliderRO = Card(&dashboard, PROGRESS_CARD, "Dimmer Level", "", 0, YASOLR_DIMMER_MAX_LEVEL, 1);
      Card _output2Bypass = Card(&dashboard, BUTTON_CARD, "Bypass Manual Control");
      Card _output2BypassRO = Card(&dashboard, STATUS_CARD, "Bypass");
      Card _output2Power = Card(&dashboard, ENERGY_CARD, "Power", "W");
      Card _output2ApparentPower = Card(&dashboard, ENERGY_CARD, "Apparent Power", "VA");
      Card _output2PowerFactor = Card(&dashboard, ENERGY_CARD, "Power Factor");
      Card _output2THDi = Card(&dashboard, ENERGY_CARD, "THDi", "%");
      Card _output2Voltage = Card(&dashboard, ENERGY_CARD, "Voltage", "V");
      Card _output2Current = Card(&dashboard, ENERGY_CARD, "Current", "A");
      Card _output2Resistance = Card(&dashboard, ENERGY_CARD, "Resistance", "Ω");
      Card _output2Energy = Card(&dashboard, ENERGY_CARD, "Energy", "kWh");
      Card _output2DimmerAuto = Card(&dashboard, BUTTON_CARD, "Dimmer Automatic Control");
      Card _output2DimmerLimiter = Card(&dashboard, SLIDER_CARD, "Dimmer Limiter", "", 0, YASOLR_DIMMER_MAX_LEVEL, 1);
      Card _output2AutoBypass = Card(&dashboard, BUTTON_CARD, "Bypass Automatic Control");
      Card _output2AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, "Bypass Week Days");
      Card _output2AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Start Time");
      Card _output2AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Stop Time");
      Card _output2AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Start Temperature");
      Card _output2AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, "Bypass Stop Temperature");

      Tab _relaysTab = Tab(&dashboard, "\u2600 Relays");
      Card _relay1Load = Card(&dashboard, TEXT_INPUT_CARD, "Relay 1 Automatic Control: Connected Load (Watts)");
      Card _relay1Switch = Card(&dashboard, BUTTON_CARD, "Relay 1 Manual Control");
      Card _relay1SwitchRO = Card(&dashboard, STATUS_CARD, "Relay 1");
      Card _relay2Load = Card(&dashboard, TEXT_INPUT_CARD, "Relay 2 Automatic Control: Connected Load (Watts)");
      Card _relay2Switch = Card(&dashboard, BUTTON_CARD, "Relay 2  Manual Control");
      Card _relay2SwitchRO = Card(&dashboard, STATUS_CARD, "Relay 2");

      Tab _managementTab = Tab(&dashboard, "\u2764 Management");
      Card _configBackup = Card(&dashboard, LINK_CARD, "Configuration Backup");
      Card _configRestore = Card(&dashboard, FILE_UPLOAD_CARD, "Configuration Restore", ".txt");
      Card _otaLink = Card(&dashboard, LINK_CARD, "OTA Firmware Update");
      Card _restart = Card(&dashboard, PUSH_BUTTON_CARD, "Restart");
      Card _energyResetO1 = Card(&dashboard, PUSH_BUTTON_CARD, "Energy Reset (only Output 1)");
      Card _energyResetO2 = Card(&dashboard, PUSH_BUTTON_CARD, "Energy Reset (only Output 2)");
      Card _energyReset = Card(&dashboard, PUSH_BUTTON_CARD, "Energy Reset");
      Card _reset = Card(&dashboard, PUSH_BUTTON_CARD, "Factory Reset");
      Card _debugMode = Card(&dashboard, BUTTON_CARD, "Debug");
      Card _consoleLink = Card(&dashboard, LINK_CARD, "Console");

      Tab _networkConfigTab = Tab(&dashboard, "\u2728 Network");
      Card _adminPwd = Card(&dashboard, PASSWORD_CARD, "Admin Password");
      Card _ntpServer = Card(&dashboard, TEXT_INPUT_CARD, "NTP Server");
      Card _ntpTimezone = Card(&dashboard, ASYNC_DROPDOWN_CARD, "Timezone");
      Card _ntpSync = Card(&dashboard, TIME_SYNC_CARD, "Sync time with browser");
      Card _wifiSSID = Card(&dashboard, TEXT_INPUT_CARD, "WiFi SSID");
      Card _wifiPwd = Card(&dashboard, PASSWORD_CARD, "WiFi Password");
      Card _apMode = Card(&dashboard, BUTTON_CARD, "Stay in AP Mode");

      Tab _mqttConfigTab = Tab(&dashboard, "\u2728 MQTT");
      Card _mqttServer = Card(&dashboard, TEXT_INPUT_CARD, "Server");
      Card _mqttPort = Card(&dashboard, TEXT_INPUT_CARD, "Port");
      Card _mqttUser = Card(&dashboard, TEXT_INPUT_CARD, "Username");
      Card _mqttPwd = Card(&dashboard, PASSWORD_CARD, "Password");
      Card _mqttSecured = Card(&dashboard, BUTTON_CARD, "SSL / TLS");
      Card _mqttServerCert = Card(&dashboard, FILE_UPLOAD_CARD, "Server Certificate", ".pem,crt,der");
      Card _mqttPublishInterval = Card(&dashboard, SLIDER_CARD, "Publish Interval", "s", 5, 30, 1);
      Card _mqttTopic = Card(&dashboard, TEXT_INPUT_CARD, "Base Topic");
      Card _haDiscovery = Card(&dashboard, BUTTON_CARD, "Home Assistant Integration");
      Card _haDiscoveryTopic = Card(&dashboard, TEXT_INPUT_CARD, "Home Assistant Discovery Topic");
      Card _mqttGridVoltage = Card(&dashboard, TEXT_INPUT_CARD, "Grid Voltage from MQTT Topic");
      Card _mqttGridPower = Card(&dashboard, TEXT_INPUT_CARD, "Grid Power from MQTT Topic");

      Tab _pinConfigTab = Tab(&dashboard, "\u21C6 GPIO");
      Card _pinDimmerO1 = Card(&dashboard, TEXT_INPUT_CARD, "Dimmer for Output 1");
      Card _pinDimmerO2 = Card(&dashboard, TEXT_INPUT_CARD, "Dimmer for Output 2");
      Card _pinDisplayClock = Card(&dashboard, TEXT_INPUT_CARD, "Display SCL (CLOCK)");
      Card _pinDisplayData = Card(&dashboard, TEXT_INPUT_CARD, "Display SDA (DATA)");
      Card _pinDS18O1 = Card(&dashboard, TEXT_INPUT_CARD, "DS18 for Output 1");
      Card _pinDS18O2 = Card(&dashboard, TEXT_INPUT_CARD, "DS18 for Output 2");
      Card _pinDS18Router = Card(&dashboard, TEXT_INPUT_CARD, "DS18 for Router");
      Card _pinJsyRX = Card(&dashboard, TEXT_INPUT_CARD, "Serial RX for JSY TX");
      Card _pinJsyTX = Card(&dashboard, TEXT_INPUT_CARD, "Serial TX for JSY RX");
      Card _pinLEDGreen = Card(&dashboard, TEXT_INPUT_CARD, "LED Green");
      Card _pinLEDRed = Card(&dashboard, TEXT_INPUT_CARD, "LED Red");
      Card _pinLEDYellow = Card(&dashboard, TEXT_INPUT_CARD, "LED Yellow");
      Card _pinPZEMRX = Card(&dashboard, TEXT_INPUT_CARD, "Serial RX for PZEM TX");
      Card _pinPZEMTX = Card(&dashboard, TEXT_INPUT_CARD, "Serial TX for PZEM RX");
      Card _pinRelay1 = Card(&dashboard, TEXT_INPUT_CARD, "Relay 1");
      Card _pinRelay2 = Card(&dashboard, TEXT_INPUT_CARD, "Relay 2");
      Card _pinRelayO1 = Card(&dashboard, TEXT_INPUT_CARD, "Relay for Output 1 Bypass");
      Card _pinRelayO2 = Card(&dashboard, TEXT_INPUT_CARD, "Relay for Output 2 Bypass");
      Card _pinZCD = Card(&dashboard, TEXT_INPUT_CARD, "Zero-Cross Detection");

      Tab _hardwareEnableTab = Tab(&dashboard, "\u2699 Hardware");
      Card _display = Card(&dashboard, BUTTON_CARD, "Display");
      Card _jsy = Card(&dashboard, BUTTON_CARD, "JSY");
      Card _led = Card(&dashboard, BUTTON_CARD, "LEDs");
      Card _mqtt = Card(&dashboard, BUTTON_CARD, "MQTT");
      Card _output1Dimmer = Card(&dashboard, BUTTON_CARD, "Output 1 Dimmer");
      Card _output1DS18 = Card(&dashboard, BUTTON_CARD, "Output 1 DS18");
      Card _output1PZEM = Card(&dashboard, BUTTON_CARD, "Output 1 PZEM");
      Card _output1Relay = Card(&dashboard, BUTTON_CARD, "Output 1 Relay (Bypass)");
      Card _output2Dimmer = Card(&dashboard, BUTTON_CARD, "Output 2 Dimmer");
      Card _output2DS18 = Card(&dashboard, BUTTON_CARD, "Output 2 DS18");
      Card _output2PZEM = Card(&dashboard, BUTTON_CARD, "Output 2 PZEM");
      Card _output2Relay = Card(&dashboard, BUTTON_CARD, "Output 2 Relay (Bypass)");
      Card _relay1 = Card(&dashboard, BUTTON_CARD, "Relay 1");
      Card _relay2 = Card(&dashboard, BUTTON_CARD, "Relay 2");
      Card _routerDS18 = Card(&dashboard, BUTTON_CARD, "Router DS18");
      Card _zcd = Card(&dashboard, BUTTON_CARD, "Zero-Cross Detection");

      Tab _hardwareConfigTab = Tab(&dashboard, "\u2699 Hardware Config");
      Card _gridFreq = Card(&dashboard, DROPDOWN_CARD, "Grid Frequency (default)");
      Card _gridVolt = Card(&dashboard, DROPDOWN_CARD, "Grid Voltage (default)");
      Card _displayType = Card(&dashboard, DROPDOWN_CARD, "Display Type");
      Card _displaySpeed = Card(&dashboard, SLIDER_CARD, "Display Speed", "s", 1, 10, 1);
      Card _displayRotation = Card(&dashboard, DROPDOWN_CARD, "Display Rotation");
      Card _relay1Type = Card(&dashboard, DROPDOWN_CARD, "Relay 1 Type");
      Card _relay2Type = Card(&dashboard, DROPDOWN_CARD, "Relay 2 Type");
      Card _output1RelayType = Card(&dashboard, DROPDOWN_CARD, "Output 1 Bypass Relay Type");
      Card _output2RelayType = Card(&dashboard, DROPDOWN_CARD, "Output 2 Bypass Relay Type");
      Card _output1PZEMSync = Card(&dashboard, PUSH_BUTTON_CARD, "Output 1 PZEM Pairing");
      Card _output2PZEMSync = Card(&dashboard, PUSH_BUTTON_CARD, "Output 2 PZEM Pairing");
#endif

    private:
      void _boolConfig(Card& card, const char* key);
      void _daysConfig(Card& card, const char* key);
      void _numConfig(Card& card, const char* key);
      void _pinConfig(Card& card, const char* key);
      void _passwordConfig(Card& card, const char* key);
      void _sliderConfig(Card& card, const char* key);
      void _textConfig(Card& card, const char* key);

      void _outputDimmerSlider(Card& card, Mycila::RouterOutput& output);
      void _outputBypassSwitch(Card& card, Mycila::RouterOutput& output);
      void _relaySwitch(Card& card, const char* relayName);

      void _pinout(Card& card, int32_t pin, std::map<int32_t, Card*>& pinout);
      void _status(Card& card, const char* key, bool enabled, bool state = true, const char* err = "");
      void _temperature(Card& card, Mycila::DS18& sensor);
  };

  extern WebsiteClass Website;
} // namespace YaSolR
