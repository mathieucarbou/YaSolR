// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <YaSolR.h>

#include <unordered_map>

#ifdef APP_MODEL_OSS
  #define LINE_CHART  BAR_CHART
  #define AREA_CHART  BAR_CHART
  #define ENERGY_CARD GENERIC_CARD
#endif

namespace YaSolR {
  class WebsiteClass {
    public:
      void initLayout();
      void initCards();
      void updateCards();
      void updateCharts();
      void updatePID();
      void resetPID();

    private:
      int _historyX[YASOLR_GRAPH_POINTS] = {0};

      // statistics
      Statistic _appName = Statistic(&dashboard, YASOLR_LBL_001);
      Statistic _appModel = Statistic(&dashboard, YASOLR_LBL_002);
      Statistic _appVersion = Statistic(&dashboard, YASOLR_LBL_003);
      Statistic _appManufacturer = Statistic(&dashboard, YASOLR_LBL_004);

      Statistic _deviceBootCount = Statistic(&dashboard, YASOLR_LBL_005);
      Statistic _deviceBootReason = Statistic(&dashboard, YASOLR_LBL_192);
      Statistic _deviceCores = Statistic(&dashboard, YASOLR_LBL_006);
      Statistic _deviceHeapTotal = Statistic(&dashboard, YASOLR_LBL_007);
      Statistic _deviceHeapUsage = Statistic(&dashboard, YASOLR_LBL_008);
      Statistic _deviceHeapUsed = Statistic(&dashboard, YASOLR_LBL_009);
      Statistic _deviceID = Statistic(&dashboard, YASOLR_LBL_010);
      Statistic _deviceModel = Statistic(&dashboard, YASOLR_LBL_011);
      Statistic _deviceRev = Statistic(&dashboard, YASOLR_LBL_012);

      Statistic _firmwareBuildHash = Statistic(&dashboard, YASOLR_LBL_013);
      Statistic _firmwareBuildTimestamp = Statistic(&dashboard, YASOLR_LBL_014);
      Statistic _firmwareFilename = Statistic(&dashboard, YASOLR_LBL_015);

      Statistic _gridEnergy = Statistic(&dashboard, YASOLR_LBL_016);
      Statistic _gridEnergyReturned = Statistic(&dashboard, YASOLR_LBL_017);
      Statistic _gridFrequency = Statistic(&dashboard, YASOLR_LBL_018);

      Statistic _udpMessageRateBuffer = Statistic(&dashboard, YASOLR_LBL_157);

      Statistic _networkHostname = Statistic(&dashboard, YASOLR_LBL_019);
      Statistic _networkInterface = Statistic(&dashboard, YASOLR_LBL_020);
      Statistic _networkAPIP = Statistic(&dashboard, YASOLR_LBL_021);
      Statistic _networkAPMAC = Statistic(&dashboard, YASOLR_LBL_022);
      Statistic _networkEthIP = Statistic(&dashboard, YASOLR_LBL_023);
      Statistic _networkEthMAC = Statistic(&dashboard, YASOLR_LBL_024);
      Statistic _networkWiFiIP = Statistic(&dashboard, YASOLR_LBL_025);
      Statistic _networkWiFiMAC = Statistic(&dashboard, YASOLR_LBL_026);
      Statistic _networkWiFiSSID = Statistic(&dashboard, YASOLR_LBL_027);
      Statistic _networkWiFiRSSI = Statistic(&dashboard, YASOLR_LBL_028);
      Statistic _networkWiFiSignal = Statistic(&dashboard, YASOLR_LBL_029);

      Statistic _output1RelaySwitchCount = Statistic(&dashboard, YASOLR_LBL_030);
      Statistic _output2RelaySwitchCount = Statistic(&dashboard, YASOLR_LBL_031);
      Statistic _relay1SwitchCount = Statistic(&dashboard, YASOLR_LBL_032);
      Statistic _relay2SwitchCount = Statistic(&dashboard, YASOLR_LBL_033);

      Statistic _time = Statistic(&dashboard, YASOLR_LBL_034);
      Statistic _uptime = Statistic(&dashboard, YASOLR_LBL_035);
#ifdef APP_MODEL_TRIAL
      Statistic _trialRemainingTime = Statistic(&dashboard, "Trial Remaining Time");
#endif

      // home

      Card _routerPower = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_036, "W");
      Card _routerApparentPower = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_037, "VA");
      Card _routerPowerFactor = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_038);
      Card _routerTHDi = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_039, "%");
      Card _routerVoltage = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_040, "V");
      Card _routerCurrent = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_041, "A");
      Card _routerResistance = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_042, "Ω");
      Card _routerEnergy = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_043, "kWh");
      Card _gridPower = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_044, "W");
      Card _routerDS18State = Card(&dashboard, TEMPERATURE_CARD, YASOLR_LBL_045, "°C");
#ifdef APP_MODEL_OSS
      Card _relay1Switch = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_073);
      Card _relay2Switch = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_076);

      Card _output1State = Card(&dashboard, STATUS_CARD, YASOLR_LBL_046, DASH_STATUS_IDLE);
      Card _output1DS18State = Card(&dashboard, TEMPERATURE_CARD, YASOLR_LBL_046 ": " YASOLR_LBL_048, "°C");
      Card _output1DimmerSlider = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_046 ": " YASOLR_LBL_050, "%", 0.0f, 100.0f, 0.01f);
      Card _output1Bypass = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_046 ": " YASOLR_LBL_051);

      Card _output2State = Card(&dashboard, STATUS_CARD, YASOLR_LBL_070, DASH_STATUS_IDLE);
      Card _output2DS18State = Card(&dashboard, TEMPERATURE_CARD, YASOLR_LBL_070 ": " YASOLR_LBL_048, "°C");
      Card _output2DimmerSlider = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_070 ": " YASOLR_LBL_050, "%", 0.0f, 100.0f, 0.01f);
      Card _output2Bypass = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_070 ": " YASOLR_LBL_051);
#endif

      int _gridPowerHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _routedPowerHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _routerTHDiHistoryY[YASOLR_GRAPH_POINTS] = {0};
      Chart _gridPowerHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_044 " (W)");
      Chart _routedPowerHistory = Chart(&dashboard, AREA_CHART, YASOLR_LBL_036 " (W)");
      Chart _routerTHDiHistory = Chart(&dashboard, BAR_CHART, YASOLR_LBL_039 " (%)");

#ifdef APP_MODEL_OSS
      Card _output1PZEMSync = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_147);
      Card _output2PZEMSync = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_148);
      Card _resistanceCalibration = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_186);
#endif

#ifdef APP_MODEL_PRO
      // tabs icons:
      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Miscellaneous_Symbols
      // https://en.wikipedia.org/wiki/List_of_Unicode_characters#Dingbats

      Tab _output1Tab = Tab(&dashboard, "\u2600 " YASOLR_LBL_046);
      Card _output1State = Card(&dashboard, STATUS_CARD, YASOLR_LBL_047, DASH_STATUS_IDLE);
      Card _output1DS18State = Card(&dashboard, TEMPERATURE_CARD, YASOLR_LBL_048, "°C");
      Card _output1DimmerSlider = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_050, "%", 0.0f, 100.0f, 0.01f);
      Card _output1DimmerSliderRO = Card(&dashboard, PROGRESS_CARD, YASOLR_LBL_050, "%", 0.0f, 100.0f, 0.01f);
      Card _output1Bypass = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_051);
      Card _output1BypassRO = Card(&dashboard, STATUS_CARD, YASOLR_LBL_051);
      Card _output1Power = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_052, "W");
      Card _output1ApparentPower = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_053, "VA");
      Card _output1PowerFactor = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_054);
      Card _output1THDi = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_055, "%");
      Card _output1Voltage = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_056, "V");
      Card _output1Current = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_057, "A");
      Card _output1Resistance = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_058, "Ω");
      Card _output1Energy = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_059, "kWh");
      Card _output1DimmerDutyLimiter = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_062, "%", 0, 100, 1);
      Card _output1DimmerTempLimiter = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_063, "°C");
      Card _output1DimmerAuto = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_060);
      Card _output1DimmerReservedExcess = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_061, "%", 0, 100, 1);
      Card _output1BypassAuto = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_064);
      Card _output1AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_065, "°C");
      Card _output1AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_066, "°C");
      Card _output1AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_067);
      Card _output1AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_068);
      Card _output1AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, YASOLR_LBL_069);

      Tab _output2Tab = Tab(&dashboard, "\u2600 " YASOLR_LBL_070);
      Card _output2State = Card(&dashboard, STATUS_CARD, YASOLR_LBL_047, DASH_STATUS_IDLE);
      Card _output2DS18State = Card(&dashboard, TEMPERATURE_CARD, YASOLR_LBL_048, "°C");
      Card _output2DimmerSlider = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_050, "%", 0.0f, 100.0f, 0.01f);
      Card _output2DimmerSliderRO = Card(&dashboard, PROGRESS_CARD, YASOLR_LBL_050, "%", 0.0f, 100.0f, 0.01f);
      Card _output2Bypass = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_051);
      Card _output2BypassRO = Card(&dashboard, STATUS_CARD, YASOLR_LBL_051);
      Card _output2Power = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_052, "W");
      Card _output2ApparentPower = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_053, "VA");
      Card _output2PowerFactor = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_054);
      Card _output2THDi = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_055, "%");
      Card _output2Voltage = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_056, "V");
      Card _output2Current = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_057, "A");
      Card _output2Resistance = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_058, "Ω");
      Card _output2Energy = Card(&dashboard, ENERGY_CARD, YASOLR_LBL_059, "kWh");
      Card _output2DimmerDutyLimiter = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_062, "%", 0, 100, 1);
      Card _output2DimmerTempLimiter = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_063, "°C");
      Card _output2DimmerAuto = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_060);
      Card _output2DimmerReservedExcess = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_158, "%", 0, 100, 1);
      Card _output2BypassAuto = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_064);
      Card _output2AutoStartTemp = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_065);
      Card _output2AutoStoptTemp = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_066);
      Card _output2AutoStartTime = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_067);
      Card _output2AutoStoptTime = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_068);
      Card _output2AutoStartWDays = Card(&dashboard, WEEK_SELECTOR_CARD, YASOLR_LBL_069);

      Tab _relaysTab = Tab(&dashboard, "\u2600 " YASOLR_LBL_071);
      Card _relay1Switch = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_073);
      Card _relay1SwitchRO = Card(&dashboard, STATUS_CARD, YASOLR_LBL_074);
      Card _relay2Switch = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_076);
      Card _relay2SwitchRO = Card(&dashboard, STATUS_CARD, YASOLR_LBL_077);

      Tab _managementTab = Tab(&dashboard, "\u2764 " YASOLR_LBL_078);
      Card _configBackup = Card(&dashboard, LINK_CARD, YASOLR_LBL_079);
      Card _configRestore = Card(&dashboard, FILE_UPLOAD_CARD, YASOLR_LBL_080, ".txt");
      Card _restart = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_082);
      Card _safeBoot = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_081);
      Card _energyReset = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_085);
      Card _reset = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_086);
      Card _debugMode = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_083);
      Card _consoleLink = Card(&dashboard, LINK_CARD, YASOLR_LBL_084);
      Card _debugInfo = Card(&dashboard, LINK_CARD, YASOLR_LBL_178);

      Tab _networkConfigTab = Tab(&dashboard, "\u2728 " YASOLR_LBL_087);
      Card _adminPwd = Card(&dashboard, PASSWORD_CARD, YASOLR_LBL_088);
      Card _ntpServer = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_089);
      Card _ntpTimezone = Card(&dashboard, ASYNC_DROPDOWN_CARD, YASOLR_LBL_090);
      Card _ntpSync = Card(&dashboard, TIME_SYNC_CARD, YASOLR_LBL_091);
      Card _wifiSSID = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_092);
      Card _wifiPwd = Card(&dashboard, PASSWORD_CARD, YASOLR_LBL_093);
      Card _staticIP = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_188);
      Card _subnetMask = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_189);
      Card _gateway = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_190);
      Card _dnsServer = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_191);
      Card _apMode = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_094);

      Tab _mqttConfigTab = Tab(&dashboard, "\u2728 " YASOLR_LBL_095);
      Card _mqttServer = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_096);
      Card _mqttPort = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_097);
      Card _mqttUser = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_098);
      Card _mqttPwd = Card(&dashboard, PASSWORD_CARD, YASOLR_LBL_099);
      Card _mqttSecured = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_100);
      Card _mqttServerCert = Card(&dashboard, FILE_UPLOAD_CARD, YASOLR_LBL_101, ".pem,crt,der");
      Card _mqttServerCertDelete = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_049);
      Card _mqttPublishInterval = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_102, "s", 5, 30, 1);
      Card _mqttTopic = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_103);
      Card _haDiscovery = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_104);
      Card _haDiscoveryTopic = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_105);
      Card _mqttGridVoltage = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_106);
      Card _mqttGridPower = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_107);
      Card _mqttTempO1 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_181);
      Card _mqttTempO2 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_182);

      Tab _pinConfigTab = Tab(&dashboard, "\u21C6 " YASOLR_LBL_108);
      Card _pinDisplayClock = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_111);
      Card _pinDisplayData = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_112);
      Card _pinJsyRX = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_116);
      Card _pinJsyTX = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_117);
      Card _pinLEDGreen = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_118);
      Card _pinLEDRed = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_119);
      Card _pinLEDYellow = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_120);
      Card _pinDimmerO1 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_131);
      Card _pinDS18O1 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_132);
      Card _pinRelayO1 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_134);
      Card _pinDimmerO2 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_135);
      Card _pinDS18O2 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_136);
      Card _pinRelayO2 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_138);
      Card _pinPZEMRX = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_121);
      Card _pinPZEMTX = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_122);
      Card _pinRelay1 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_074);
      Card _pinRelay2 = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_077);
      Card _pinDS18Router = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_139);
      Card _pinZCD = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_125);

      Tab _hardwareEnableTab = Tab(&dashboard, "\u2699 " YASOLR_LBL_126);
      Card _display = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_127);
      Card _jsy = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_128);
      Card _led = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_129);
      Card _mqtt = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_095);
      Card _output1Dimmer = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_131);
      Card _output1DS18 = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_132);
      Card _output1PZEM = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_133);
      Card _output1Relay = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_134);
      Card _output2Dimmer = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_135);
      Card _output2DS18 = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_136);
      Card _output2PZEM = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_137);
      Card _output2Relay = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_138);
      Card _relay1 = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_074);
      Card _relay2 = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_077);
      Card _routerDS18 = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_139);
      Card _zcd = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_125);

      Tab _hardwareConfigTab = Tab(&dashboard, "\u2699 " YASOLR_LBL_140);
      Card _gridFreq = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_141);
      Card _displaySpeed = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_142, "s", 1, 10, 1);
      Card _displayType = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_143);
      Card _displayRotation = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_144);
      Card _relay1Type = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_151);
      Card _relay2Type = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_152);
      Card _relay1Load = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_072);
      Card _relay2Load = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_075);
      Card _output1RelayType = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_149);
      Card _output2RelayType = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_150);
      Card _output1DimmerMapper = Card(&dashboard, RANGE_SLIDER_CARD, YASOLR_LBL_183, "%", 0, 100, 1);
      Card _output2DimmerMapper = Card(&dashboard, RANGE_SLIDER_CARD, YASOLR_LBL_184, "%", 0, 100, 1);
      Card _output1PZEMSync = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_147);
      Card _output2PZEMSync = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_148);
      Card _output1ResistanceInput = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_145);
      Card _output2ResistanceInput = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_146);
      Card _resistanceCalibration = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_186);

      Tab _pidTab = Tab(&dashboard, "\u2699 " YASOLR_LBL_159);
      Card _pidView = Card(&dashboard, BUTTON_CARD, YASOLR_LBL_169);
      Card _pidPMode = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_160);
      Card _pidDMode = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_161);
      Card _pidICMode = Card(&dashboard, DROPDOWN_CARD, YASOLR_LBL_162);
      Card _pidSetpoint = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_163);
      Card _pidKp = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_166);
      Card _pidKi = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_167);
      Card _pidKd = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_168);
      Card _pidOutMin = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_164);
      Card _pidOutMax = Card(&dashboard, TEXT_INPUT_CARD, YASOLR_LBL_165);
      Card _pidReset = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_177);

      // input,output,error,pTerm,iTerm,dTerm,sum
      int _pidInputHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _pidOutputHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _pidErrorHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _pidPTermHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _pidITermHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _pidDTermHistoryY[YASOLR_GRAPH_POINTS] = {0};
      int _pidSumHistoryY[YASOLR_GRAPH_POINTS] = {0};
      Chart _pidInputHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_170);
      Chart _pidOutputHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_171);
      Chart _pidErrorHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_172);
      Chart _pidSumHistory = Chart(&dashboard, BAR_CHART, YASOLR_LBL_173);
      Chart _pidPTermHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_174);
      Chart _pidITermHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_175);
      Chart _pidDTermHistory = Chart(&dashboard, LINE_CHART, YASOLR_LBL_176);
#endif

    private:
      void _boolConfig(Card& card, const char* key);
      void _daysConfig(Card& card, const char* key);
      void _floatConfig(Card& card, const char* key);
      void _numConfig(Card& card, const char* key);
      void _pinConfig(Card& card, const char* key);
      void _passwordConfig(Card& card, const char* key);
      void _sliderConfig(Card& card, const char* key);
      void _percentageSlider(Card& card, const char* key);
      void _textConfig(Card& card, const char* key);

      void _outputDimmerSlider(Card& card, Mycila::RouterOutput& output);
      void _outputBypassSwitch(Card& card, Mycila::RouterOutput& output);
      void _relaySwitch(Card& card, Mycila::RouterRelay& relay);

      void _pinout(Card& card, int32_t pin, std::unordered_map<int32_t, Card*>& pinout);
      void _status(Card& card, const char* key, bool enabled, bool state = true, const char* err = "");
      void _temperature(Card& card, Mycila::DS18& sensor);
      void _temperature(Card& card, Mycila::RouterOutput& output);
  };

  extern WebsiteClass Website;
} // namespace YaSolR
