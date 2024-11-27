// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolRWebsite.h>

#include <string>
#include <unordered_map>

#define HIDDEN_PWD "********"

#ifdef APP_MODEL_OSS
  #define LINE_CHART  BAR_CHART
  #define AREA_CHART  BAR_CHART
  #define ENERGY_CARD GENERIC_CARD
#endif

#ifdef APP_MODEL_PRO
static const ChartSize chartSize = {.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12};
#endif

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
Card _mqttServerCert = Card(&dashboard, FILE_UPLOAD_CARD, YASOLR_LBL_101, ".pem");
Card _mqttServerCertDelete = Card(&dashboard, PUSH_BUTTON_CARD, YASOLR_LBL_049);
Card _mqttPublishInterval = Card(&dashboard, SLIDER_CARD, YASOLR_LBL_102, "s", 1, 30, 1);
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

void YaSolR::Website::initLayout() {
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
      config.set(KEY_OUTPUT1_RESISTANCE, Mycila::string::to_string(router.getOutputs()[0]->config.calibratedResistance, 2));
      config.set(KEY_OUTPUT2_RESISTANCE, Mycila::string::to_string(router.getOutputs()[1]->config.calibratedResistance, 2));
    });

    dashboardInitTask.resume();
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
    const size_t len = strlen(value);
    timeval tv;
    tv.tv_sec = std::stoul(std::string(value, len - 3));
    tv.tv_usec = atol(value + len - 3);
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
      dashboardInitTask.resume();
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
      config.set(KEY_OUTPUT1_DIMMER_MIN, std::string(value, comma - value));
      config.set(KEY_OUTPUT1_DIMMER_MAX, comma + 1);
    }
    _output1DimmerMapper.update(value);
    dashboard.refreshCard(&_output1DimmerMapper);
  });
  _output2DimmerMapper.attachCallback([this](const char* value) {
    const char* comma = strchr(value, ',');
    if (comma != nullptr) {
      config.set(KEY_OUTPUT2_DIMMER_MIN, std::string(value, comma - value));
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

void YaSolR::Website::initCards() {
  logger.debug(TAG, "Initializing cards");

  // Statistics

  _appManufacturer.set(Mycila::AppInfo.manufacturer);
  _appModel.set(Mycila::AppInfo.model);
  _appName.set(Mycila::AppInfo.name);
  _appVersion.set(Mycila::AppInfo.version);
  _deviceBootCount.set(std::to_string(Mycila::System::getBootCount()));
  _deviceBootReason.set(Mycila::System::getLastRebootReason());
  _deviceCores.set(std::to_string(ESP.getChipCores()));
  _deviceModel.set(ESP.getChipModel());
  _deviceRev.set(std::to_string(ESP.getChipRevision()));
  _deviceID.set(Mycila::AppInfo.id);
  _firmwareBuildHash.set(Mycila::AppInfo.buildHash);
  _firmwareBuildTimestamp.set(Mycila::AppInfo.buildDate);
  _firmwareFilename.set(Mycila::AppInfo.firmware);
  _networkAPMAC.set(espConnect.getMACAddress(Mycila::ESPConnect::Mode::AP));
  _networkEthMAC.set(espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH).empty() ? std::string("N/A") : espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH));
  _networkHostname.set(Mycila::AppInfo.defaultHostname);
  _networkWiFiMAC.set(espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA));

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
  _displayRotation.update(config.getString(KEY_DISPLAY_ROTATION) + "°", "0°,90°,180°,270°");
  _output1RelayType.update(config.get(KEY_OUTPUT1_RELAY_TYPE), "NO,NC");
  _output2RelayType.update(config.get(KEY_OUTPUT2_RELAY_TYPE), "NO,NC");
  _relay1Type.update(config.get(KEY_RELAY1_TYPE), "NO,NC");
  _relay2Type.update(config.get(KEY_RELAY2_TYPE), "NO,NC");
  _relay1Load.update(load1);
  _relay2Load.update(load2);
  _output1ResistanceInput.update(config.get(KEY_OUTPUT1_RESISTANCE), config.getFloat(KEY_OUTPUT1_RESISTANCE) == 0 ? DASH_STATUS_DANGER : DASH_STATUS_SUCCESS);
  _output2ResistanceInput.update(config.get(KEY_OUTPUT2_RESISTANCE), config.getFloat(KEY_OUTPUT2_RESISTANCE) == 0 ? DASH_STATUS_DANGER : DASH_STATUS_SUCCESS);
  _output1DimmerMapper.update(config.getString(KEY_OUTPUT1_DIMMER_MIN) + "," + config.get(KEY_OUTPUT1_DIMMER_MAX));
  _output2DimmerMapper.update(config.getString(KEY_OUTPUT2_DIMMER_MIN) + "," + config.get(KEY_OUTPUT2_DIMMER_MAX));

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

void YaSolR::Website::updateCards() {
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
  _output1RelaySwitchCount.set(std::to_string(bypassRelayO1.getSwitchCount()));
  _output2RelaySwitchCount.set(std::to_string(bypassRelayO2.getSwitchCount()));
  _deviceHeapTotal.set(std::to_string(memory.total) + " bytes");
  _deviceHeapUsed.set(std::to_string(memory.used) + " bytes");
  _deviceHeapUsage.set(Mycila::string::to_string(memory.usage, 2) + " %");
  _gridEnergy.set(Mycila::string::to_string(gridMetrics.energy, 3) + " kWh");
  _gridEnergyReturned.set(Mycila::string::to_string(gridMetrics.energyReturned, 3) + " kWh");
  _gridFrequency.set(Mycila::string::to_string(detectGridFrequency(), 0) + " Hz");
  _networkAPIP.set(espConnect.getIPAddress(Mycila::ESPConnect::Mode::AP).toString().c_str());
  _networkEthIP.set(espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  _networkInterface.set(mode == Mycila::ESPConnect::Mode::AP ? "AP" : (mode == Mycila::ESPConnect::Mode::STA ? "WiFi" : (mode == Mycila::ESPConnect::Mode::ETH ? "Ethernet" : "")));
  _networkWiFiIP.set(espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  _networkWiFiRSSI.set(std::to_string(espConnect.getWiFiRSSI()) + " dBm");
  _networkWiFiSignal.set(std::to_string(espConnect.getWiFiSignalQuality()) + " %");
  _networkWiFiSSID.set(espConnect.getWiFiSSID());
  _relay1SwitchCount.set(std::to_string(relay1.getSwitchCount()));
  _relay2SwitchCount.set(std::to_string(relay2.getSwitchCount()));
  _udpMessageRateBuffer.set(Mycila::string::to_string(udpMessageRateBuffer.rate(), 2) + " msg/s");
  _time.set(Mycila::Time::getLocalStr());
  _uptime.set(Mycila::Time::toDHHMMSS(Mycila::System::getUptime()));
#ifdef APP_MODEL_TRIAL
  _trialRemainingTime.set(Mycila::Time::toDHHMMSS(Mycila::Trial.getRemaining()));
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

void YaSolR::Website::updateCharts() {
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

void YaSolR::Website::updatePID() {
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

void YaSolR::Website::resetPID() {
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

void YaSolR::Website::_sliderConfig(Card& card, const char* key) {
  card.attachCallback([key, &card](int value) {
    config.set(key, std::to_string(value));
    card.update(config.getInt(key));
    dashboard.refreshCard(&card);
  });
}

void YaSolR::Website::_percentageSlider(Card& card, const char* key) {
  card.attachCallback([key, &card](int value) {
    config.set(key, std::to_string(value));
    card.update(value);
    dashboard.refreshCard(&card);
  });
}

void YaSolR::Website::_floatConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    if (value[0]) {
      config.set(key, value);
    } else {
      config.unset(key);
    }
    card.update(config.get(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::Website::_numConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    if (value[0]) {
      config.set(key, std::to_string(strtol(value, nullptr, 10)));
    } else {
      config.unset(key);
    }
    card.update(config.getInt(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::Website::_pinConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card, this](const char* value) {
    if (value[0]) {
      config.set(key, std::to_string(strtol(value, nullptr, 10)));
    } else {
      config.unset(key);
    }
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::Website::_boolConfig(Card& card, const char* key) {
  card.attachCallback([key, &card, this](int value) {
    config.setBool(key, value);
    card.update(config.getBool(key) ? 1 : 0);
    dashboard.refreshCard(&card);
  });
}

void YaSolR::Website::_textConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card](const char* value) {
    config.set(key, value);
    card.update(config.get(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::Website::_daysConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card, this](const char* value) {
    config.set(key, value[0] ? value : "none");
    card.update(config.get(key));
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::Website::_passwordConfig(Card& card, const char* key) {
#ifdef APP_MODEL_PRO
  card.attachCallback([key, &card, this](const char* value) {
    if (value[0]) {
      config.set(key, value);
    } else {
      config.unset(key);
    }
    card.update(config.isEmpty(key) ? "" : HIDDEN_PWD);
    dashboard.refreshCard(&card);
  });
#endif
}

void YaSolR::Website::_relaySwitch(Card& card, Mycila::RouterRelay& relay) {
  card.attachCallback([&card, &relay, this](int value) {
    relay.tryRelayState(value);
    card.update(relay.isOn());
    dashboard.refreshCard(&card);
  });
}

void YaSolR::Website::_outputBypassSwitch(Card& card, Mycila::RouterOutput& output) {
  card.attachCallback([&card, &output, this](int value) {
    if (output.isBypassEnabled()) {
      output.setBypass(value);
    }
    card.update(output.isBypassOn());
    dashboard.refreshCard(&card);
    dashboardInitTask.resume();
  });
}

void YaSolR::Website::_outputDimmerSlider(Card& card, Mycila::RouterOutput& output) {
  card.attachCallbackF([&card, &output, this](float value) {
    if (output.isDimmerEnabled()) {
      output.setDimmerDutyCycle(value / 100);
    }
    card.update(output.getDimmerDutyCycle() * 100);
    dashboard.refreshCard(&card);
    dashboardUpdateTask.requestEarlyRun();
  });
}

void YaSolR::Website::_temperature(Card& card, Mycila::DS18& sensor) {
  if (!sensor.isEnabled()) {
    card.update(YASOLR_LBL_115, "");
  } else if (!sensor.isValid()) {
    card.update(YASOLR_LBL_123, "");
  } else {
    card.update(sensor.getTemperature().value_or(0), "°C");
  }
}

void YaSolR::Website::_temperature(Card& card, Mycila::RouterOutput& output) {
  if (output.temperature().neverUpdated()) {
    card.update(YASOLR_LBL_115, "");
  } else if (output.temperature().isAbsent()) {
    card.update(YASOLR_LBL_123, "");
  } else {
    card.update(output.temperature().get(), "°C");
  }
}

void YaSolR::Website::_status(Card& card, const char* key, bool enabled, bool active, const char* err) {
  const bool configEnabled = config.getBool(key);
  if (!configEnabled)
    card.update(config.getBool(key), DASH_STATUS_IDLE "," YASOLR_LBL_115);
  else if (!enabled)
    card.update(config.getBool(key), DASH_STATUS_DANGER "," YASOLR_LBL_124);
  else if (!active)
    card.update(config.getBool(key), (std::string(DASH_STATUS_WARNING) + "," + err).c_str());
  else
    card.update(config.getBool(key), DASH_STATUS_SUCCESS "," YASOLR_LBL_130);
}

void YaSolR::Website::_pinout(Card& card, int32_t pin, std::unordered_map<int32_t, Card*>& pinout) {
  if (pin == GPIO_NUM_NC) {
    card.update(YASOLR_LBL_115, DASH_STATUS_IDLE);
  } else if (pinout.find(pin) != pinout.end()) {
    std::string v = std::to_string(pin) + " (" YASOLR_LBL_153 ")";
    pinout[pin]->update(v, DASH_STATUS_DANGER);
    card.update(v, DASH_STATUS_DANGER);
  } else if (!GPIO_IS_VALID_GPIO(pin)) {
    pinout[pin] = &card;
    card.update(std::to_string(pin) + " (" YASOLR_LBL_154 ")", DASH_STATUS_DANGER);
  } else if (!GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    pinout[pin] = &card;
    card.update(std::to_string(pin) + " (" YASOLR_LBL_155 ")", DASH_STATUS_WARNING);
  } else {
    pinout[pin] = &card;
    card.update(std::to_string(pin) + " (" YASOLR_LBL_156 ")", DASH_STATUS_SUCCESS);
  }
}
