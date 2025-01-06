// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr_dashboard.h>

#include <string>
#include <unordered_map>

#ifdef APP_MODEL_OSS
  #define LineChart  BarChart
  #define AreaChart  BarChart
  #define EnergyCard GenericCard
#endif

#ifdef APP_MODEL_PRO
static const ChartSize chartSize = {.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12};
#endif

int8_t _historyX[YASOLR_GRAPH_POINTS] = {0};

// statistics
dash::StatisticValue<const char*> _appName(dashboard, YASOLR_LBL_001);
dash::StatisticValue<const char*> _appModel(dashboard, YASOLR_LBL_002);
dash::StatisticValue<const char*> _appVersion(dashboard, YASOLR_LBL_003);
dash::StatisticValue<const char*> _appManufacturer(dashboard, YASOLR_LBL_004);

dash::StatisticValue<uint32_t> _deviceBootCount(dashboard, YASOLR_LBL_005);
dash::StatisticValue<const char*> _deviceBootReason(dashboard, YASOLR_LBL_192);
dash::StatisticValue<uint8_t> _deviceCores(dashboard, YASOLR_LBL_006);
dash::StatisticValue<size_t> _deviceHeapTotal(dashboard, YASOLR_LBL_007);
dash::StatisticValue<float, 2> _deviceHeapUsage(dashboard, YASOLR_LBL_008);
dash::StatisticValue<size_t> _deviceHeapUsed(dashboard, YASOLR_LBL_009);
dash::StatisticValue<const char*> _deviceID(dashboard, YASOLR_LBL_010);
dash::StatisticValue<const char*> _deviceModel(dashboard, YASOLR_LBL_011);
dash::StatisticValue<uint16_t> _deviceRev(dashboard, YASOLR_LBL_012);

dash::StatisticValue<const char*> _firmwareBuildHash(dashboard, YASOLR_LBL_013);
dash::StatisticValue<const char*> _firmwareBuildTimestamp(dashboard, YASOLR_LBL_014);
dash::StatisticValue<const char*> _firmwareFilename(dashboard, YASOLR_LBL_015);

dash::StatisticValue<float, 3> _gridEnergy(dashboard, YASOLR_LBL_016);
dash::StatisticValue<float, 3> _gridEnergyReturned(dashboard, YASOLR_LBL_017);
dash::StatisticValue<float, 0> _gridFrequency(dashboard, YASOLR_LBL_018);

dash::StatisticValue<float, 2> _udpMessageRateBuffer(dashboard, YASOLR_LBL_157);

dash::StatisticValue<const char*> _networkHostname(dashboard, YASOLR_LBL_019);
dash::StatisticValue<const char*> _networkInterface(dashboard, YASOLR_LBL_020);
dash::StatisticValue _networkAPIP(dashboard, YASOLR_LBL_021);
dash::StatisticValue _networkAPMAC(dashboard, YASOLR_LBL_022);
dash::StatisticValue _networkEthIP(dashboard, YASOLR_LBL_023);
dash::StatisticValue _networkEthMAC(dashboard, YASOLR_LBL_024);
dash::StatisticValue _networkWiFiIP(dashboard, YASOLR_LBL_025);
dash::StatisticValue _networkWiFiMAC(dashboard, YASOLR_LBL_026);
dash::StatisticValue _networkWiFiSSID(dashboard, YASOLR_LBL_027);
dash::StatisticValue<int8_t> _networkWiFiRSSI(dashboard, YASOLR_LBL_028);
dash::StatisticValue<int8_t> _networkWiFiSignal(dashboard, YASOLR_LBL_029);

dash::StatisticValue<uint64_t> _output1RelaySwitchCount(dashboard, YASOLR_LBL_030);
dash::StatisticValue<uint64_t> _output2RelaySwitchCount(dashboard, YASOLR_LBL_031);
dash::StatisticValue<uint64_t> _relay1SwitchCount(dashboard, YASOLR_LBL_032);
dash::StatisticValue<uint64_t> _relay2SwitchCount(dashboard, YASOLR_LBL_033);

dash::StatisticValue _time(dashboard, YASOLR_LBL_034);
dash::StatisticValue _uptime(dashboard, YASOLR_LBL_035);
#ifdef APP_MODEL_TRIAL
dash::StatisticValue _trialRemainingTime(dashboard, "Trial Remaining Time");
#endif

// home

dash::EnergyCard<float, 0> _routerPower(dashboard, YASOLR_LBL_036, "W");
dash::EnergyCard<float, 0> _routerApparentPower(dashboard, YASOLR_LBL_037, "VA");
dash::EnergyCard<float, 2> _routerPowerFactor(dashboard, YASOLR_LBL_038);
dash::EnergyCard<float, 2> _routerTHDi(dashboard, YASOLR_LBL_039, "%");
dash::EnergyCard<float, 0> _routerVoltage(dashboard, YASOLR_LBL_040, "V");
dash::EnergyCard<float, 2> _routerCurrent(dashboard, YASOLR_LBL_041, "A");
dash::EnergyCard<float, 2> _routerResistance(dashboard, YASOLR_LBL_042, "Ω");
dash::EnergyCard<float, 3> _routerEnergy(dashboard, YASOLR_LBL_043, "kWh");
dash::EnergyCard<float, 0> _gridPower(dashboard, YASOLR_LBL_044, "W");
dash::TemperatureCard<float, 2> _routerDS18State(dashboard, YASOLR_LBL_045);

#ifdef APP_MODEL_OSS
dash::SwitchCard _relay1Switch(dashboard, YASOLR_LBL_073);
dash::SwitchCard _relay2Switch(dashboard, YASOLR_LBL_076);

dash::FeedbackCard<const char*> _output1State(dashboard, YASOLR_LBL_046);
dash::TemperatureCard<float, 2> _output1DS18State(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_048);
dash::SliderCard<float, 2> _output1DimmerSlider(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
dash::SwitchCard _output1Bypass(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_051);

dash::FeedbackCard<const char*> _output2State(dashboard, YASOLR_LBL_070);
dash::TemperatureCard<float, 2> _output2DS18State(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_048);
dash::SliderCard<float, 2> _output2DimmerSlider(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
dash::SwitchCard _output2Bypass(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_051);
#endif

int16_t _gridPowerHistoryY[YASOLR_GRAPH_POINTS] = {0};
uint16_t _routedPowerHistoryY[YASOLR_GRAPH_POINTS] = {0};
uint8_t _routerTHDiHistoryY[YASOLR_GRAPH_POINTS] = {0};
dash::LineChart<int8_t, int16_t> _gridPowerHistory(dashboard, YASOLR_LBL_044 " (W)");
dash::AreaChart<int8_t, uint16_t> _routedPowerHistory(dashboard, YASOLR_LBL_036 " (W)");
dash::BarChart<int8_t, uint8_t> _routerTHDiHistory(dashboard, YASOLR_LBL_039 " (%)");

#ifdef APP_MODEL_OSS
dash::SwitchCard _output1PZEMSync(dashboard, YASOLR_LBL_147);
dash::SwitchCard _output2PZEMSync(dashboard, YASOLR_LBL_148);
dash::SwitchCard _resistanceCalibration(dashboard, YASOLR_LBL_186);
#endif

#ifdef APP_MODEL_PRO
// tabs icons:
// https://en.wikipedia.org/wiki/List_of_Unicode_characters#Miscellaneous_Symbols
// https://en.wikipedia.org/wiki/List_of_Unicode_characters#Dingbats

dash::Tab _output1Tab(dashboard, "\u2600 " YASOLR_LBL_046);
dash::FeedbackCard<const char*> _output1State(dashboard, YASOLR_LBL_047);
dash::TemperatureCard<float, 2> _output1DS18State(dashboard, YASOLR_LBL_048);
dash::SliderCard<float, 2> _output1DimmerSlider(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
dash::ProgressCard<float, 2> _output1DimmerSliderRO(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, "%");
dash::SwitchCard _output1Bypass(dashboard, YASOLR_LBL_051);
dash::FeedbackCard<const char*> _output1BypassRO(dashboard, YASOLR_LBL_051);
dash::EnergyCard<float, 0> _output1Power(dashboard, YASOLR_LBL_052, "W");
dash::EnergyCard<float, 0> _output1ApparentPower(dashboard, YASOLR_LBL_053, "VA");
dash::EnergyCard<float, 2> _output1PowerFactor(dashboard, YASOLR_LBL_054);
dash::EnergyCard<float, 2> _output1THDi(dashboard, YASOLR_LBL_055, "%");
dash::EnergyCard<float, 0> _output1Voltage(dashboard, YASOLR_LBL_056, "V");
dash::EnergyCard<float, 2> _output1Current(dashboard, YASOLR_LBL_057, "A");
dash::EnergyCard<float, 2> _output1Resistance(dashboard, YASOLR_LBL_058, "Ω");
dash::EnergyCard<float, 3> _output1Energy(dashboard, YASOLR_LBL_059, "kWh");
dash::PercentageSliderCard _output1DimmerDutyLimiter(dashboard, YASOLR_LBL_062);
dash::TextInputCard<uint8_t> _output1DimmerTempLimiter(dashboard, YASOLR_LBL_063);
dash::TextInputCard<uint16_t> _output1DimmerExcessLimiter(dashboard, YASOLR_LBL_061);
dash::SwitchCard _output1DimmerAuto(dashboard, YASOLR_LBL_060);
dash::SwitchCard _output1BypassAuto(dashboard, YASOLR_LBL_064);
dash::TextInputCard<uint8_t> _output1AutoStartTemp(dashboard, YASOLR_LBL_065);
dash::TextInputCard<uint8_t> _output1AutoStoptTemp(dashboard, YASOLR_LBL_066);
dash::TextInputCard<const char*> _output1AutoStartTime(dashboard, YASOLR_LBL_067);
dash::TextInputCard<const char*> _output1AutoStoptTime(dashboard, YASOLR_LBL_068);
dash::WeekCard<const char*> _output1AutoStartWDays(dashboard, YASOLR_LBL_069);

dash::Tab _output2Tab(dashboard, "\u2600 " YASOLR_LBL_070);
dash::FeedbackCard<const char*> _output2State(dashboard, YASOLR_LBL_047);
dash::TemperatureCard<float, 2> _output2DS18State(dashboard, YASOLR_LBL_048);
dash::SliderCard<float, 2> _output2DimmerSlider(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
dash::ProgressCard<float, 2> _output2DimmerSliderRO(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, "%");
dash::SwitchCard _output2Bypass(dashboard, YASOLR_LBL_051);
dash::FeedbackCard<const char*> _output2BypassRO(dashboard, YASOLR_LBL_051);
dash::EnergyCard<float, 0> _output2Power(dashboard, YASOLR_LBL_052, "W");
dash::EnergyCard<float, 0> _output2ApparentPower(dashboard, YASOLR_LBL_053, "VA");
dash::EnergyCard<float, 2> _output2PowerFactor(dashboard, YASOLR_LBL_054);
dash::EnergyCard<float, 2> _output2THDi(dashboard, YASOLR_LBL_055, "%");
dash::EnergyCard<float, 0> _output2Voltage(dashboard, YASOLR_LBL_056, "V");
dash::EnergyCard<float, 2> _output2Current(dashboard, YASOLR_LBL_057, "A");
dash::EnergyCard<float, 2> _output2Resistance(dashboard, YASOLR_LBL_058, "Ω");
dash::EnergyCard<float, 3> _output2Energy(dashboard, YASOLR_LBL_059, "kWh");
dash::PercentageSliderCard _output2DimmerDutyLimiter(dashboard, YASOLR_LBL_062);
dash::TextInputCard<uint8_t> _output2DimmerTempLimiter(dashboard, YASOLR_LBL_063);
dash::TextInputCard<uint16_t> _output2DimmerExcessLimiter(dashboard, YASOLR_LBL_061);
dash::SwitchCard _output2DimmerAuto(dashboard, YASOLR_LBL_060);
dash::SwitchCard _output2BypassAuto(dashboard, YASOLR_LBL_064);
dash::TextInputCard<uint8_t> _output2AutoStartTemp(dashboard, YASOLR_LBL_065);
dash::TextInputCard<uint8_t> _output2AutoStoptTemp(dashboard, YASOLR_LBL_066);
dash::TextInputCard<const char*> _output2AutoStartTime(dashboard, YASOLR_LBL_067);
dash::TextInputCard<const char*> _output2AutoStoptTime(dashboard, YASOLR_LBL_068);
dash::WeekCard<const char*> _output2AutoStartWDays(dashboard, YASOLR_LBL_069);

dash::Tab _relaysTab(dashboard, "\u2600 " YASOLR_LBL_071);
dash::SwitchCard _relay1Switch(dashboard, YASOLR_LBL_073);
dash::FeedbackCard<const char*> _relay1SwitchRO(dashboard, YASOLR_LBL_074);
dash::SwitchCard _relay2Switch(dashboard, YASOLR_LBL_076);
dash::FeedbackCard<const char*> _relay2SwitchRO(dashboard, YASOLR_LBL_077);

dash::Tab _managementTab(dashboard, "\u2764 " YASOLR_LBL_078);
dash::LinkCard<const char*> _configBackup(dashboard, YASOLR_LBL_079);
dash::FileUploadCard<const char*> _configRestore(dashboard, YASOLR_LBL_080, ".txt");
dash::PushButtonCard _restart(dashboard, YASOLR_LBL_082);
dash::PushButtonCard _safeBoot(dashboard, YASOLR_LBL_081);
dash::PushButtonCard _energyReset(dashboard, YASOLR_LBL_085);
dash::PushButtonCard _reset(dashboard, YASOLR_LBL_086);
dash::LinkCard<const char*> _consoleLink(dashboard, YASOLR_LBL_084);
dash::LinkCard<const char*> _debugInfo(dashboard, YASOLR_LBL_178);

dash::Tab _networkConfigTab(dashboard, "\u2728 " YASOLR_LBL_087);
dash::PasswordCard _adminPwd(dashboard, YASOLR_LBL_088, YASOLR_HIDDEN_PWD);
dash::TextInputCard<const char*> _ntpServer(dashboard, YASOLR_LBL_089);
dash::AsyncDropdownCard<const char*> _ntpTimezone(dashboard, YASOLR_LBL_090, "/timezones");
dash::TimeSyncCard _ntpSync(dashboard, YASOLR_LBL_091);
dash::TextInputCard<const char*> _wifiSSID(dashboard, YASOLR_LBL_092);
dash::PasswordCard _wifiPwd(dashboard, YASOLR_LBL_093, YASOLR_HIDDEN_PWD);
dash::TextInputCard<const char*> _staticIP(dashboard, YASOLR_LBL_188);
dash::TextInputCard<const char*> _subnetMask(dashboard, YASOLR_LBL_189);
dash::TextInputCard<const char*> _gateway(dashboard, YASOLR_LBL_190);
dash::TextInputCard<const char*> _dnsServer(dashboard, YASOLR_LBL_191);
dash::SwitchCard _apMode(dashboard, YASOLR_LBL_094);

dash::Tab _mqttConfigTab(dashboard, "\u2728 " YASOLR_LBL_095);
dash::TextInputCard<const char*> _mqttServer(dashboard, YASOLR_LBL_096);
dash::TextInputCard<uint16_t> _mqttPort(dashboard, YASOLR_LBL_097);
dash::TextInputCard<const char*> _mqttUser(dashboard, YASOLR_LBL_098);
dash::PasswordCard _mqttPwd(dashboard, YASOLR_LBL_099, YASOLR_HIDDEN_PWD);
dash::SwitchCard _mqttSecured(dashboard, YASOLR_LBL_100);
dash::FileUploadCard _mqttServerCert(dashboard, YASOLR_LBL_101, ".pem");
dash::PushButtonCard _mqttServerCertDelete(dashboard, YASOLR_LBL_049);
dash::SliderCard<uint8_t> _mqttPublishInterval(dashboard, YASOLR_LBL_102, 1, 30, 1, "s");
dash::TextInputCard<const char*> _mqttTopic(dashboard, YASOLR_LBL_103);
dash::SwitchCard _haDiscovery(dashboard, YASOLR_LBL_104);
dash::TextInputCard<const char*> _haDiscoveryTopic(dashboard, YASOLR_LBL_105);
dash::TextInputCard<const char*> _mqttGridVoltage(dashboard, YASOLR_LBL_106);
dash::TextInputCard<const char*> _mqttGridPower(dashboard, YASOLR_LBL_107);
dash::TextInputCard<const char*> _mqttTempO1(dashboard, YASOLR_LBL_181);
dash::TextInputCard<const char*> _mqttTempO2(dashboard, YASOLR_LBL_182);

dash::Tab _pinConfigTab(dashboard, "\u21C6 " YASOLR_LBL_108);
dash::FeedbackTextInputCard<int32_t> _pinDisplayClock(dashboard, YASOLR_LBL_111);
dash::FeedbackTextInputCard<int32_t> _pinDisplayData(dashboard, YASOLR_LBL_112);
dash::FeedbackTextInputCard<int32_t> _pinJsyRX(dashboard, YASOLR_LBL_116);
dash::FeedbackTextInputCard<int32_t> _pinJsyTX(dashboard, YASOLR_LBL_117);
dash::FeedbackTextInputCard<int32_t> _pinLEDGreen(dashboard, YASOLR_LBL_118);
dash::FeedbackTextInputCard<int32_t> _pinLEDRed(dashboard, YASOLR_LBL_119);
dash::FeedbackTextInputCard<int32_t> _pinLEDYellow(dashboard, YASOLR_LBL_120);
dash::FeedbackTextInputCard<int32_t> _pinDimmerO1(dashboard, YASOLR_LBL_131);
dash::FeedbackTextInputCard<int32_t> _pinDS18O1(dashboard, YASOLR_LBL_132);
dash::FeedbackTextInputCard<int32_t> _pinRelayO1(dashboard, YASOLR_LBL_134);
dash::FeedbackTextInputCard<int32_t> _pinDimmerO2(dashboard, YASOLR_LBL_135);
dash::FeedbackTextInputCard<int32_t> _pinDS18O2(dashboard, YASOLR_LBL_136);
dash::FeedbackTextInputCard<int32_t> _pinRelayO2(dashboard, YASOLR_LBL_138);
dash::FeedbackTextInputCard<int32_t> _pinPZEMRX(dashboard, YASOLR_LBL_121);
dash::FeedbackTextInputCard<int32_t> _pinPZEMTX(dashboard, YASOLR_LBL_122);
dash::FeedbackTextInputCard<int32_t> _pinRelay1(dashboard, YASOLR_LBL_074);
dash::FeedbackTextInputCard<int32_t> _pinRelay2(dashboard, YASOLR_LBL_077);
dash::FeedbackTextInputCard<int32_t> _pinDS18Router(dashboard, YASOLR_LBL_139);
dash::FeedbackTextInputCard<int32_t> _pinZCD(dashboard, YASOLR_LBL_125);

dash::Tab _hardwareEnableTab(dashboard, "\u2699 " YASOLR_LBL_126);
dash::FeedbackSwitchCard _debugMode(dashboard, YASOLR_LBL_083);
dash::FeedbackSwitchCard _display(dashboard, YASOLR_LBL_127);
dash::FeedbackSwitchCard _jsy(dashboard, YASOLR_LBL_128);
dash::FeedbackSwitchCard _jsyRemote(dashboard, YASOLR_LBL_187);
dash::FeedbackSwitchCard _led(dashboard, YASOLR_LBL_129);
dash::FeedbackSwitchCard _mqtt(dashboard, YASOLR_LBL_095);
dash::FeedbackSwitchCard _output1Dimmer(dashboard, YASOLR_LBL_131);
dash::FeedbackSwitchCard _output1DS18(dashboard, YASOLR_LBL_132);
dash::FeedbackSwitchCard _output1PZEM(dashboard, YASOLR_LBL_133);
dash::FeedbackSwitchCard _output1Relay(dashboard, YASOLR_LBL_134);
dash::FeedbackSwitchCard _output2Dimmer(dashboard, YASOLR_LBL_135);
dash::FeedbackSwitchCard _output2DS18(dashboard, YASOLR_LBL_136);
dash::FeedbackSwitchCard _output2PZEM(dashboard, YASOLR_LBL_137);
dash::FeedbackSwitchCard _output2Relay(dashboard, YASOLR_LBL_138);
dash::FeedbackSwitchCard _relay1(dashboard, YASOLR_LBL_074);
dash::FeedbackSwitchCard _relay2(dashboard, YASOLR_LBL_077);
dash::FeedbackSwitchCard _routerDS18(dashboard, YASOLR_LBL_139);
dash::FeedbackSwitchCard _zcd(dashboard, YASOLR_LBL_125);

dash::Tab _hardwareConfigTab(dashboard, "\u2699 " YASOLR_LBL_140);
dash::DropdownCard<const char*> _gridFreq(dashboard, YASOLR_LBL_141, "Auto-detect,50 Hz,60 Hz");
dash::SliderCard<uint8_t> _displaySpeed(dashboard, YASOLR_LBL_142, 1, 10, 1, "s");
dash::DropdownCard<const char*> _displayType(dashboard, YASOLR_LBL_143, "SH1106,SH1107,SSD1306");
dash::DropdownCard<uint16_t> _displayRotation(dashboard, YASOLR_LBL_144, "0,90,180,270");
dash::DropdownCard<const char*> _relay1Type(dashboard, YASOLR_LBL_151, "NO,NC");
dash::DropdownCard<const char*> _relay2Type(dashboard, YASOLR_LBL_152, "NO,NC");
dash::TextInputCard<uint16_t> _relay1Load(dashboard, YASOLR_LBL_072);
dash::TextInputCard<uint16_t> _relay2Load(dashboard, YASOLR_LBL_075);
dash::DropdownCard<const char*> _output1RelayType(dashboard, YASOLR_LBL_149, "NO,NC");
dash::DropdownCard<const char*> _output2RelayType(dashboard, YASOLR_LBL_150, "NO,NC");
dash::RangeSliderCard<uint8_t> _output1DimmerMapper(dashboard, YASOLR_LBL_183, 0, 100, 1, "%");
dash::RangeSliderCard<uint8_t> _output2DimmerMapper(dashboard, YASOLR_LBL_184, 0, 100, 1, "%");
dash::SwitchCard _output1PZEMSync(dashboard, YASOLR_LBL_147);
dash::SwitchCard _output2PZEMSync(dashboard, YASOLR_LBL_148);
dash::FeedbackTextInputCard<float, 2> _output1ResistanceInput(dashboard, YASOLR_LBL_145);
dash::FeedbackTextInputCard<float, 2> _output2ResistanceInput(dashboard, YASOLR_LBL_146);
dash::SwitchCard _resistanceCalibration(dashboard, YASOLR_LBL_186);

dash::Tab _pidTab(dashboard, "\u2699 " YASOLR_LBL_159);
dash::SwitchCard _pidView(dashboard, YASOLR_LBL_169);
dash::DropdownCard<const char*> _pidPMode(dashboard, YASOLR_LBL_160, YASOLR_PID_P_MODE_1 "," YASOLR_PID_P_MODE_2 "," YASOLR_PID_P_MODE_3);
dash::DropdownCard<const char*> _pidDMode(dashboard, YASOLR_LBL_161, YASOLR_PID_D_MODE_1 "," YASOLR_PID_D_MODE_2 "," YASOLR_PID_D_MODE_3);
dash::DropdownCard<const char*> _pidICMode(dashboard, YASOLR_LBL_162, YASOLR_PID_IC_MODE_0 "," YASOLR_PID_IC_MODE_1 "," YASOLR_PID_IC_MODE_2);
dash::TextInputCard<int> _pidSetpoint(dashboard, YASOLR_LBL_163);
dash::TextInputCard<float, 4> _pidKp(dashboard, YASOLR_LBL_166);
dash::TextInputCard<float, 4> _pidKi(dashboard, YASOLR_LBL_167);
dash::TextInputCard<float, 4> _pidKd(dashboard, YASOLR_LBL_168);
dash::TextInputCard<int> _pidOutMin(dashboard, YASOLR_LBL_164);
dash::TextInputCard<int> _pidOutMax(dashboard, YASOLR_LBL_165);

// input,output,error,pTerm,iTerm,dTerm,sum
int16_t _pidInputHistoryY[YASOLR_GRAPH_POINTS] = {0};
int16_t _pidOutputHistoryY[YASOLR_GRAPH_POINTS] = {0};
int16_t _pidErrorHistoryY[YASOLR_GRAPH_POINTS] = {0};
int16_t _pidPTermHistoryY[YASOLR_GRAPH_POINTS] = {0};
int16_t _pidITermHistoryY[YASOLR_GRAPH_POINTS] = {0};
int16_t _pidDTermHistoryY[YASOLR_GRAPH_POINTS] = {0};
int16_t _pidSumHistoryY[YASOLR_GRAPH_POINTS] = {0};
dash::LineChart<int8_t, int16_t> _pidInputHistory(dashboard, YASOLR_LBL_170);
dash::LineChart<int8_t, int16_t> _pidOutputHistory(dashboard, YASOLR_LBL_171);
dash::LineChart<int8_t, int16_t> _pidErrorHistory(dashboard, YASOLR_LBL_172);
dash::BarChart<int8_t, int16_t> _pidSumHistory(dashboard, YASOLR_LBL_173);
dash::LineChart<int8_t, int16_t> _pidPTermHistory(dashboard, YASOLR_LBL_174);
dash::LineChart<int8_t, int16_t> _pidITermHistory(dashboard, YASOLR_LBL_175);
dash::LineChart<int8_t, int16_t> _pidDTermHistory(dashboard, YASOLR_LBL_176);
#endif

void YaSolR::Website::begin() {
  logger.info(TAG, "Initialize dashboard layout...");

  for (int i = 0; i < YASOLR_GRAPH_POINTS; i++)
    _historyX[i] = i - YASOLR_GRAPH_POINTS;

  // overview

  _gridPowerHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.setX(_historyX, YASOLR_GRAPH_POINTS);

  if (output1) {
    _outputBypassSwitch(_output1Bypass, *output1);
    _outputDimmerSlider(_output1DimmerSlider, *output1);
  }

  if (output2) {
    _outputBypassSwitch(_output2Bypass, *output2);
    _outputDimmerSlider(_output2DimmerSlider, *output2);
  }

  if (relay1)
    _relaySwitch(_relay1Switch, *relay1);
  if (relay2)
    _relaySwitch(_relay2Switch, *relay2);

  _output1PZEMSync.onChange([](bool value) {
    pzemO1PairingTask->resume();
    _output1PZEMSync.setValue(!pzemO1PairingTask->isPaused());
    dashboard.refresh(_output1PZEMSync);
  });

  _output2PZEMSync.onChange([](bool value) {
    pzemO2PairingTask->resume();
    _output2PZEMSync.setValue(!pzemO2PairingTask->isPaused());
    dashboard.refresh(_output2PZEMSync);
  });

  _resistanceCalibration.onChange([this](bool value) {
    if (value && !router.isCalibrationRunning()) {
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

      // because we set false to trigger events
      dashboardInitTask.resume();
      if (mqttPublishConfigTask)
        mqttPublishConfigTask->resume();
      if (mqttPublishTask)
        mqttPublishTask->requestEarlyRun();
    }

    _resistanceCalibration.setValue(router.isCalibrationRunning());
    dashboard.refresh(_resistanceCalibration);
  });

#ifdef APP_MODEL_PRO
  dashboard.setChartAnimations(false);

  // overview graphs

  _gridPowerHistory.setSize(chartSize);
  _routedPowerHistory.setSize(chartSize);
  _routerTHDiHistory.setSize(chartSize);

  // PID

  _pidInputHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidOutputHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidErrorHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidPTermHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidITermHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidDTermHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidSumHistory.setX(_historyX, YASOLR_GRAPH_POINTS);

  _pidInputHistory.setSize(chartSize);
  _pidOutputHistory.setSize(chartSize);
  _pidPTermHistory.setSize(chartSize);
  _pidITermHistory.setSize(chartSize);
  _pidDTermHistory.setSize(chartSize);
  _pidErrorHistory.setSize(chartSize);
  _pidSumHistory.setSize(chartSize);

  // output 1

  _output1State.setTab(_output1Tab);
  _output1DS18State.setTab(_output1Tab);
  _output1DimmerSlider.setTab(_output1Tab);
  _output1DimmerSliderRO.setTab(_output1Tab);
  _output1Bypass.setTab(_output1Tab);
  _output1BypassRO.setTab(_output1Tab);

  _output1Power.setTab(_output1Tab);
  _output1PowerFactor.setTab(_output1Tab);
  _output1THDi.setTab(_output1Tab);
  _output1Energy.setTab(_output1Tab);

  _output1ApparentPower.setTab(_output1Tab);
  _output1Voltage.setTab(_output1Tab);
  _output1Current.setTab(_output1Tab);
  _output1Resistance.setTab(_output1Tab);

  _output1BypassAuto.setTab(_output1Tab);
  _output1DimmerAuto.setTab(_output1Tab);
  _output1AutoStartTemp.setTab(_output1Tab);
  _output1AutoStartTime.setTab(_output1Tab);
  _output1AutoStartWDays.setTab(_output1Tab);
  _output1AutoStoptTemp.setTab(_output1Tab);
  _output1AutoStoptTime.setTab(_output1Tab);
  _output1DimmerExcessLimiter.setTab(_output1Tab);
  _output1DimmerDutyLimiter.setTab(_output1Tab);
  _output1DimmerTempLimiter.setTab(_output1Tab);

  _boolConfig(_output1BypassAuto, KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  _boolConfig(_output1DimmerAuto, KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  _daysConfig(_output1AutoStartWDays, KEY_OUTPUT1_DAYS);
  _numConfig(_output1AutoStartTemp, KEY_OUTPUT1_TEMPERATURE_START);
  _numConfig(_output1AutoStoptTemp, KEY_OUTPUT1_TEMPERATURE_STOP);
  _numConfig(_output1DimmerTempLimiter, KEY_OUTPUT1_DIMMER_TEMP_LIMITER);
  _numConfig(_output1DimmerExcessLimiter, KEY_OUTPUT1_EXCESS_LIMITER);
  _sliderConfig(_output1DimmerDutyLimiter, KEY_OUTPUT1_DIMMER_LIMIT);
  _textConfig(_output1AutoStartTime, KEY_OUTPUT1_TIME_START);
  _textConfig(_output1AutoStoptTime, KEY_OUTPUT1_TIME_STOP);

  // output 2

  _output2State.setTab(_output2Tab);
  _output2DS18State.setTab(_output2Tab);
  _output2DimmerSlider.setTab(_output2Tab);
  _output2DimmerSliderRO.setTab(_output2Tab);
  _output2Bypass.setTab(_output2Tab);
  _output2BypassRO.setTab(_output2Tab);

  _output2Power.setTab(_output2Tab);
  _output2PowerFactor.setTab(_output2Tab);
  _output2THDi.setTab(_output2Tab);
  _output2Energy.setTab(_output2Tab);

  _output2ApparentPower.setTab(_output2Tab);
  _output2Voltage.setTab(_output2Tab);
  _output2Current.setTab(_output2Tab);
  _output2Resistance.setTab(_output2Tab);

  _output2BypassAuto.setTab(_output2Tab);
  _output2DimmerAuto.setTab(_output2Tab);
  _output2AutoStartTemp.setTab(_output2Tab);
  _output2AutoStartTime.setTab(_output2Tab);
  _output2AutoStartWDays.setTab(_output2Tab);
  _output2AutoStoptTemp.setTab(_output2Tab);
  _output2AutoStoptTime.setTab(_output2Tab);
  _output2DimmerExcessLimiter.setTab(_output2Tab);
  _output2DimmerDutyLimiter.setTab(_output2Tab);
  _output2DimmerTempLimiter.setTab(_output2Tab);

  _boolConfig(_output2BypassAuto, KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  _boolConfig(_output2DimmerAuto, KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  _daysConfig(_output2AutoStartWDays, KEY_OUTPUT2_DAYS);
  _numConfig(_output2AutoStartTemp, KEY_OUTPUT2_TEMPERATURE_START);
  _numConfig(_output2AutoStoptTemp, KEY_OUTPUT2_TEMPERATURE_STOP);
  _numConfig(_output2DimmerTempLimiter, KEY_OUTPUT2_DIMMER_TEMP_LIMITER);
  _numConfig(_output2DimmerExcessLimiter, KEY_OUTPUT2_EXCESS_LIMITER);
  _sliderConfig(_output2DimmerDutyLimiter, KEY_OUTPUT2_DIMMER_LIMIT);
  _textConfig(_output2AutoStartTime, KEY_OUTPUT2_TIME_START);
  _textConfig(_output2AutoStoptTime, KEY_OUTPUT2_TIME_STOP);

  // relays

  _relay1Switch.setTab(_relaysTab);
  _relay1SwitchRO.setTab(_relaysTab);
  _relay2Switch.setTab(_relaysTab);
  _relay2SwitchRO.setTab(_relaysTab);

  // management

  _configBackup.setTab(_managementTab);
  _configRestore.setTab(_managementTab);
  _consoleLink.setTab(_managementTab);
  _debugInfo.setTab(_managementTab);
  _safeBoot.setTab(_managementTab);
  _reset.setTab(_managementTab);
  _restart.setTab(_managementTab);
  _energyReset.setTab(_managementTab);

  _energyReset.onPush([]() {
    if (jsy)
      jsy->resetEnergy();
    if (pzemO1)
      pzemO1->resetEnergy();
    if (pzemO2)
      pzemO2->resetEnergy();
  });
  _reset.onPush([]() { resetTask.resume(); });
  _restart.onPush([]() { restartTask.resume(); });
  _safeBoot.onPush([]() { safeBootTask.resume(); });

  // network (config)

  _adminPwd.setTab(_networkConfigTab);
  _apMode.setTab(_networkConfigTab);
  _ntpServer.setTab(_networkConfigTab);
  _ntpSync.setTab(_networkConfigTab);
  _ntpTimezone.setTab(_networkConfigTab);
  _wifiPwd.setTab(_networkConfigTab);
  _wifiSSID.setTab(_networkConfigTab);
  _staticIP.setTab(_networkConfigTab);
  _subnetMask.setTab(_networkConfigTab);
  _gateway.setTab(_networkConfigTab);
  _dnsServer.setTab(_networkConfigTab);

  _ntpTimezone.onChange([](const char* value) {
    config.set(KEY_NTP_TIMEZONE, value);
    _ntpTimezone.setValue(value);
    dashboard.refresh(_ntpTimezone);
  });

  _boolConfig(_apMode, KEY_ENABLE_AP_MODE);
  _passwordConfig(_adminPwd, KEY_ADMIN_PASSWORD);
  _passwordConfig(_wifiPwd, KEY_WIFI_PASSWORD);
  _textConfig(_ntpServer, KEY_NTP_SERVER);
  _textConfig(_wifiSSID, KEY_WIFI_SSID);
  _textConfig(_staticIP, KEY_NET_IP);
  _textConfig(_subnetMask, KEY_NET_SUBNET);
  _textConfig(_gateway, KEY_NET_GATEWAY);
  _textConfig(_dnsServer, KEY_NET_DNS);

  _ntpSync.onSync([](const timeval& tv) {
    Mycila::NTP.sync(tv);
  });

  // mqtt (config)

  _haDiscovery.setTab(_mqttConfigTab);
  _haDiscoveryTopic.setTab(_mqttConfigTab);
  _mqttGridPower.setTab(_mqttConfigTab);
  _mqttGridVoltage.setTab(_mqttConfigTab);
  _mqttPort.setTab(_mqttConfigTab);
  _mqttPublishInterval.setTab(_mqttConfigTab);
  _mqttPwd.setTab(_mqttConfigTab);
  _mqttSecured.setTab(_mqttConfigTab);
  _mqttServer.setTab(_mqttConfigTab);
  _mqttServerCert.setTab(_mqttConfigTab);
  _mqttServerCertDelete.setTab(_mqttConfigTab);
  _mqttTopic.setTab(_mqttConfigTab);
  _mqttUser.setTab(_mqttConfigTab);
  _mqttTempO1.setTab(_mqttConfigTab);
  _mqttTempO2.setTab(_mqttConfigTab);

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

  _mqttServerCertDelete.onPush([this]() {
    if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE) && LittleFS.remove(YASOLR_MQTT_SERVER_CERT_FILE)) {
      logger.warn(TAG, "MQTT server certificate deleted successfully!");
      dashboardInitTask.resume();
    }
  });

  // GPIO (configuration)

  _pinDimmerO1.setTab(_pinConfigTab);
  _pinDimmerO2.setTab(_pinConfigTab);
  _pinDisplayClock.setTab(_pinConfigTab);
  _pinDisplayData.setTab(_pinConfigTab);
  _pinDS18O1.setTab(_pinConfigTab);
  _pinDS18O2.setTab(_pinConfigTab);
  _pinDS18Router.setTab(_pinConfigTab);
  _pinJsyRX.setTab(_pinConfigTab);
  _pinJsyTX.setTab(_pinConfigTab);
  _pinLEDGreen.setTab(_pinConfigTab);
  _pinLEDRed.setTab(_pinConfigTab);
  _pinLEDYellow.setTab(_pinConfigTab);
  _pinPZEMRX.setTab(_pinConfigTab);
  _pinPZEMTX.setTab(_pinConfigTab);
  _pinRelay1.setTab(_pinConfigTab);
  _pinRelay2.setTab(_pinConfigTab);
  _pinRelayO1.setTab(_pinConfigTab);
  _pinRelayO2.setTab(_pinConfigTab);
  _pinZCD.setTab(_pinConfigTab);

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

  _debugMode.setTab(_hardwareEnableTab);
  _display.setTab(_hardwareEnableTab);
  _jsy.setTab(_hardwareEnableTab);
  _jsyRemote.setTab(_hardwareEnableTab);
  _led.setTab(_hardwareEnableTab);
  _mqtt.setTab(_hardwareEnableTab);
  _output1Dimmer.setTab(_hardwareEnableTab);
  _output1PZEM.setTab(_hardwareEnableTab);
  _output1Relay.setTab(_hardwareEnableTab);
  _output1DS18.setTab(_hardwareEnableTab);
  _output2Dimmer.setTab(_hardwareEnableTab);
  _output2PZEM.setTab(_hardwareEnableTab);
  _output2Relay.setTab(_hardwareEnableTab);
  _output2DS18.setTab(_hardwareEnableTab);
  _relay1.setTab(_hardwareEnableTab);
  _relay2.setTab(_hardwareEnableTab);
  _routerDS18.setTab(_hardwareEnableTab);
  _zcd.setTab(_hardwareEnableTab);

  _boolConfig(_debugMode, KEY_ENABLE_DEBUG);
  _boolConfig(_display, KEY_ENABLE_DISPLAY);
  _boolConfig(_jsy, KEY_ENABLE_JSY);
  _boolConfig(_jsyRemote, KEY_ENABLE_JSY_REMOTE);
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

  _gridFreq.setTab(_hardwareConfigTab);
  _displayRotation.setTab(_hardwareConfigTab);
  _displayType.setTab(_hardwareConfigTab);
  _displaySpeed.setTab(_hardwareConfigTab);
  _output1PZEMSync.setTab(_hardwareConfigTab);
  _output2PZEMSync.setTab(_hardwareConfigTab);
  _output1RelayType.setTab(_hardwareConfigTab);
  _output2RelayType.setTab(_hardwareConfigTab);
  _relay1Type.setTab(_hardwareConfigTab);
  _relay2Type.setTab(_hardwareConfigTab);
  _relay1Load.setTab(_hardwareConfigTab);
  _relay2Load.setTab(_hardwareConfigTab);
  _output1ResistanceInput.setTab(_hardwareConfigTab);
  _output2ResistanceInput.setTab(_hardwareConfigTab);
  _output1DimmerMapper.setTab(_hardwareConfigTab);
  _output2DimmerMapper.setTab(_hardwareConfigTab);
  _resistanceCalibration.setTab(_hardwareConfigTab);

  _gridFreq.onChange([](const char* value) {
    if (strcmp(value, "50 Hz") == 0)
      config.set(KEY_GRID_FREQUENCY, "50");
    else if (strcmp(value, "60 Hz") == 0)
      config.set(KEY_GRID_FREQUENCY, "60");
    else
      config.unset(KEY_GRID_FREQUENCY);
    _gridFreq.setValue(config.get(KEY_GRID_FREQUENCY));
    dashboard.refresh(_gridFreq);
  });

  _numConfig(_displayRotation, KEY_DISPLAY_ROTATION);
  _numConfig(_relay1Load, KEY_RELAY1_LOAD);
  _numConfig(_relay2Load, KEY_RELAY2_LOAD);
  _textConfig(_displayType, KEY_DISPLAY_TYPE);
  _textConfig(_output1RelayType, KEY_OUTPUT1_RELAY_TYPE);
  _textConfig(_output2RelayType, KEY_OUTPUT2_RELAY_TYPE);
  _textConfig(_relay1Type, KEY_RELAY1_TYPE);
  _textConfig(_relay2Type, KEY_RELAY2_TYPE);
  _sliderConfig(_displaySpeed, KEY_DISPLAY_SPEED);

  _output1ResistanceInput.onChange([](const std::optional<float> value) {
    if (value.has_value())
      config.set(KEY_OUTPUT1_RESISTANCE, dash::to_string<float, 2>(value.value()));
    else
      config.unset(KEY_OUTPUT1_RESISTANCE);
    _output1ResistanceInput.setValue(config.getFloat(KEY_OUTPUT1_RESISTANCE));
    dashboard.refresh(_output1ResistanceInput);
  });
  _output2ResistanceInput.onChange([](const std::optional<float> value) {
    if (value.has_value())
      config.set(KEY_OUTPUT2_RESISTANCE, dash::to_string<float, 2>(value.value()));
    else
      config.unset(KEY_OUTPUT2_RESISTANCE);
    _output2ResistanceInput.setValue(config.getFloat(KEY_OUTPUT2_RESISTANCE));
    dashboard.refresh(_output2ResistanceInput);
  });

  _output1DimmerMapper.onChange([](const dash::Range<uint8_t>& range) {
    config.set(KEY_OUTPUT1_DIMMER_MIN, std::to_string(range.low()));
    config.set(KEY_OUTPUT1_DIMMER_MAX, std::to_string(range.high()));
    _output1DimmerMapper.setValue({static_cast<uint8_t>(config.getInt(KEY_OUTPUT1_DIMMER_MIN)),
                                   static_cast<uint8_t>(config.getInt(KEY_OUTPUT1_DIMMER_MAX))});
    dashboard.refresh(_output1DimmerMapper);
  });

  _output2DimmerMapper.onChange([](const dash::Range<uint8_t>& range) {
    config.set(KEY_OUTPUT2_DIMMER_MIN, std::to_string(range.low()));
    config.set(KEY_OUTPUT2_DIMMER_MAX, std::to_string(range.high()));
    _output2DimmerMapper.setValue({static_cast<uint8_t>(config.getInt(KEY_OUTPUT2_DIMMER_MIN)),
                                   static_cast<uint8_t>(config.getInt(KEY_OUTPUT2_DIMMER_MAX))});
    dashboard.refresh(_output2DimmerMapper);
  });

  // PID

  _pidView.setTab(_pidTab);
  _pidPMode.setTab(_pidTab);
  _pidDMode.setTab(_pidTab);
  _pidICMode.setTab(_pidTab);
  _pidSetpoint.setTab(_pidTab);
  _pidKp.setTab(_pidTab);
  _pidKi.setTab(_pidTab);
  _pidKd.setTab(_pidTab);
  _pidOutMin.setTab(_pidTab);
  _pidOutMax.setTab(_pidTab);

  _pidInputHistory.setTab(_pidTab);
  _pidOutputHistory.setTab(_pidTab);
  _pidErrorHistory.setTab(_pidTab);
  _pidSumHistory.setTab(_pidTab);
  _pidPTermHistory.setTab(_pidTab);
  _pidITermHistory.setTab(_pidTab);
  _pidDTermHistory.setTab(_pidTab);

  _pidPMode.onChange([](const char* value) {
    if (strcmp(value, YASOLR_PID_P_MODE_1) == 0)
      config.set(KEY_PID_P_MODE, "1");
    else if (strcmp(value, YASOLR_PID_P_MODE_2) == 0)
      config.set(KEY_PID_P_MODE, "2");
    else if (strcmp(value, YASOLR_PID_P_MODE_3) == 0)
      config.set(KEY_PID_P_MODE, "3");
    else
      config.unset(KEY_PID_P_MODE);
    _pidPMode.setValue(config.get(KEY_PID_P_MODE));
    dashboard.refresh(_pidPMode);
  });

  _pidDMode.onChange([](const char* value) {
    if (strcmp(value, YASOLR_PID_D_MODE_1) == 0)
      config.set(KEY_PID_D_MODE, "1");
    else if (strcmp(value, YASOLR_PID_D_MODE_2) == 0)
      config.set(KEY_PID_D_MODE, "2");
    else if (strcmp(value, YASOLR_PID_D_MODE_3) == 0)
      config.set(KEY_PID_D_MODE, "3");
    else
      config.unset(KEY_PID_D_MODE);
    _pidDMode.setValue(config.get(KEY_PID_D_MODE));
    dashboard.refresh(_pidDMode);
  });

  _pidICMode.onChange([](const char* value) {
    if (strcmp(value, YASOLR_PID_IC_MODE_0) == 0)
      config.set(KEY_PID_IC_MODE, "0");
    else if (strcmp(value, YASOLR_PID_IC_MODE_1) == 0)
      config.set(KEY_PID_IC_MODE, "1");
    else if (strcmp(value, YASOLR_PID_IC_MODE_2) == 0)
      config.set(KEY_PID_IC_MODE, "2");
    else
      config.unset(KEY_PID_IC_MODE);
    _pidICMode.setValue(config.get(KEY_PID_IC_MODE));
    dashboard.refresh(_pidICMode);
  });

  _numConfig(_pidSetpoint, KEY_PID_SETPOINT);
  _numConfig(_pidOutMin, KEY_PID_OUT_MIN);
  _numConfig(_pidOutMax, KEY_PID_OUT_MAX);

  _pidView.onChange([this](bool value) {
    _pidView.setValue(value);
    if (value)
      resetPID();
    dashboard.refresh(_pidView);
    dashboardInitTask.resume();
  });

  _pidKp.onChange([](const std::optional<float> value) {
    if (value.has_value())
      config.set(KEY_PID_KP, dash::to_string<float, 4>(value.value()));
    else
      config.unset(KEY_PID_KP);
    _pidKp.setValue(config.getFloat(KEY_PID_KP));
    dashboard.refresh(_pidKp);
  });
  _pidKi.onChange([](const std::optional<float> value) {
    if (value.has_value())
      config.set(KEY_PID_KI, dash::to_string<float, 4>(value.value()));
    else
      config.unset(KEY_PID_KI);
    _pidKi.setValue(config.getFloat(KEY_PID_KI));
    dashboard.refresh(_pidKi);
  });
  _pidKd.onChange([](const std::optional<float> value) {
    if (value.has_value())
      config.set(KEY_PID_KD, dash::to_string<float, 4>(value.value()));
    else
      config.unset(KEY_PID_KD);
    _pidKd.setValue(config.getFloat(KEY_PID_KD));
    dashboard.refresh(_pidKd);
  });
#endif

  initCards();
  updateCards();
}

void YaSolR::Website::initCards() {
  logger.debug(TAG, "Initialize dashboard cards...");

  // Statistics

  Mycila::ESPConnect::Mode mode = espConnect.getMode();
  _appManufacturer.setValue(Mycila::AppInfo.manufacturer.c_str());
  _appModel.setValue(Mycila::AppInfo.model.c_str());
  _appName.setValue(Mycila::AppInfo.name.c_str());
  _appVersion.setValue(Mycila::AppInfo.version.c_str());
  _deviceBootCount.setValue(Mycila::System::getBootCount());
  _deviceBootReason.setValue(Mycila::System::getLastRebootReason());
  _deviceCores.setValue(ESP.getChipCores());
  _deviceID.setValue(Mycila::AppInfo.id.c_str());
  _deviceModel.setValue(ESP.getChipModel());
  _deviceRev.setValue(ESP.getChipRevision());
  _firmwareBuildHash.setValue(Mycila::AppInfo.buildHash.c_str());
  _firmwareBuildTimestamp.setValue(Mycila::AppInfo.buildDate.c_str());
  _firmwareFilename.setValue(Mycila::AppInfo.firmware.c_str());
  _networkAPIP.setValue(espConnect.getIPAddress(Mycila::ESPConnect::Mode::AP).toString().c_str());
  _networkAPMAC.setValue(espConnect.getMACAddress(Mycila::ESPConnect::Mode::AP));
  _networkEthIP.setValue(espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  _networkEthMAC.setValue(espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH).empty() ? std::string("N/A") : espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH));
  _networkHostname.setValue(Mycila::AppInfo.defaultHostname.c_str());
  _networkInterface.setValue(mode == Mycila::ESPConnect::Mode::AP ? "AP" : (mode == Mycila::ESPConnect::Mode::STA ? "WiFi" : (mode == Mycila::ESPConnect::Mode::ETH ? "Ethernet" : "")));
  _networkWiFiIP.setValue(espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  _networkWiFiMAC.setValue(espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA));
  _networkWiFiSSID.setValue(espConnect.getWiFiSSID());

  if (!output1) {
    _output1State.setFeedback("DISABLED", dash::Status::IDLE);
    _output1DS18State.setValue(NAN);
    _output1DimmerSlider.setValue(0);
    _output1Bypass.setValue(false);
  }

  if (!output2) {
    _output2State.setFeedback("DISABLED", dash::Status::IDLE);
    _output2DS18State.setValue(NAN);
    _output2DimmerSlider.setValue(0);
    _output2Bypass.setValue(false);
  }

#ifdef APP_MODEL_PRO
  const bool jsyEnabled = config.getBool(KEY_ENABLE_JSY) && jsy && jsy->isEnabled();

  // output 1

  const bool dimmer1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_DIMMER) && output1 && output1->isDimmerOnline();
  const bool output1RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT1_RELAY) && output1 && output1->isBypassRelayEnabled();
  const bool bypass1Possible = dimmer1Enabled || output1RelayEnabled;
  const bool autoDimmer1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  const bool autoBypass1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  const bool output1TempEnabled = (config.getBool(KEY_ENABLE_OUTPUT1_DS18) && ds18O1 && ds18O1->isEnabled()) || !config.isEmpty(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  const bool pzem1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_PZEM) && pzemO1 && pzemO1->isEnabled();
  const char* output1Days = config.get(KEY_OUTPUT1_DAYS);

  _output1DimmerAuto.setValue(autoDimmer1Activated);
  _output1DimmerExcessLimiter.setValue(config.getInt(KEY_OUTPUT1_EXCESS_LIMITER));
  _output1DimmerDutyLimiter.setValue(config.getInt(KEY_OUTPUT1_DIMMER_LIMIT));
  _output1DimmerTempLimiter.setValue(config.getInt(KEY_OUTPUT1_DIMMER_TEMP_LIMITER));
  _output1BypassAuto.setValue(autoBypass1Activated);
  _output1AutoStartWDays.setValue(strcmp(output1Days, YASOLR_WEEK_DAYS_EMPTY) == 0 ? "" : output1Days);
  _output1AutoStartTemp.setValue(config.getInt(KEY_OUTPUT1_TEMPERATURE_START));
  _output1AutoStartTime.setValue(config.get(KEY_OUTPUT1_TIME_START));
  _output1AutoStoptTemp.setValue(config.getInt(KEY_OUTPUT1_TEMPERATURE_STOP));
  _output1AutoStoptTime.setValue(config.get(KEY_OUTPUT1_TIME_STOP));

  _output1Tab.setDisplay(dimmer1Enabled || output1RelayEnabled);
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
  _output1DimmerExcessLimiter.setDisplay(dimmer1Enabled);
  _output1DimmerDutyLimiter.setDisplay(dimmer1Enabled);
  _output1DimmerTempLimiter.setDisplay(dimmer1Enabled && output1TempEnabled);
  _output1BypassAuto.setDisplay(bypass1Possible);
  _output1AutoStartWDays.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1AutoStartTime.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1AutoStoptTime.setDisplay(bypass1Possible && autoBypass1Activated);
  _output1AutoStartTemp.setDisplay(bypass1Possible && autoBypass1Activated && output1TempEnabled);
  _output1AutoStoptTemp.setDisplay(bypass1Possible && autoBypass1Activated && output1TempEnabled);

  if (!output1) {
    _output1DimmerSliderRO.setValue(0);
    _output1BypassRO.setFeedback("DISABLED", dash::Status::IDLE);
    _output1Power.setValue(0);
    _output1ApparentPower.setValue(0);
    _output1PowerFactor.setValue(NAN);
    _output1THDi.setValue(NAN);
    _output1Voltage.setValue(0);
    _output1Current.setValue(0);
    _output1Resistance.setValue(NAN);
    _output1Energy.setValue(0);
  }

  // output 2

  const bool dimmer2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_DIMMER) && output2 && output2->isDimmerOnline();
  const bool output2RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT2_RELAY) && output2 && output2->isBypassRelayEnabled();
  const bool bypass2Possible = dimmer2Enabled || output2RelayEnabled;
  const bool autoDimmer2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  const bool autoBypass2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  const bool output2TempEnabled = (config.getBool(KEY_ENABLE_OUTPUT2_DS18) && ds18O2 && ds18O2->isEnabled()) || !config.isEmpty(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  const bool pzem2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_PZEM) && pzemO2 && pzemO2->isEnabled();
  const char* output2Days = config.get(KEY_OUTPUT2_DAYS);

  _output2DimmerAuto.setValue(autoDimmer2Activated);
  _output2DimmerExcessLimiter.setValue(config.getInt(KEY_OUTPUT2_EXCESS_LIMITER));
  _output2DimmerDutyLimiter.setValue(config.getInt(KEY_OUTPUT2_DIMMER_LIMIT));
  _output2DimmerTempLimiter.setValue(config.getInt(KEY_OUTPUT2_DIMMER_TEMP_LIMITER));
  _output2BypassAuto.setValue(autoBypass2Activated);
  _output2AutoStartWDays.setValue(strcmp(output2Days, YASOLR_WEEK_DAYS_EMPTY) == 0 ? "" : output2Days);
  _output2AutoStartTemp.setValue(config.getInt(KEY_OUTPUT2_TEMPERATURE_START));
  _output2AutoStartTime.setValue(config.get(KEY_OUTPUT2_TIME_START));
  _output2AutoStoptTemp.setValue(config.getInt(KEY_OUTPUT2_TEMPERATURE_STOP));
  _output2AutoStoptTime.setValue(config.get(KEY_OUTPUT2_TIME_STOP));

  _output2Tab.setDisplay(dimmer2Enabled || output2RelayEnabled);
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
  _output2DimmerExcessLimiter.setDisplay(dimmer2Enabled);
  _output2DimmerDutyLimiter.setDisplay(dimmer2Enabled);
  _output2DimmerTempLimiter.setDisplay(dimmer2Enabled && output2TempEnabled);
  _output2BypassAuto.setDisplay(bypass2Possible);
  _output2AutoStartWDays.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2AutoStartTime.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2AutoStoptTime.setDisplay(bypass2Possible && autoBypass2Activated);
  _output2AutoStartTemp.setDisplay(bypass2Possible && autoBypass2Activated && output2TempEnabled);
  _output2AutoStoptTemp.setDisplay(bypass2Possible && autoBypass2Activated && output2TempEnabled);

  if (!output2) {
    _output2DimmerSliderRO.setValue(0);
    _output2BypassRO.setFeedback("DISABLED", dash::Status::IDLE);
    _output2Power.setValue(0);
    _output2ApparentPower.setValue(0);
    _output2PowerFactor.setValue(NAN);
    _output2THDi.setValue(NAN);
    _output2Voltage.setValue(0);
    _output2Current.setValue(0);
    _output2Resistance.setValue(NAN);
    _output2Energy.setValue(0);
  }

  // relays

  const uint16_t load1 = config.getInt(KEY_RELAY1_LOAD);
  const uint16_t load2 = config.getInt(KEY_RELAY2_LOAD);
  const bool relay1Enabled = config.getBool(KEY_ENABLE_RELAY1) && relay1 && relay1->isEnabled();
  const bool relay2Enabled = config.getBool(KEY_ENABLE_RELAY2) && relay2 && relay2->isEnabled();
  _relaysTab.setDisplay(relay1Enabled || relay2Enabled);
  _relay1Switch.setDisplay(relay1Enabled && load1 <= 0);
  _relay1SwitchRO.setDisplay(relay1Enabled && load1 > 0);
  _relay2Switch.setDisplay(relay2Enabled && load2 <= 0);
  _relay2SwitchRO.setDisplay(relay2Enabled && load2 > 0);

  // management

  _configBackup.setValue("/api/config/backup");
  _configRestore.setValue("/api/config/restore");
  _consoleLink.setValue("/console");
  _debugInfo.setValue("/api/debug");
  _energyReset.setDisplay(jsyEnabled || pzem1Enabled || pzem2Enabled);
  _consoleLink.setDisplay(logger.isDebugEnabled());
  _debugInfo.setDisplay(logger.isDebugEnabled());

  // network

  _adminPwd.setValue(config.get(KEY_ADMIN_PASSWORD));
  _apMode.setValue(config.getBool(KEY_ENABLE_AP_MODE));
  _ntpServer.setValue(config.get(KEY_NTP_SERVER));
  _ntpTimezone.setValue(config.get(KEY_NTP_TIMEZONE));
  _wifiPwd.setValue(config.get(KEY_WIFI_PASSWORD));
  _wifiSSID.setValue(config.get(KEY_WIFI_SSID));
  _staticIP.setValue(config.get(KEY_NET_IP));
  _subnetMask.setValue(config.get(KEY_NET_SUBNET));
  _gateway.setValue(config.get(KEY_NET_GATEWAY));
  _dnsServer.setValue(config.get(KEY_NET_DNS));

  // mqtt

  _haDiscovery.setValue(config.getBool(KEY_ENABLE_HA_DISCOVERY));
  _haDiscoveryTopic.setValue(config.get(KEY_HA_DISCOVERY_TOPIC));
  _mqttGridPower.setValue(config.get(KEY_GRID_POWER_MQTT_TOPIC));
  _mqttGridVoltage.setValue(config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC));
  _mqttTempO1.setValue(config.get(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC));
  _mqttTempO2.setValue(config.get(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC));
  _mqttPort.setValue(config.getInt(KEY_MQTT_PORT));
  _mqttPublishInterval.setValue(config.getInt(KEY_MQTT_PUBLISH_INTERVAL));
  _mqttPwd.setValue(config.get(KEY_MQTT_PASSWORD));
  _mqttSecured.setValue(config.getBool(KEY_MQTT_SECURED));
  _mqttServer.setValue(config.get(KEY_MQTT_SERVER));
  _mqttServerCert.setValue("/api/config/mqttServerCertificate");
  _mqttTopic.setValue(config.get(KEY_MQTT_TOPIC));
  _mqttUser.setValue(config.get(KEY_MQTT_USERNAME));

  const bool serverCertExists = LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE);
  _mqttConfigTab.setDisplay(config.getBool(KEY_ENABLE_MQTT));
  _mqttServerCert.setDisplay(!serverCertExists);
  _mqttServerCertDelete.setDisplay(serverCertExists);

  _haDiscoveryTopic.setDisplay(config.getBool(KEY_ENABLE_HA_DISCOVERY));

  // GPIO

  std::unordered_map<int32_t, dash::FeedbackTextInputCard<int32_t>*> pinout = {};
  _pinout(_pinDimmerO1, KEY_PIN_OUTPUT1_DIMMER, pinout);
  _pinout(_pinDimmerO2, KEY_PIN_OUTPUT2_DIMMER, pinout);
  _pinout(_pinDisplayClock, KEY_PIN_DISPLAY_SCL, pinout);
  _pinout(_pinDisplayData, KEY_PIN_DISPLAY_SDA, pinout);
  _pinout(_pinDS18O1, KEY_PIN_OUTPUT1_DS18, pinout);
  _pinout(_pinDS18O2, KEY_PIN_OUTPUT2_DS18, pinout);
  _pinout(_pinDS18Router, KEY_PIN_ROUTER_DS18, pinout);
  _pinout(_pinJsyRX, KEY_PIN_JSY_RX, pinout);
  _pinout(_pinJsyTX, KEY_PIN_JSY_TX, pinout);
  _pinout(_pinLEDGreen, KEY_PIN_LIGHTS_GREEN, pinout);
  _pinout(_pinLEDRed, KEY_PIN_LIGHTS_RED, pinout);
  _pinout(_pinLEDYellow, KEY_PIN_LIGHTS_YELLOW, pinout);
  _pinout(_pinPZEMRX, KEY_PIN_PZEM_RX, pinout);
  _pinout(_pinPZEMTX, KEY_PIN_PZEM_TX, pinout);
  _pinout(_pinRelay1, KEY_PIN_RELAY1, pinout);
  _pinout(_pinRelay2, KEY_PIN_RELAY2, pinout);
  _pinout(_pinRelayO1, KEY_PIN_OUTPUT1_RELAY, pinout);
  _pinout(_pinRelayO2, KEY_PIN_OUTPUT2_RELAY, pinout);
  _pinout(_pinZCD, KEY_PIN_ZCD, pinout);
  pinout.clear();

  // Hardware

  _status(_debugMode, KEY_ENABLE_DEBUG, logger.isDebugEnabled());
  _status(_display, KEY_ENABLE_DISPLAY, display && display->isEnabled());
  _status(_jsyRemote, KEY_ENABLE_JSY_REMOTE, udp && udp->connected());
  _status(_led, KEY_ENABLE_LIGHTS, lights.isEnabled());
  _status(_output1Relay, KEY_ENABLE_OUTPUT1_RELAY, output1 && output1->isBypassRelayEnabled());
  _status(_output2Relay, KEY_ENABLE_OUTPUT2_RELAY, output2 && output2->isBypassRelayEnabled());
  _status(_relay1, KEY_ENABLE_RELAY1, relay1 && relay1->isEnabled());
  _status(_relay2, KEY_ENABLE_RELAY2, relay2 && relay2->isEnabled());

  // Hardware Config

  switch (config.getInt(KEY_GRID_FREQUENCY)) {
    case 50:
      _gridFreq.setValue("50 Hz");
      break;
    case 60:
      _gridFreq.setValue("60 Hz");
      break;
    default:
      _gridFreq.setValue("Auto-detect");
      break;
  }

  _displayType.setValue(config.get(KEY_DISPLAY_TYPE));
  _displaySpeed.setValue(config.getInt(KEY_DISPLAY_SPEED));
  _displayRotation.setValue(config.getInt(KEY_DISPLAY_ROTATION));
  _output1RelayType.setValue(config.get(KEY_OUTPUT1_RELAY_TYPE));
  _output2RelayType.setValue(config.get(KEY_OUTPUT2_RELAY_TYPE));
  _relay1Type.setValue(config.get(KEY_RELAY1_TYPE));
  _relay2Type.setValue(config.get(KEY_RELAY2_TYPE));
  _relay1Load.setValue(load1);
  _relay2Load.setValue(load2);
  _output1ResistanceInput.setValue(config.getFloat(KEY_OUTPUT1_RESISTANCE));
  _output1ResistanceInput.setStatus(_output1ResistanceInput.value() ? dash::Status::SUCCESS : dash::Status::DANGER);
  _output2ResistanceInput.setValue(config.getFloat(KEY_OUTPUT2_RESISTANCE));
  _output2ResistanceInput.setStatus(_output2ResistanceInput.value() ? dash::Status::SUCCESS : dash::Status::DANGER);
  _output1DimmerMapper.setValue({static_cast<uint8_t>(config.getInt(KEY_OUTPUT1_DIMMER_MIN)),
                                 static_cast<uint8_t>(config.getInt(KEY_OUTPUT1_DIMMER_MAX))});
  _output2DimmerMapper.setValue({static_cast<uint8_t>(config.getInt(KEY_OUTPUT2_DIMMER_MIN)),
                                 static_cast<uint8_t>(config.getInt(KEY_OUTPUT2_DIMMER_MAX))});
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

  switch (config.getInt(KEY_PID_P_MODE)) {
    case 1:
      _pidPMode.setValue(YASOLR_PID_P_MODE_1);
      break;
    case 2:
      _pidPMode.setValue(YASOLR_PID_P_MODE_2);
      break;
    case 3:
      _pidPMode.setValue(YASOLR_PID_P_MODE_3);
      break;
    default:
      _pidPMode.setValue("");
      break;
  }
  switch (config.getInt(KEY_PID_D_MODE)) {
    case 1:
      _pidDMode.setValue(YASOLR_PID_D_MODE_1);
      break;
    case 2:
      _pidDMode.setValue(YASOLR_PID_D_MODE_2);
      break;
    default:
      _pidDMode.setValue("");
      break;
  }
  switch (config.getInt(KEY_PID_IC_MODE)) {
    case 0:
      _pidICMode.setValue(YASOLR_PID_IC_MODE_0);
      break;
    case 1:
      _pidICMode.setValue(YASOLR_PID_IC_MODE_1);
      break;
    case 2:
      _pidICMode.setValue(YASOLR_PID_IC_MODE_2);
      break;
    default:
      _pidICMode.setValue("");
      break;
  }
  _pidSetpoint.setValue(config.getInt(KEY_PID_SETPOINT));
  _pidKp.setValue(config.getFloat(KEY_PID_KP));
  _pidKi.setValue(config.getFloat(KEY_PID_KI));
  _pidKd.setValue(config.getFloat(KEY_PID_KD));
  _pidOutMin.setValue(config.getInt(KEY_PID_OUT_MIN));
  _pidOutMax.setValue(config.getInt(KEY_PID_OUT_MAX));

  const bool pidViewEnabled = pidCharts();
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
  grid.getGridMeasurements(gridMetrics);

  // stats
  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);
  _output1RelaySwitchCount.setValue(output1 ? output1->getBypassRelaySwitchCount() : 0);
  _output2RelaySwitchCount.setValue(output2 ? output2->getBypassRelaySwitchCount() : 0);
  _deviceHeapTotal.setValue(memory.total);
  _deviceHeapUsed.setValue(memory.used);
  _deviceHeapUsage.setValue(memory.usage);
  _gridEnergy.setValue(gridMetrics.energy);
  _gridEnergyReturned.setValue(gridMetrics.energyReturned);
  _gridFrequency.setValue(yasolr_frequency());
  _networkWiFiRSSI.setValue(espConnect.getWiFiRSSI());
  _networkWiFiSignal.setValue(espConnect.getWiFiSignalQuality());
  _relay1SwitchCount.setValue(relay1 ? relay1->getSwitchCount() : 0);
  _relay2SwitchCount.setValue(relay2 ? relay2->getSwitchCount() : 0);
  _udpMessageRateBuffer.setValue(udpMessageRateBuffer ? udpMessageRateBuffer->rate() : 0);
  _time.setValue(Mycila::Time::getLocalStr());
  _uptime.setValue(Mycila::Time::toDHHMMSS(Mycila::System::getUptime()));
#ifdef APP_MODEL_TRIAL
  _trialRemainingTime.setValue(Mycila::Time::toDHHMMSS(Mycila::Trial.getRemaining()));
#endif

  // home

  Mycila::Router::Metrics routerMetrics;
  router.getRouterMeasurements(routerMetrics);

  _routerPower.setValue(routerMetrics.power);
  _routerApparentPower.setValue(routerMetrics.apparentPower);
  _routerPowerFactor.setValue(routerMetrics.powerFactor);
  _routerTHDi.setValue(routerMetrics.thdi * 100);
  _routerVoltage.setValue(gridMetrics.voltage);
  _routerCurrent.setValue(routerMetrics.current);
  _routerResistance.setValue(routerMetrics.resistance);
  _routerEnergy.setValue(routerMetrics.energy);
  _gridPower.setValue(gridMetrics.power);
  _routerDS18State.setValue(ds18Sys ? ds18Sys->getTemperature().value_or(0.0f) : 0);

  // output 1

  if (output1) {
    switch (output1->getState()) {
      case Mycila::RouterOutput::State::OUTPUT_DISABLED:
      case Mycila::RouterOutput::State::OUTPUT_IDLE:
        _output1State.setFeedback(output1->getStateName(), dash::Status::IDLE);
        break;
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_AUTO:
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_MANUAL:
        _output1State.setFeedback(output1->getStateName(), dash::Status::WARNING);
        break;
      case Mycila::RouterOutput::State::OUTPUT_ROUTING:
        _output1State.setFeedback(output1->getStateName(), dash::Status::SUCCESS);
        break;
      default:
        _output1State.setFeedback(YASOLR_LBL_109, dash::Status::DANGER);
        break;
    }
    _output1DS18State.setValue(output1->temperature().orElse(NAN));
    _output1DimmerSlider.setValue(output1->getDimmerDutyCycle() * 100);
    _output1Bypass.setValue(output1->isBypassOn());
  }

  // output 2

  if (output2) {
    switch (output2->getState()) {
      case Mycila::RouterOutput::State::OUTPUT_DISABLED:
      case Mycila::RouterOutput::State::OUTPUT_IDLE:
        _output2State.setFeedback(output2->getStateName(), dash::Status::IDLE);
        break;
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_AUTO:
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_MANUAL:
        _output2State.setFeedback(output2->getStateName(), dash::Status::WARNING);
        break;
      case Mycila::RouterOutput::State::OUTPUT_ROUTING:
        _output2State.setFeedback(output2->getStateName(), dash::Status::SUCCESS);
        break;
      default:
        _output2State.setFeedback(YASOLR_LBL_109, dash::Status::DANGER);
        break;
    }
    _output2DS18State.setValue(output2->temperature().orElse(NAN));
    _output2DimmerSlider.setValue(output2->getDimmerDutyCycle() * 100);
    _output2Bypass.setValue(output2->isBypassOn());
  }

  // relay

  _relay1Switch.setValue(relay1 && relay1->isOn());
  _relay2Switch.setValue(relay1 && relay2->isOn());

  // Hardware (config)

  _output1PZEMSync.setValue(pzemO1PairingTask && !pzemO1PairingTask->isPaused());
  _output2PZEMSync.setValue(pzemO2PairingTask && !pzemO2PairingTask->isPaused());
  _resistanceCalibration.setValue(router.isCalibrationRunning());

#ifdef APP_MODEL_PRO
  // Output 1

  if (output1) {
    Mycila::RouterOutput::Metrics output1Measurements;
    output1->getOutputMeasurements(output1Measurements);

    _output1DimmerSliderRO.setValue(output1->getDimmerDutyCycleLive() * 100);
    _output1BypassRO.setFeedback(YASOLR_STATE(output1->isBypassOn()), output1->isBypassOn() ? dash::Status::SUCCESS : dash::Status::IDLE);
    _output1Power.setValue(output1Measurements.power);
    _output1ApparentPower.setValue(output1Measurements.apparentPower);
    _output1PowerFactor.setValue(output1Measurements.powerFactor);
    _output1THDi.setValue(output1Measurements.thdi * 100);
    _output1Voltage.setValue(output1Measurements.dimmedVoltage);
    _output1Current.setValue(output1Measurements.current);
    _output1Resistance.setValue(output1Measurements.resistance);
    _output1Energy.setValue(output1Measurements.energy);
  }

  // output 2

  if (output2) {
    Mycila::RouterOutput::Metrics output2Measurements;
    output2->getOutputMeasurements(output2Measurements);

    _output2DimmerSliderRO.setValue(output2->getDimmerDutyCycleLive() * 100);
    _output2BypassRO.setFeedback(YASOLR_STATE(output2->isBypassOn()), output2->isBypassOn() ? dash::Status::SUCCESS : dash::Status::IDLE);
    _output2Power.setValue(output2Measurements.power);
    _output2ApparentPower.setValue(output2Measurements.apparentPower);
    _output2PowerFactor.setValue(output2Measurements.powerFactor);
    _output2THDi.setValue(output2Measurements.thdi * 100);
    _output2Voltage.setValue(output2Measurements.dimmedVoltage);
    _output2Current.setValue(output2Measurements.current);
    _output2Resistance.setValue(output2Measurements.resistance);
    _output2Energy.setValue(output2Measurements.energy);
  }

  // relays

  _relay1SwitchRO.setFeedback(YASOLR_STATE(relay1 && relay1->isOn()), relay1 && relay1->isOn() ? dash::Status::SUCCESS : dash::Status::IDLE);
  _relay2SwitchRO.setFeedback(YASOLR_STATE(relay2 && relay2->isOn()), relay2 && relay2->isOn() ? dash::Status::SUCCESS : dash::Status::IDLE);

  // Hardware

  _status(_jsy, KEY_ENABLE_JSY, jsy && jsy->isEnabled(), jsy && jsy->isConnected(), YASOLR_LBL_110);
  _status(_mqtt, KEY_ENABLE_MQTT, mqtt && mqtt->isEnabled(), mqtt && mqtt->isConnected(), mqtt && mqtt->getLastError() ? mqtt->getLastError() : YASOLR_LBL_113);
  _status(_output1Dimmer, KEY_ENABLE_OUTPUT1_DIMMER, output1 && output1->isDimmerEnabled(), output1 && output1->isDimmerOnline(), YASOLR_LBL_110);
  _status(_output1DS18, KEY_ENABLE_OUTPUT1_DS18, ds18O1 && ds18O1->isEnabled(), ds18O1 && ds18O1->getLastTime() > 0, YASOLR_LBL_114);
  _status(_output1PZEM, KEY_ENABLE_OUTPUT1_PZEM, pzemO1 && pzemO1->isEnabled(), pzemO1 && pzemO1->isConnected() && pzemO1->getDeviceAddress() == YASOLR_PZEM_ADDRESS_OUTPUT1, pzemO1 && pzemO1->isConnected() ? YASOLR_LBL_180 : YASOLR_LBL_110);
  _status(_output2Dimmer, KEY_ENABLE_OUTPUT2_DIMMER, output2 && output2->isDimmerEnabled(), output2 && output2->isDimmerOnline(), YASOLR_LBL_110);
  _status(_output2DS18, KEY_ENABLE_OUTPUT2_DS18, ds18O2 && ds18O2->isEnabled(), ds18O2 && ds18O2->getLastTime() > 0, YASOLR_LBL_114);
  _status(_output2PZEM, KEY_ENABLE_OUTPUT2_PZEM, pzemO2 && pzemO2->isEnabled(), pzemO2 && pzemO2->isConnected() && pzemO2->getDeviceAddress() == YASOLR_PZEM_ADDRESS_OUTPUT2, pzemO2 && pzemO2->isConnected() ? YASOLR_LBL_180 : YASOLR_LBL_110);
  _status(_routerDS18, KEY_ENABLE_DS18_SYSTEM, ds18Sys && ds18Sys->isEnabled(), ds18Sys && ds18Sys->getLastTime() > 0, YASOLR_LBL_114);
  _status(_zcd, KEY_ENABLE_ZCD, pulseAnalyzer && pulseAnalyzer->isEnabled(), pulseAnalyzer && pulseAnalyzer->isOnline(), YASOLR_LBL_110);
#endif
}

void YaSolR::Website::updateCharts() {
  // read last metrics
  Mycila::Router::Metrics routerMetrics;
  router.getRouterMeasurements(routerMetrics);

  // shift array
  memmove(&_gridPowerHistoryY[0], &_gridPowerHistoryY[1], sizeof(_gridPowerHistoryY) - sizeof(*_gridPowerHistoryY));
  memmove(&_routedPowerHistoryY[0], &_routedPowerHistoryY[1], sizeof(_routedPowerHistoryY) - sizeof(*_routedPowerHistoryY));
  memmove(&_routerTHDiHistoryY[0], &_routerTHDiHistoryY[1], sizeof(_routerTHDiHistoryY) - sizeof(*_routerTHDiHistoryY));

  // set new value
  _gridPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = round(grid.getPower().orElse(0));
  _routedPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = round(routerMetrics.power);
  _routerTHDiHistoryY[YASOLR_GRAPH_POINTS - 1] = isnan(routerMetrics.thdi) ? 0 : round(routerMetrics.thdi * 100);

  // update charts
  _gridPowerHistory.setY(_gridPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.setY(_routedPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.setY(_routerTHDiHistoryY, YASOLR_GRAPH_POINTS);
}

void YaSolR::Website::updatePID() {
#ifdef APP_MODEL_PRO
  // shift array
  memmove(&_pidInputHistoryY[0], &_pidInputHistoryY[1], sizeof(_pidInputHistoryY) - sizeof(*_pidInputHistoryY));
  memmove(&_pidOutputHistoryY[0], &_pidOutputHistoryY[1], sizeof(_pidOutputHistoryY) - sizeof(*_pidOutputHistoryY));
  memmove(&_pidErrorHistoryY[0], &_pidErrorHistoryY[1], sizeof(_pidErrorHistoryY) - sizeof(*_pidErrorHistoryY));
  memmove(&_pidSumHistoryY[0], &_pidSumHistoryY[1], sizeof(_pidSumHistoryY) - sizeof(*_pidSumHistoryY));
  memmove(&_pidPTermHistoryY[0], &_pidPTermHistoryY[1], sizeof(_pidPTermHistoryY) - sizeof(*_pidPTermHistoryY));
  memmove(&_pidITermHistoryY[0], &_pidITermHistoryY[1], sizeof(_pidITermHistoryY) - sizeof(*_pidITermHistoryY));
  memmove(&_pidDTermHistoryY[0], &_pidDTermHistoryY[1], sizeof(_pidDTermHistoryY) - sizeof(*_pidDTermHistoryY));

  // set new values
  _pidInputHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getInput());
  _pidOutputHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getOutput());
  _pidErrorHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getError());
  _pidSumHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getSum());
  _pidPTermHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getPTerm());
  _pidITermHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getITerm());
  _pidDTermHistoryY[YASOLR_GRAPH_POINTS - 1] = round(pidController.getDTerm());

  // update charts
  _pidInputHistory.setY(_pidInputHistoryY, YASOLR_GRAPH_POINTS);
  _pidOutputHistory.setY(_pidOutputHistoryY, YASOLR_GRAPH_POINTS);
  _pidErrorHistory.setY(_pidErrorHistoryY, YASOLR_GRAPH_POINTS);
  _pidSumHistory.setY(_pidSumHistoryY, YASOLR_GRAPH_POINTS);
  _pidPTermHistory.setY(_pidPTermHistoryY, YASOLR_GRAPH_POINTS);
  _pidITermHistory.setY(_pidITermHistoryY, YASOLR_GRAPH_POINTS);
  _pidDTermHistory.setY(_pidDTermHistoryY, YASOLR_GRAPH_POINTS);
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

bool YaSolR::Website::pidCharts() const {
#ifdef APP_MODEL_PRO
  return _pidView.optional().value_or(false);
#else
  return false;
#endif
}
