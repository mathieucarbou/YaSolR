// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr_dashboard.h>

#include <string>
#include <unordered_map>

#ifdef APP_MODEL_OSS
  #define LineChart                BarChart
  #define AreaChart                BarChart
  #define EnergyCard               GenericCard
  #define FeedbackToggleButtonCard ToggleButtonCard
  #define IndicatorButtonCard      ToggleButtonCard
#endif

#ifdef APP_MODEL_PRO
static constexpr dash::Widget::Size FULL_SIZE = {.xs = 12, .sm = 12, .md = 12, .lg = 12, .xl = 12, .xxl = 12};

// tabs icons:
// https://en.wikipedia.org/wiki/List_of_Unicode_characters#Miscellaneous_Symbols
// https://en.wikipedia.org/wiki/List_of_Unicode_characters#Dingbats

// tabs are declared early in order to have the smallest IDs that never change
static dash::Tab _output1Tab(dashboard, YASOLR_LBL_046, dash::Icon::ZAP_ICON);
static dash::Tab _output2Tab(dashboard, YASOLR_LBL_070, dash::Icon::ZAP_ICON);
static dash::Tab _pidTab(dashboard, YASOLR_LBL_159, dash::Icon::RECYCLE_ICON);
static dash::Tab _networkTab(dashboard, YASOLR_LBL_087, dash::Icon::WIFI_ICON);
static dash::Tab _ntpTab(dashboard, YASOLR_LBL_158, dash::Icon::CLOCK_ICON);
static dash::Tab _mqttTab(dashboard, YASOLR_LBL_095, dash::Icon::CLOUD_UPLOAD_ICON);
static dash::Tab _gpioTab(dashboard, YASOLR_LBL_108, dash::Icon::CABLE_ICON);
static dash::Tab _hardwareConfigTab(dashboard, YASOLR_LBL_177, dash::Icon::CPU_ICON);
static dash::Tab _output1ConfigTab(dashboard, YASOLR_LBL_138, dash::Icon::COG_ICON);
static dash::Tab _output2ConfigTab(dashboard, YASOLR_LBL_139, dash::Icon::COG_ICON);
static dash::Tab _systemTab(dashboard, YASOLR_LBL_078, dash::Icon::MONITOR_COG_ICON);
static dash::Tab _debugTab(dashboard, YASOLR_LBL_083, dash::Icon::BUG_ICON);

#endif

static int8_t _historyX[YASOLR_GRAPH_POINTS] = {0};

// statistics

static dash::StatisticValue<const char*> _appName(dashboard, YASOLR_LBL_001);
static dash::StatisticValue<const char*> _appModel(dashboard, YASOLR_LBL_002);
static dash::StatisticValue<const char*> _appVersion(dashboard, YASOLR_LBL_003);
static dash::StatisticValue<const char*> _appManufacturer(dashboard, YASOLR_LBL_004);
static dash::StatisticValue<uint32_t> _deviceBootCount(dashboard, YASOLR_LBL_005);
static dash::StatisticValue<const char*> _deviceBootReason(dashboard, YASOLR_LBL_192);
static dash::StatisticValue<uint8_t> _deviceCores(dashboard, YASOLR_LBL_006);
static dash::StatisticValue<size_t> _deviceHeapTotal(dashboard, YASOLR_LBL_007);
static dash::StatisticValue<size_t> _deviceHeapUsed(dashboard, YASOLR_LBL_009);
static dash::StatisticValue<float, 2> _deviceHeapUsage(dashboard, YASOLR_LBL_008);
static dash::StatisticValue<size_t> _deviceHeapMinFree(dashboard, YASOLR_LBL_149);
static dash::StatisticValue<const char*> _deviceID(dashboard, YASOLR_LBL_010);
static dash::StatisticValue<const char*> _deviceModel(dashboard, YASOLR_LBL_011);
static dash::StatisticValue<const char*> _firmwareBuildHash(dashboard, YASOLR_LBL_013);
static dash::StatisticValue<const char*> _firmwareBuildTimestamp(dashboard, YASOLR_LBL_014);
static dash::StatisticValue<const char*> _firmwareFilename(dashboard, YASOLR_LBL_015);
static dash::StatisticValue<uint32_t> _gridEnergy(dashboard, YASOLR_LBL_016);
static dash::StatisticValue<uint32_t> _gridEnergyReturned(dashboard, YASOLR_LBL_017);
static dash::StatisticValue<float, 1> _gridFrequency(dashboard, YASOLR_LBL_018);
static dash::StatisticValue<float, 2> _udpMessageRateBuffer(dashboard, YASOLR_LBL_157);
static dash::StatisticValue<const char*> _networkHostname(dashboard, YASOLR_LBL_019);
static dash::StatisticValue<const char*> _networkInterface(dashboard, YASOLR_LBL_020);
static dash::StatisticValue _networkAPIP(dashboard, YASOLR_LBL_021);
static dash::StatisticValue _networkAPMAC(dashboard, YASOLR_LBL_022);
static dash::StatisticValue _networkEthIP(dashboard, YASOLR_LBL_023);
static dash::StatisticValue _networkEthIPv6Local(dashboard, YASOLR_LBL_202);
static dash::StatisticValue _networkEthIPv6Global(dashboard, YASOLR_LBL_204);
static dash::StatisticValue _networkEthMAC(dashboard, YASOLR_LBL_024);
static dash::StatisticValue _networkWiFiIP(dashboard, YASOLR_LBL_025);
static dash::StatisticValue _networkWiFiIPv6Local(dashboard, YASOLR_LBL_201);
static dash::StatisticValue _networkWiFiIPv6Global(dashboard, YASOLR_LBL_203);
static dash::StatisticValue _networkWiFiMAC(dashboard, YASOLR_LBL_026);
static dash::StatisticValue _networkWiFiBSSID(dashboard, YASOLR_LBL_161);
static dash::StatisticValue _networkWiFiSSID(dashboard, YASOLR_LBL_027);
static dash::StatisticValue<int8_t> _networkWiFiRSSI(dashboard, YASOLR_LBL_028);
static dash::StatisticValue<int8_t> _networkWiFiSignal(dashboard, YASOLR_LBL_029);
static dash::StatisticValue<uint64_t> _output1RelaySwitchCount(dashboard, YASOLR_LBL_030);
static dash::StatisticValue<uint64_t> _output2RelaySwitchCount(dashboard, YASOLR_LBL_031);
static dash::StatisticValue<uint64_t> _relay1SwitchCount(dashboard, YASOLR_LBL_032);
static dash::StatisticValue<uint64_t> _relay2SwitchCount(dashboard, YASOLR_LBL_033);
static dash::StatisticValue _time(dashboard, YASOLR_LBL_034);
static dash::StatisticValue _uptime(dashboard, YASOLR_LBL_035);

#ifdef APP_MODEL_TRIAL
static dash::StatisticValue _trialRemainingTime(dashboard, "Trial Remaining Time");
#endif

// home

static dash::FeedbackCard _output1State(dashboard, YASOLR_LBL_046);
static dash::FeedbackCard _output2State(dashboard, YASOLR_LBL_070);
static dash::FeedbackToggleButtonCard _relay1Switch(dashboard, YASOLR_LBL_074);
static dash::FeedbackToggleButtonCard _relay2Switch(dashboard, YASOLR_LBL_077);

static dash::EnergyCard<float, 0> _routerPower(dashboard, YASOLR_LBL_036, "W");
static dash::EnergyCard<float, 0> _routerApparentPower(dashboard, YASOLR_LBL_037, "VA");
static dash::EnergyCard<float, 2> _routerPowerFactor(dashboard, YASOLR_LBL_038);
static dash::EnergyCard<float, 2> _routerTHDi(dashboard, YASOLR_LBL_039, "%");
static dash::EnergyCard<float, 0> _routerVoltage(dashboard, YASOLR_LBL_040, "V");
static dash::EnergyCard<float, 2> _routerCurrent(dashboard, YASOLR_LBL_041, "A");
static dash::EnergyCard<float, 2> _routerResistance(dashboard, YASOLR_LBL_042, "Ω");
static dash::EnergyCard<uint32_t> _routerEnergy(dashboard, YASOLR_LBL_043, "Wh");
static dash::EnergyCard<float, 0> _gridPower(dashboard, YASOLR_LBL_044, "W");
static dash::TemperatureCard<float, 2> _routerDS18State(dashboard, YASOLR_LBL_045);

#ifdef APP_MODEL_OSS
static dash::TemperatureCard<float, 2> _output1DS18State(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_048);
static dash::SliderCard<float, 2> _output1DimmerSlider(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
static dash::ToggleButtonCard _output1Bypass(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_051);
static dash::TemperatureCard<float, 2> _output2DS18State(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_048);
static dash::SliderCard<float, 2> _output2DimmerSlider(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
static dash::ToggleButtonCard _output2Bypass(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_051);
#endif

static int16_t _gridPowerHistoryY[YASOLR_GRAPH_POINTS] = {0};
static uint16_t _routedPowerHistoryY[YASOLR_GRAPH_POINTS] = {0};
static uint8_t _routerTHDiHistoryY[YASOLR_GRAPH_POINTS] = {0};
static dash::LineChart<int8_t, int16_t> _gridPowerHistory(dashboard, YASOLR_LBL_044 " (W)");
static dash::AreaChart<int8_t, uint16_t> _routedPowerHistory(dashboard, YASOLR_LBL_036 " (W)");
static dash::BarChart<int8_t, uint8_t> _routerTHDiHistory(dashboard, YASOLR_LBL_039 " (%)");

#ifdef APP_MODEL_OSS
static dash::IndicatorButtonCard _output1PZEMSync(dashboard, YASOLR_LBL_147);
static dash::IndicatorButtonCard _output2PZEMSync(dashboard, YASOLR_LBL_148);
static dash::IndicatorButtonCard _output1ResistanceCalibration(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_186);
static dash::IndicatorButtonCard _output2ResistanceCalibration(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_186);
#endif

#ifdef APP_MODEL_PRO
// tab: output 1

static dash::ToggleButtonCard _output1DimmerAuto(dashboard, YASOLR_LBL_060);
static dash::SliderCard<float, 2> _output1DimmerSlider(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
static dash::ProgressCard<float, 2> _output1DimmerSliderRO(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, "%");
static dash::ToggleButtonCard _output1BypassAuto(dashboard, YASOLR_LBL_064);
static dash::FeedbackToggleButtonCard _output1Bypass(dashboard, YASOLR_LBL_051);

static dash::EnergyCard<float, 0> _output1Power(dashboard, YASOLR_LBL_052, "W");
static dash::EnergyCard<float, 0> _output1ApparentPower(dashboard, YASOLR_LBL_053, "VA");
static dash::EnergyCard<float, 2> _output1PowerFactor(dashboard, YASOLR_LBL_054);
static dash::EnergyCard<float, 2> _output1THDi(dashboard, YASOLR_LBL_055, "%");
static dash::EnergyCard<float, 0> _output1Voltage(dashboard, YASOLR_LBL_056, "V");
static dash::EnergyCard<float, 2> _output1Current(dashboard, YASOLR_LBL_057, "A");
static dash::EnergyCard<float, 2> _output1Resistance(dashboard, YASOLR_LBL_058, "Ω");
static dash::EnergyCard<uint32_t> _output1Energy(dashboard, YASOLR_LBL_059, "Wh");

static dash::TemperatureCard<float, 2> _output1DS18State(dashboard, YASOLR_LBL_048);

// tab: output 2

static dash::ToggleButtonCard _output2DimmerAuto(dashboard, YASOLR_LBL_060);
static dash::SliderCard<float, 2> _output2DimmerSlider(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, 0.01f, "%");
static dash::ProgressCard<float, 2> _output2DimmerSliderRO(dashboard, YASOLR_LBL_050, 0.0f, 100.0f, "%");
static dash::ToggleButtonCard _output2BypassAuto(dashboard, YASOLR_LBL_064);
static dash::FeedbackToggleButtonCard _output2Bypass(dashboard, YASOLR_LBL_051);

static dash::EnergyCard<float, 0> _output2Power(dashboard, YASOLR_LBL_052, "W");
static dash::EnergyCard<float, 0> _output2ApparentPower(dashboard, YASOLR_LBL_053, "VA");
static dash::EnergyCard<float, 2> _output2PowerFactor(dashboard, YASOLR_LBL_054);
static dash::EnergyCard<float, 2> _output2THDi(dashboard, YASOLR_LBL_055, "%");
static dash::EnergyCard<float, 0> _output2Voltage(dashboard, YASOLR_LBL_056, "V");
static dash::EnergyCard<float, 2> _output2Current(dashboard, YASOLR_LBL_057, "A");
static dash::EnergyCard<float, 2> _output2Resistance(dashboard, YASOLR_LBL_058, "Ω");
static dash::EnergyCard<uint32_t> _output2Energy(dashboard, YASOLR_LBL_059, "Wh");

static dash::TemperatureCard<float, 2> _output2DS18State(dashboard, YASOLR_LBL_048);

// tab: system

static dash::LinkCard<const char*> _configBackup(dashboard, YASOLR_LBL_079);
static dash::FileUploadCard<const char*> _configRestore(dashboard, YASOLR_LBL_080, ".txt");
static dash::PushButtonCard _restart(dashboard, YASOLR_LBL_082);
static dash::PushButtonCard _safeBoot(dashboard, YASOLR_LBL_081);
static dash::PushButtonCard _reset(dashboard, YASOLR_LBL_086);
static dash::PushButtonCard _energyReset(dashboard, YASOLR_LBL_085);
static dash::FileUploadCard<const char*> _safebootUpload(dashboard, YASOLR_LBL_193, ".bin,.bin.gz");
static dash::FeedbackCard<const char*> _safebootUploadStatus(dashboard, YASOLR_LBL_194);

// tab: debug

static dash::ToggleButtonCard _debugMode(dashboard, YASOLR_LBL_083);
static dash::LinkCard<const char*> _debugInfo(dashboard, YASOLR_LBL_178);
static dash::LinkCard<const char*> _startupLogs(dashboard, YASOLR_LBL_184);
static dash::LinkCard<const char*> _consoleLink(dashboard, YASOLR_LBL_084);

// tab: network

static dash::InputCard _hostnameWidget(dashboard, YASOLR_LBL_073);
static dash::InputCard<const char*> _wifiSSID(dashboard, YASOLR_LBL_092);
static dash::InputCard<const char*> _wifiBSSID(dashboard, YASOLR_LBL_047);
static dash::PasswordCard _wifiPwd(dashboard, YASOLR_LBL_093, YASOLR_HIDDEN_PWD);
static dash::InputCard<const char*> _staticIP(dashboard, YASOLR_LBL_188);
static dash::InputCard<const char*> _subnetMask(dashboard, YASOLR_LBL_189);
static dash::InputCard<const char*> _gateway(dashboard, YASOLR_LBL_190);
static dash::InputCard<const char*> _dnsServer(dashboard, YASOLR_LBL_191);
static dash::PasswordCard _adminPwd(dashboard, YASOLR_LBL_088, YASOLR_HIDDEN_PWD);
static dash::ToggleButtonCard _apMode(dashboard, YASOLR_LBL_094);

// tab: ntp

static dash::InputCard<const char*> _ntpServer(dashboard, YASOLR_LBL_089);
static dash::AsyncDropdownCard<const char*> _ntpTimezone(dashboard, YASOLR_LBL_090, "/timezones");
static dash::TimeSyncCard _ntpSync(dashboard, YASOLR_LBL_091);

// tab: mqtt

static dash::FeedbackToggleButtonCard _mqtt(dashboard, YASOLR_LBL_095);
static dash::InputCard<const char*> _mqttServer(dashboard, YASOLR_LBL_096);
static dash::InputCard<uint16_t> _mqttPort(dashboard, YASOLR_LBL_097);
static dash::InputCard<const char*> _mqttUser(dashboard, YASOLR_LBL_098);
static dash::PasswordCard _mqttPwd(dashboard, YASOLR_LBL_099, YASOLR_HIDDEN_PWD);
static dash::ToggleButtonCard _mqttSecured(dashboard, YASOLR_LBL_100);
static dash::FileUploadCard _mqttServerCert(dashboard, YASOLR_LBL_101, ".pem");
static dash::PushButtonCard _mqttServerCertDelete(dashboard, YASOLR_LBL_049);
static dash::InputCard<const char*> _mqttTopic(dashboard, YASOLR_LBL_103);
static dash::SliderCard<uint8_t> _mqttPublishInterval(dashboard, YASOLR_LBL_102, 1, 30, 1, "s");
static dash::SeparatorCard<const char*> _haSep(dashboard, YASOLR_LBL_107);
static dash::ToggleButtonCard _haDiscovery(dashboard, YASOLR_LBL_104);
static dash::InputCard<const char*> _haDiscoveryTopic(dashboard, YASOLR_LBL_105);
static dash::SeparatorCard<const char*> _mqttSep1(dashboard, YASOLR_LBL_179);
static dash::InputCard<const char*> _mqttGridVoltage(dashboard, YASOLR_LBL_106);
static dash::InputCard<const char*> _mqttGridPower(dashboard, YASOLR_LBL_044);
static dash::InputCard<const char*> _mqttTempO1(dashboard, YASOLR_LBL_181);
static dash::InputCard<const char*> _mqttTempO2(dashboard, YASOLR_LBL_182);

// tab: pid

static int16_t _pidInputHistoryY[YASOLR_GRAPH_POINTS] = {0};
static int16_t _pidOutputHistoryY[YASOLR_GRAPH_POINTS] = {0};
static int16_t _pidErrorHistoryY[YASOLR_GRAPH_POINTS] = {0};
static int16_t _pidSumHistoryY[YASOLR_GRAPH_POINTS] = {0};
static int16_t _pidPTermHistoryY[YASOLR_GRAPH_POINTS] = {0};
static int16_t _pidITermHistoryY[YASOLR_GRAPH_POINTS] = {0};
static int16_t _pidDTermHistoryY[YASOLR_GRAPH_POINTS] = {0};
static dash::DropdownCard<const char*> _pidPMode(dashboard, YASOLR_LBL_160, YASOLR_PID_P_MODE_1 "," YASOLR_PID_P_MODE_2);
static dash::DropdownCard<const char*> _pidICMode(dashboard, YASOLR_LBL_162, YASOLR_PID_IC_MODE_1 "," YASOLR_PID_IC_MODE_2);
static dash::InputCard<int> _pidOutMin(dashboard, YASOLR_LBL_164);
static dash::InputCard<int> _pidOutMax(dashboard, YASOLR_LBL_165);
static dash::InputCard<int> _pidSetpoint(dashboard, YASOLR_LBL_163);
static dash::InputCard<float, 4> _pidKp(dashboard, YASOLR_LBL_166);
static dash::InputCard<float, 4> _pidKi(dashboard, YASOLR_LBL_167);
static dash::InputCard<float, 4> _pidKd(dashboard, YASOLR_LBL_168);
static dash::ToggleButtonCard _pidView(dashboard, YASOLR_LBL_169);
static dash::LineChart<int8_t, int16_t> _pidInputHistory(dashboard, YASOLR_LBL_170);
static dash::LineChart<int8_t, int16_t> _pidOutputHistory(dashboard, YASOLR_LBL_171);
static dash::LineChart<int8_t, int16_t> _pidErrorHistory(dashboard, YASOLR_LBL_172);
static dash::BarChart<int8_t, int16_t> _pidSumHistory(dashboard, YASOLR_LBL_173);
static dash::LineChart<int8_t, int16_t> _pidPTermHistory(dashboard, YASOLR_LBL_174);
static dash::LineChart<int8_t, int16_t> _pidITermHistory(dashboard, YASOLR_LBL_175);
static dash::LineChart<int8_t, int16_t> _pidDTermHistory(dashboard, YASOLR_LBL_176);

// tab: gpio

static dash::SeparatorCard<const char*> _gpioTitle(dashboard, YASOLR_LBL_126);
static dash::SeparatorCard<const char*> _gpioSep4(dashboard, YASOLR_LBL_012);
static dash::FeedbackInputCard<int32_t> _pinZCD(dashboard, YASOLR_LBL_125);
static dash::DropdownCard<const char*> _serialJsy(dashboard, YASOLR_LBL_150, YASOLR_UART_CHOICES);
static dash::FeedbackInputCard<int32_t> _pinJsyRX(dashboard, YASOLR_LBL_116);
static dash::FeedbackInputCard<int32_t> _pinJsyTX(dashboard, YASOLR_LBL_117);
static dash::DropdownCard<const char*> _serialPZEM(dashboard, YASOLR_LBL_152, YASOLR_UART_CHOICES);
static dash::FeedbackInputCard<int32_t> _pinPZEMRX(dashboard, YASOLR_LBL_121);
static dash::FeedbackInputCard<int32_t> _pinPZEMTX(dashboard, YASOLR_LBL_122);
static dash::SeparatorCard<const char*> _gpioSep1(dashboard, YASOLR_LBL_046);
static dash::FeedbackInputCard<int32_t> _pinDimmerO1(dashboard, YASOLR_LBL_050);
static dash::FeedbackInputCard<int32_t> _pinRelayO1(dashboard, YASOLR_LBL_134);
static dash::FeedbackInputCard<int32_t> _pinDS18O1(dashboard, YASOLR_LBL_132);
static dash::SeparatorCard<const char*> _gpioSep2(dashboard, YASOLR_LBL_070);
static dash::FeedbackInputCard<int32_t> _pinDimmerO2(dashboard, YASOLR_LBL_050);
static dash::FeedbackInputCard<int32_t> _pinRelayO2(dashboard, YASOLR_LBL_134);
static dash::FeedbackInputCard<int32_t> _pinDS18O2(dashboard, YASOLR_LBL_132);
static dash::SeparatorCard<const char*> _gpioSep3(dashboard, YASOLR_LBL_071);
static dash::FeedbackInputCard<int32_t> _pinRelay1(dashboard, YASOLR_LBL_074);
static dash::FeedbackInputCard<int32_t> _pinRelay2(dashboard, YASOLR_LBL_077);
static dash::SeparatorCard<const char*> _gpioSep5(dashboard, YASOLR_LBL_078);
static dash::FeedbackInputCard<int32_t> _pinDS18Router(dashboard, YASOLR_LBL_132);
static dash::FeedbackInputCard<int32_t> _pinLEDGreen(dashboard, YASOLR_LBL_118);
static dash::FeedbackInputCard<int32_t> _pinLEDYellow(dashboard, YASOLR_LBL_120);
static dash::FeedbackInputCard<int32_t> _pinLEDRed(dashboard, YASOLR_LBL_119);
static dash::FeedbackInputCard<int32_t> _pinI2CSCL(dashboard, YASOLR_LBL_111);
static dash::FeedbackInputCard<int32_t> _pinI2CSDA(dashboard, YASOLR_LBL_112);

// tab: hardware

// grid
static dash::SeparatorCard<const char*> _gridSep(dashboard, YASOLR_LBL_012);
static dash::DropdownCard<const char*> _gridFreq(dashboard, YASOLR_LBL_141, "Auto-detect,50 Hz,60 Hz");
static dash::FeedbackToggleButtonCard _jsy(dashboard, YASOLR_LBL_128);
static dash::FeedbackToggleButtonCard _jsyRemote(dashboard, YASOLR_LBL_187);
static dash::FeedbackToggleButtonCard _zcd(dashboard, YASOLR_LBL_125);
static dash::FeedbackToggleButtonCard _victron(dashboard, YASOLR_LBL_195);
static dash::InputCard<const char*> _victronServer(dashboard, YASOLR_LBL_196);
static dash::InputCard<uint16_t> _victronPort(dashboard, YASOLR_LBL_197);

// output 1
static dash::SeparatorCard<const char*> _output1Sep(dashboard, YASOLR_LBL_046);

// output 1 dimmer
static dash::FeedbackToggleButtonCard _output1Dimmer(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_050);
static dash::DropdownCard<const char*> _output1DimmerType(dashboard, YASOLR_LBL_151, YASOLR_DIMMER_LSA_GP8211S "," YASOLR_DIMMER_LSA_GP8403 "," YASOLR_DIMMER_LSA_GP8413 "," YASOLR_DIMMER_LSA_PWM "," YASOLR_DIMMER_LSA_PWM_ZCD "," YASOLR_DIMMER_RANDOM_SSR "," YASOLR_DIMMER_ROBODYN "," YASOLR_DIMMER_TRIAC "," YASOLR_DIMMER_ZC_SSR);
static dash::RangeSliderCard<uint8_t> _output1DimmerMapper(dashboard, YASOLR_LBL_183, 0, 100, 1, "%");
static dash::FeedbackToggleButtonCard _output1PZEM(dashboard, YASOLR_LBL_133);
static dash::IndicatorButtonCard _output1PZEMSync(dashboard, YASOLR_LBL_147);

// output 1 bypass relay
static dash::FeedbackToggleButtonCard _output1Relay(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_134);
static dash::DropdownCard<const char*> _output1RelayType(dashboard, YASOLR_LBL_151, "NO,NC");

// output 1 ds18
static dash::FeedbackToggleButtonCard _output1DS18(dashboard, YASOLR_LBL_046 ": " YASOLR_LBL_132);

// output 2
static dash::SeparatorCard<const char*> _output2Sep(dashboard, YASOLR_LBL_070);

// output 2 dimmer
static dash::FeedbackToggleButtonCard _output2Dimmer(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_050);
static dash::DropdownCard<const char*> _output2DimmerType(dashboard, YASOLR_LBL_151, YASOLR_DIMMER_LSA_GP8211S "," YASOLR_DIMMER_LSA_GP8403 "," YASOLR_DIMMER_LSA_GP8413 "," YASOLR_DIMMER_LSA_PWM "," YASOLR_DIMMER_LSA_PWM_ZCD "," YASOLR_DIMMER_RANDOM_SSR "," YASOLR_DIMMER_ROBODYN "," YASOLR_DIMMER_TRIAC "," YASOLR_DIMMER_ZC_SSR);
static dash::RangeSliderCard<uint8_t> _output2DimmerMapper(dashboard, YASOLR_LBL_183, 0, 100, 1, "%");
static dash::FeedbackToggleButtonCard _output2PZEM(dashboard, YASOLR_LBL_133);
static dash::IndicatorButtonCard _output2PZEMSync(dashboard, YASOLR_LBL_148);

// output 2 bypass relay
static dash::FeedbackToggleButtonCard _output2Relay(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_134);
static dash::DropdownCard<const char*> _output2RelayType(dashboard, YASOLR_LBL_151, "NO,NC");

// output 2 ds18
static dash::FeedbackToggleButtonCard _output2DS18(dashboard, YASOLR_LBL_070 ": " YASOLR_LBL_132);

// Relays
static dash::SeparatorCard<const char*> _relaySep(dashboard, YASOLR_LBL_071);

// relay1
static dash::FeedbackToggleButtonCard _relay1(dashboard, YASOLR_LBL_074);
static dash::DropdownCard<const char*> _relay1Type(dashboard, YASOLR_LBL_151, "NO,NC");
static dash::InputCard<uint16_t> _relay1Load(dashboard, YASOLR_LBL_072);
static dash::PercentageSliderCard _relay1Tolerance(dashboard, YASOLR_LBL_198);

// relay2
static dash::FeedbackToggleButtonCard _relay2(dashboard, YASOLR_LBL_077);
static dash::DropdownCard<const char*> _relay2Type(dashboard, YASOLR_LBL_151, "NO,NC");
static dash::InputCard<uint16_t> _relay2Load(dashboard, YASOLR_LBL_075);
static dash::PercentageSliderCard _relay2Tolerance(dashboard, YASOLR_LBL_199);

// System
static dash::SeparatorCard<const char*> _routerSep(dashboard, YASOLR_LBL_078);

// router ds18
static dash::FeedbackToggleButtonCard _routerDS18(dashboard, YASOLR_LBL_078 ": " YASOLR_LBL_132);

// router led
static dash::FeedbackToggleButtonCard _led(dashboard, YASOLR_LBL_078 ": " YASOLR_LBL_129);

// display
static dash::FeedbackToggleButtonCard _display(dashboard, YASOLR_LBL_078 ": " YASOLR_LBL_127);
static dash::DropdownCard<const char*> _displayType(dashboard, YASOLR_LBL_143, "SH1106,SH1107,SSD1306");
static dash::DropdownCard<uint16_t> _displayRotation(dashboard, YASOLR_LBL_144, "0,90,180,270");
static dash::SliderCard<uint8_t> _displaySpeed(dashboard, YASOLR_LBL_142, 1, 10, 1, "s");

// tab: output 1 config

static dash::SeparatorCard<const char*> _output1ConfigSep0(dashboard, YASOLR_LBL_140);
static dash::FeedbackInputCard<float, 2> _output1ResistanceInput(dashboard, YASOLR_LBL_145);
static dash::IndicatorButtonCard _output1ResistanceCalibration(dashboard, YASOLR_LBL_186);
static dash::SeparatorCard<const char*> _output1ConfigSep1(dashboard, YASOLR_LBL_136);
static dash::PercentageSliderCard _output1DimmerDutyLimiter(dashboard, YASOLR_LBL_062);
static dash::InputCard<uint8_t> _output1DimmerTempLimiter(dashboard, YASOLR_LBL_063);
static dash::InputCard<uint16_t> _output1DimmerExcessLimiter(dashboard, YASOLR_LBL_061);
static dash::SeparatorCard<const char*> _output1ConfigSep2(dashboard, YASOLR_LBL_137);
static dash::InputCard<uint8_t> _output1AutoStartTemp(dashboard, YASOLR_LBL_065);
static dash::InputCard<uint8_t> _output1AutoStoptTemp(dashboard, YASOLR_LBL_066);
static dash::InputCard<const char*> _output1AutoStartTime(dashboard, YASOLR_LBL_067);
static dash::InputCard<const char*> _output1AutoStoptTime(dashboard, YASOLR_LBL_068);
static dash::WeekCard<const char*> _output1AutoStartWDays(dashboard, YASOLR_LBL_069);
static dash::SliderCard<float, 1> _output1BypassTimeout(dashboard, YASOLR_LBL_200, 0.0f, 24.0f, 0.5f, "h");

// tab: output 2 config

static dash::SeparatorCard<const char*> _output2ConfigSep0(dashboard, YASOLR_LBL_140);
static dash::FeedbackInputCard<float, 2> _output2ResistanceInput(dashboard, YASOLR_LBL_145);
static dash::IndicatorButtonCard _output2ResistanceCalibration(dashboard, YASOLR_LBL_186);
static dash::SeparatorCard<const char*> _output2ConfigSep1(dashboard, YASOLR_LBL_136);
static dash::PercentageSliderCard _output2DimmerDutyLimiter(dashboard, YASOLR_LBL_062);
static dash::InputCard<uint8_t> _output2DimmerTempLimiter(dashboard, YASOLR_LBL_063);
static dash::InputCard<uint16_t> _output2DimmerExcessLimiter(dashboard, YASOLR_LBL_061);
static dash::SeparatorCard<const char*> _output2ConfigSep2(dashboard, YASOLR_LBL_137);
static dash::InputCard<uint8_t> _output2AutoStartTemp(dashboard, YASOLR_LBL_065);
static dash::InputCard<uint8_t> _output2AutoStoptTemp(dashboard, YASOLR_LBL_066);
static dash::InputCard<const char*> _output2AutoStartTime(dashboard, YASOLR_LBL_067);
static dash::InputCard<const char*> _output2AutoStoptTime(dashboard, YASOLR_LBL_068);
static dash::WeekCard<const char*> _output2AutoStartWDays(dashboard, YASOLR_LBL_069);
static dash::SliderCard<float, 1> _output2BypassTimeout(dashboard, YASOLR_LBL_200, 0.0f, 24.0f, 0.5f, "h");
#endif

static void _onChangeResistanceCalibration() {
  if (!router.isCalibrationRunning()) {
    config.set(KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_FALSE, false);
    config.set(KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_FALSE, false);
    config.set(KEY_OUTPUT1_DIMMER_LIMIT, "100", false);
    config.set(KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_FALSE, false);
    config.set(KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_FALSE, false);
    config.set(KEY_OUTPUT2_DIMMER_LIMIT, "100", false);

    router.beginCalibration([]() {
      if (output1)
        config.set(KEY_OUTPUT1_RESISTANCE, Mycila::string::to_string(output1->config.calibratedResistance, 2));
      if (output2)
        config.set(KEY_OUTPUT2_RESISTANCE, Mycila::string::to_string(output2->config.calibratedResistance, 2));
    });

    // because we set false to trigger events
    dashboardInitTask.resume();
    if (mqttPublishConfigTask)
      mqttPublishConfigTask->resume();
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  }

  dashboard.refresh(_output1ResistanceCalibration);
  dashboard.refresh(_output2ResistanceCalibration);
}

void YaSolR::Website::begin() {
  LOGI(TAG, "Initialize dashboard layout");

  for (int i = 0; i < YASOLR_GRAPH_POINTS; i++)
    _historyX[i] = i - YASOLR_GRAPH_POINTS;

  // home

  if (relay1)
    _relaySwitch(_relay1Switch, *relay1);
  if (relay2)
    _relaySwitch(_relay2Switch, *relay2);
  if (output1) {
    _outputBypassSwitch(_output1Bypass, *output1);
    _outputDimmerSlider(_output1DimmerSlider, *output1);
  }
  if (output2) {
    _outputBypassSwitch(_output2Bypass, *output2);
    _outputDimmerSlider(_output2DimmerSlider, *output2);
  }

  _gridPowerHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.setX(_historyX, YASOLR_GRAPH_POINTS);

#if APP_MODEL_PRO
  _output1ResistanceCalibration.onPush(_onChangeResistanceCalibration);
  _output2ResistanceCalibration.onPush(_onChangeResistanceCalibration);

  _output1PZEMSync.onPush([]() {
    if (pzemO1PairingTask) {
      if (pzemO1PairingTask->running())
        return;
      if (!pzemO1PairingTask->scheduled())
        pzemO1PairingTask->resume();
    }
    _output1PZEMSync.setIndicator(pzemO1PairingTask && (pzemO1PairingTask->scheduled() || pzemO1PairingTask->running()), dash::Status::WARNING);
    dashboard.refresh(_output1PZEMSync);
  });

  _output2PZEMSync.onPush([]() {
    if (pzemO2PairingTask)
      if (pzemO2PairingTask->running())
        return;
    if (!pzemO2PairingTask->scheduled())
      pzemO2PairingTask->resume();
    _output2PZEMSync.setIndicator(pzemO2PairingTask && (pzemO2PairingTask->scheduled() || pzemO2PairingTask->running()), dash::Status::WARNING);
    dashboard.refresh(_output2PZEMSync);
  });
#else
  _output1ResistanceCalibration.onChange([](bool value) { if (value) _onChangeResistanceCalibration(); });
  _output2ResistanceCalibration.onChange([](bool value) { if (value) _onChangeResistanceCalibration(); });

  _output1PZEMSync.onChange([](bool value) {
    if (pzemO1PairingTask) {
      if (pzemO1PairingTask->running())
        return;
      if (!pzemO1PairingTask->scheduled())
        pzemO1PairingTask->resume();
    }
    _output1PZEMSync.setValue(pzemO1PairingTask && (pzemO1PairingTask->scheduled() || pzemO1PairingTask->running()));
    dashboard.refresh(_output1PZEMSync);
  });

  _output2PZEMSync.onChange([](bool value) {
    if (pzemO2PairingTask) {
      if (pzemO2PairingTask->running())
        return;
      if (!pzemO2PairingTask->scheduled())
        pzemO2PairingTask->resume();
    }
    _output2PZEMSync.setValue(pzemO2PairingTask && (pzemO2PairingTask->scheduled() || pzemO2PairingTask->running()));
    dashboard.refresh(_output2PZEMSync);
  });
#endif

#ifdef APP_MODEL_PRO
  dashboard.setChartAnimations(false);

  // tab: home

  _gridPowerHistory.setSize(FULL_SIZE);
  _routedPowerHistory.setSize(FULL_SIZE);
  _routerTHDiHistory.setSize(FULL_SIZE);

  // tab: output 1

  _output1DS18State.setTab(_output1Tab);
  _output1DimmerAuto.setTab(_output1Tab);
  _output1DimmerSlider.setTab(_output1Tab);
  _output1DimmerSliderRO.setTab(_output1Tab);
  _output1Power.setTab(_output1Tab);
  _output1ApparentPower.setTab(_output1Tab);
  _output1PowerFactor.setTab(_output1Tab);
  _output1THDi.setTab(_output1Tab);
  _output1Voltage.setTab(_output1Tab);
  _output1Current.setTab(_output1Tab);
  _output1Resistance.setTab(_output1Tab);
  _output1Energy.setTab(_output1Tab);
  _output1BypassAuto.setTab(_output1Tab);
  _output1Bypass.setTab(_output1Tab);

  _boolConfig(_output1DimmerAuto, KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  _boolConfig(_output1BypassAuto, KEY_ENABLE_OUTPUT1_AUTO_BYPASS);

  // tab: output 2

  _output2DS18State.setTab(_output2Tab);
  _output2DimmerAuto.setTab(_output2Tab);
  _output2DimmerSlider.setTab(_output2Tab);
  _output2DimmerSliderRO.setTab(_output2Tab);
  _output2Power.setTab(_output2Tab);
  _output2ApparentPower.setTab(_output2Tab);
  _output2PowerFactor.setTab(_output2Tab);
  _output2THDi.setTab(_output2Tab);
  _output2Voltage.setTab(_output2Tab);
  _output2Current.setTab(_output2Tab);
  _output2Resistance.setTab(_output2Tab);
  _output2Energy.setTab(_output2Tab);
  _output2BypassAuto.setTab(_output2Tab);
  _output2Bypass.setTab(_output2Tab);

  _boolConfig(_output2DimmerAuto, KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  _boolConfig(_output2BypassAuto, KEY_ENABLE_OUTPUT2_AUTO_BYPASS);

  // tab: system

  _configBackup.setTab(_systemTab);
  _configRestore.setTab(_systemTab);
  _restart.setTab(_systemTab);
  _safeBoot.setTab(_systemTab);
  _reset.setTab(_systemTab);
  _energyReset.setTab(_systemTab);
  _safebootUpload.setTab(_systemTab);
  _safebootUploadStatus.setTab(_systemTab);

  _restart.onPush([]() { restartTask.resume(); });
  _safeBoot.onPush([]() { safeBootTask.resume(); });
  _reset.onPush([]() { resetTask.resume(); });
  _energyReset.onPush([]() {
    if (jsy)
      jsy->resetEnergy();
    if (pzemO1)
      pzemO1->resetEnergy();
    if (pzemO2)
      pzemO2->resetEnergy();
  });

  // tab: debug

  _debugMode.setTab(_debugTab);
  _debugInfo.setTab(_debugTab);
  _startupLogs.setTab(_debugTab);
  _consoleLink.setTab(_debugTab);

  _boolConfig(_debugMode, KEY_ENABLE_DEBUG);

  // tab: network

  _adminPwd.setTab(_networkTab);
  _wifiSSID.setTab(_networkTab);
  _wifiBSSID.setTab(_networkTab);
  _wifiPwd.setTab(_networkTab);
  _apMode.setTab(_networkTab);
  _staticIP.setTab(_networkTab);
  _subnetMask.setTab(_networkTab);
  _gateway.setTab(_networkTab);
  _dnsServer.setTab(_networkTab);
  _hostnameWidget.setTab(_networkTab);

  _passwordConfig(_adminPwd, KEY_ADMIN_PASSWORD);
  _boolConfig(_apMode, KEY_ENABLE_AP_MODE);
  _textConfig(_wifiSSID, KEY_WIFI_SSID);
  _textConfig(_wifiBSSID, KEY_WIFI_BSSID);
  _passwordConfig(_wifiPwd, KEY_WIFI_PASSWORD);
  _textConfig(_staticIP, KEY_NET_IP);
  _textConfig(_subnetMask, KEY_NET_SUBNET);
  _textConfig(_gateway, KEY_NET_GATEWAY);
  _textConfig(_dnsServer, KEY_NET_DNS);
  _hostnameWidget.onChange([](const std::optional<std::string>& value) {
    config.set(KEY_HOSTNAME, value.value_or(""));
    _hostnameWidget.setValue(config.get(KEY_HOSTNAME));
    dashboard.refresh(_hostnameWidget);
  });

  // tab: ntp

  _ntpServer.setTab(_ntpTab);
  _ntpTimezone.setTab(_ntpTab);
  _ntpSync.setTab(_ntpTab);

  _textConfig(_ntpServer, KEY_NTP_SERVER);

  _ntpTimezone.onChange([](const char* value) {
    config.set(KEY_NTP_TIMEZONE, value);
    _ntpTimezone.setValue(value);
    dashboard.refresh(_ntpTimezone);
  });

  _ntpSync.onSync([](const timeval& tv) {
    Mycila::NTP.sync(tv);
  });

  // tab: mqtt

  _mqtt.setTab(_mqttTab);
  _mqttServer.setTab(_mqttTab);
  _mqttPort.setTab(_mqttTab);
  _mqttUser.setTab(_mqttTab);
  _mqttPwd.setTab(_mqttTab);
  _mqttSecured.setTab(_mqttTab);
  _mqttServerCert.setTab(_mqttTab);
  _mqttServerCertDelete.setTab(_mqttTab);
  _mqttTopic.setTab(_mqttTab);
  _mqttPublishInterval.setTab(_mqttTab);
  _haSep.setTab(_mqttTab);
  _haDiscovery.setTab(_mqttTab);
  _haDiscoveryTopic.setTab(_mqttTab);
  _mqttSep1.setTab(_mqttTab);
  _mqttGridVoltage.setTab(_mqttTab);
  _mqttGridPower.setTab(_mqttTab);
  _mqttTempO1.setTab(_mqttTab);
  _mqttTempO2.setTab(_mqttTab);

  _mqtt.setSize(FULL_SIZE);

  _boolConfig(_mqtt, KEY_ENABLE_MQTT);
  _textConfig(_mqttServer, KEY_MQTT_SERVER);
  _numConfig(_mqttPort, KEY_MQTT_PORT);
  _textConfig(_mqttUser, KEY_MQTT_USERNAME);
  _passwordConfig(_mqttPwd, KEY_MQTT_PASSWORD);
  _boolConfig(_mqttSecured, KEY_MQTT_SECURED);
  _textConfig(_mqttTopic, KEY_MQTT_TOPIC);
  _sliderConfig(_mqttPublishInterval, KEY_MQTT_PUBLISH_INTERVAL);
  _textConfig(_mqttGridVoltage, KEY_GRID_VOLTAGE_MQTT_TOPIC);
  _textConfig(_mqttGridPower, KEY_GRID_POWER_MQTT_TOPIC);
  _textConfig(_mqttTempO1, KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  _textConfig(_mqttTempO2, KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  _boolConfig(_haDiscovery, KEY_ENABLE_HA_DISCOVERY);
  _textConfig(_haDiscoveryTopic, KEY_HA_DISCOVERY_TOPIC);

  _mqttServerCertDelete.onPush([this]() {
    if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE) && LittleFS.remove(YASOLR_MQTT_SERVER_CERT_FILE)) {
      LOGW(TAG, "MQTT server certificate deleted successfully!");
      dashboardInitTask.resume();
    }
  });

  // tab: pid

  _pidInputHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidOutputHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidErrorHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidSumHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidPTermHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidITermHistory.setX(_historyX, YASOLR_GRAPH_POINTS);
  _pidDTermHistory.setX(_historyX, YASOLR_GRAPH_POINTS);

  _pidView.setTab(_pidTab);
  _pidPMode.setTab(_pidTab);
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

  _pidView.setSize(FULL_SIZE);
  _pidInputHistory.setSize(FULL_SIZE);
  _pidOutputHistory.setSize(FULL_SIZE);
  _pidErrorHistory.setSize(FULL_SIZE);
  _pidSumHistory.setSize(FULL_SIZE);
  _pidPTermHistory.setSize(FULL_SIZE);
  _pidITermHistory.setSize(FULL_SIZE);
  _pidDTermHistory.setSize(FULL_SIZE);

  _numConfig(_pidSetpoint, KEY_PID_SETPOINT);
  _numConfig(_pidKp, KEY_PID_KP);
  _numConfig(_pidKi, KEY_PID_KI);
  _numConfig(_pidKd, KEY_PID_KD);
  _numConfig(_pidOutMin, KEY_PID_OUT_MIN);
  _numConfig(_pidOutMax, KEY_PID_OUT_MAX);

  _pidView.onChange([this](bool value) {
    _pidView.setValue(value);
    if (value)
      resetPIDCharts();
    dashboard.refresh(_pidView);
    dashboardInitTask.resume();
  });

  _pidPMode.onChange([](const char* value) {
    if (strcmp(value, YASOLR_PID_P_MODE_1) == 0)
      config.set(KEY_PID_P_MODE, "1");
    else if (strcmp(value, YASOLR_PID_P_MODE_2) == 0)
      config.set(KEY_PID_P_MODE, "2");
    else
      config.unset(KEY_PID_P_MODE);
    _pidPMode.setValue(config.get(KEY_PID_P_MODE));
    dashboard.refresh(_pidPMode);
  });

  _pidICMode.onChange([](const char* value) {
    if (strcmp(value, YASOLR_PID_IC_MODE_1) == 0)
      config.set(KEY_PID_IC_MODE, "1");
    else if (strcmp(value, YASOLR_PID_IC_MODE_2) == 0)
      config.set(KEY_PID_IC_MODE, "2");
    else
      config.unset(KEY_PID_IC_MODE);
    _pidICMode.setValue(config.get(KEY_PID_IC_MODE));
    dashboard.refresh(_pidICMode);
  });

  // tab: gpio

  _gpioTitle.setTab(_gpioTab);
  _gpioSep1.setTab(_gpioTab);
  _pinDimmerO1.setTab(_gpioTab);
  _pinDS18O1.setTab(_gpioTab);
  _pinRelayO1.setTab(_gpioTab);
  _gpioSep2.setTab(_gpioTab);
  _pinDimmerO2.setTab(_gpioTab);
  _pinDS18O2.setTab(_gpioTab);
  _pinRelayO2.setTab(_gpioTab);
  _gpioSep3.setTab(_gpioTab);
  _pinRelay1.setTab(_gpioTab);
  _pinRelay2.setTab(_gpioTab);
  _gpioSep4.setTab(_gpioTab);
  _serialJsy.setTab(_gpioTab);
  _pinJsyRX.setTab(_gpioTab);
  _pinJsyTX.setTab(_gpioTab);
  _serialPZEM.setTab(_gpioTab);
  _pinPZEMRX.setTab(_gpioTab);
  _pinPZEMTX.setTab(_gpioTab);
  _pinZCD.setTab(_gpioTab);
  _gpioSep5.setTab(_gpioTab);
  _pinI2CSCL.setTab(_gpioTab);
  _pinI2CSDA.setTab(_gpioTab);
  _pinDS18Router.setTab(_gpioTab);
  _pinLEDGreen.setTab(_gpioTab);
  _pinLEDRed.setTab(_gpioTab);
  _pinLEDYellow.setTab(_gpioTab);

  _gpioTitle.setSubtitle(YASOLR_LBL_131);

  _numConfig(_pinDimmerO1, KEY_PIN_OUTPUT1_DIMMER);
  _numConfig(_pinDS18O1, KEY_PIN_OUTPUT1_DS18);
  _numConfig(_pinRelayO1, KEY_PIN_OUTPUT1_RELAY);
  _numConfig(_pinDimmerO2, KEY_PIN_OUTPUT2_DIMMER);
  _numConfig(_pinDS18O2, KEY_PIN_OUTPUT2_DS18);
  _numConfig(_pinRelayO2, KEY_PIN_OUTPUT2_RELAY);
  _numConfig(_pinRelay1, KEY_PIN_RELAY1);
  _numConfig(_pinRelay2, KEY_PIN_RELAY2);
  _numConfig(_pinJsyRX, KEY_PIN_JSY_RX);
  _numConfig(_pinJsyTX, KEY_PIN_JSY_TX);
  _numConfig(_pinPZEMRX, KEY_PIN_PZEM_RX);
  _numConfig(_pinPZEMTX, KEY_PIN_PZEM_TX);
  _numConfig(_pinZCD, KEY_PIN_ZCD);
  _numConfig(_pinI2CSCL, KEY_PIN_I2C_SCL);
  _numConfig(_pinI2CSDA, KEY_PIN_I2C_SDA);
  _numConfig(_pinDS18Router, KEY_PIN_ROUTER_DS18);
  _numConfig(_pinLEDGreen, KEY_PIN_LIGHTS_GREEN);
  _numConfig(_pinLEDRed, KEY_PIN_LIGHTS_RED);
  _numConfig(_pinLEDYellow, KEY_PIN_LIGHTS_YELLOW);

  _textConfig(_serialJsy, KEY_JSY_UART);
  _textConfig(_serialPZEM, KEY_PZEM_UART);

  // tab: hardware

  // grid
  _gridSep.setTab(_hardwareConfigTab);
  _gridFreq.setTab(_hardwareConfigTab);
  _jsy.setTab(_hardwareConfigTab);
  _jsyRemote.setTab(_hardwareConfigTab);
  _zcd.setTab(_hardwareConfigTab);
  _victron.setTab(_hardwareConfigTab);
  _victronServer.setTab(_hardwareConfigTab);
  _victronPort.setTab(_hardwareConfigTab);

  _boolConfig(_jsy, KEY_ENABLE_JSY);
  _boolConfig(_jsyRemote, KEY_ENABLE_JSY_REMOTE);
  _boolConfig(_zcd, KEY_ENABLE_ZCD);
  _boolConfig(_victron, KEY_ENABLE_VICTRON_MODBUS);
  _textConfig(_victronServer, KEY_VICTRON_MODBUS_SERVER);
  _numConfig(_victronPort, KEY_VICTRON_MODBUS_PORT);

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

  _output1Sep.setTab(_hardwareConfigTab);

  // output 1 dimmer
  _output1Dimmer.setTab(_hardwareConfigTab);
  _output1Dimmer.setSize(FULL_SIZE);
  _output1DimmerType.setTab(_hardwareConfigTab);
  _output1DimmerMapper.setTab(_hardwareConfigTab);
  _output1PZEM.setTab(_hardwareConfigTab);
  _output1PZEMSync.setTab(_hardwareConfigTab);
  _boolConfig(_output1Dimmer, KEY_ENABLE_OUTPUT1_DIMMER);
  _textConfig(_output1DimmerType, KEY_OUTPUT1_DIMMER_TYPE);
  _rangeConfig(_output1DimmerMapper, KEY_OUTPUT1_DIMMER_MIN, KEY_OUTPUT1_DIMMER_MAX);
  _boolConfig(_output1PZEM, KEY_ENABLE_OUTPUT1_PZEM);

  // output 1 bypass relay
  _output1Relay.setTab(_hardwareConfigTab);
  _output1Relay.setSize(FULL_SIZE);
  _output1RelayType.setTab(_hardwareConfigTab);
  _boolConfig(_output1Relay, KEY_ENABLE_OUTPUT1_RELAY);
  _textConfig(_output1RelayType, KEY_OUTPUT1_RELAY_TYPE);

  // output 1 ds18
  _output1DS18.setTab(_hardwareConfigTab);
  _output1DS18.setSize(FULL_SIZE);
  _boolConfig(_output1DS18, KEY_ENABLE_OUTPUT1_DS18);

  _output2Sep.setTab(_hardwareConfigTab);

  // output 2 dimmer
  _output2Dimmer.setTab(_hardwareConfigTab);
  _output2Dimmer.setSize(FULL_SIZE);
  _output2DimmerType.setTab(_hardwareConfigTab);
  _output2DimmerMapper.setTab(_hardwareConfigTab);
  _output2PZEM.setTab(_hardwareConfigTab);
  _output2PZEMSync.setTab(_hardwareConfigTab);
  _boolConfig(_output2Dimmer, KEY_ENABLE_OUTPUT2_DIMMER);
  _textConfig(_output2DimmerType, KEY_OUTPUT2_DIMMER_TYPE);
  _rangeConfig(_output2DimmerMapper, KEY_OUTPUT2_DIMMER_MIN, KEY_OUTPUT2_DIMMER_MAX);
  _boolConfig(_output2PZEM, KEY_ENABLE_OUTPUT2_PZEM);

  // output 2 bypass relay
  _output2Relay.setTab(_hardwareConfigTab);
  _output2Relay.setSize(FULL_SIZE);
  _output2RelayType.setTab(_hardwareConfigTab);
  _boolConfig(_output2Relay, KEY_ENABLE_OUTPUT2_RELAY);
  _textConfig(_output2RelayType, KEY_OUTPUT2_RELAY_TYPE);

  // output 2 ds18
  _output2DS18.setTab(_hardwareConfigTab);
  _output2DS18.setSize(FULL_SIZE);
  _boolConfig(_output2DS18, KEY_ENABLE_OUTPUT2_DS18);

  _relaySep.setTab(_hardwareConfigTab);

  // relay1
  _relay1.setTab(_hardwareConfigTab);
  _relay1.setSize(FULL_SIZE);
  _relay1Type.setTab(_hardwareConfigTab);
  _relay1Load.setTab(_hardwareConfigTab);
  _relay1Tolerance.setTab(_hardwareConfigTab);
  _boolConfig(_relay1, KEY_ENABLE_RELAY1);
  _numConfig(_relay1Load, KEY_RELAY1_LOAD);
  _textConfig(_relay1Type, KEY_RELAY1_TYPE);
  _sliderConfig(_relay1Tolerance, KEY_RELAY1_TOLERANCE);

  // relay2
  _relay2.setTab(_hardwareConfigTab);
  _relay2.setSize(FULL_SIZE);
  _relay2Type.setTab(_hardwareConfigTab);
  _relay2Load.setTab(_hardwareConfigTab);
  _relay2Tolerance.setTab(_hardwareConfigTab);
  _boolConfig(_relay2, KEY_ENABLE_RELAY2);
  _textConfig(_relay2Type, KEY_RELAY2_TYPE);
  _numConfig(_relay2Load, KEY_RELAY2_LOAD);
  _sliderConfig(_relay2Tolerance, KEY_RELAY2_TOLERANCE);

  _routerSep.setTab(_hardwareConfigTab);

  // router ds18
  _routerDS18.setTab(_hardwareConfigTab);
  _routerDS18.setSize(FULL_SIZE);
  _boolConfig(_routerDS18, KEY_ENABLE_DS18_SYSTEM);

  // router led
  _led.setTab(_hardwareConfigTab);
  _led.setSize(FULL_SIZE);
  _boolConfig(_led, KEY_ENABLE_LIGHTS);

  // display
  _display.setTab(_hardwareConfigTab);
  _display.setSize(FULL_SIZE);
  _displayType.setTab(_hardwareConfigTab);
  _displayRotation.setTab(_hardwareConfigTab);
  _displaySpeed.setTab(_hardwareConfigTab);
  _boolConfig(_display, KEY_ENABLE_DISPLAY);
  _textConfig(_displayType, KEY_DISPLAY_TYPE);
  _numConfig(_displayRotation, KEY_DISPLAY_ROTATION);
  _sliderConfig(_displaySpeed, KEY_DISPLAY_SPEED);

  // tab: output 1 config

  _output1ConfigSep0.setTab(_output1ConfigTab);
  _output1ResistanceInput.setTab(_output1ConfigTab);
  _output1ResistanceCalibration.setTab(_output1ConfigTab);
  _output1ConfigSep1.setTab(_output1ConfigTab);
  _output1DimmerDutyLimiter.setTab(_output1ConfigTab);
  _output1DimmerTempLimiter.setTab(_output1ConfigTab);
  _output1DimmerExcessLimiter.setTab(_output1ConfigTab);
  _output1ConfigSep2.setTab(_output1ConfigTab);
  _output1AutoStartTemp.setTab(_output1ConfigTab);
  _output1AutoStoptTemp.setTab(_output1ConfigTab);
  _output1AutoStartTime.setTab(_output1ConfigTab);
  _output1AutoStoptTime.setTab(_output1ConfigTab);
  _output1AutoStartWDays.setTab(_output1ConfigTab);
  _output1BypassTimeout.setTab(_output1ConfigTab);
  _numConfig(_output1ResistanceInput, KEY_OUTPUT1_RESISTANCE);
  _sliderConfig(_output1DimmerDutyLimiter, KEY_OUTPUT1_DIMMER_LIMIT);
  _numConfig(_output1DimmerTempLimiter, KEY_OUTPUT1_DIMMER_TEMP_LIMITER);
  _numConfig(_output1DimmerExcessLimiter, KEY_OUTPUT1_EXCESS_LIMITER);
  _numConfig(_output1AutoStartTemp, KEY_OUTPUT1_TEMPERATURE_START);
  _numConfig(_output1AutoStoptTemp, KEY_OUTPUT1_TEMPERATURE_STOP);
  _textConfig(_output1AutoStartTime, KEY_OUTPUT1_TIME_START);
  _textConfig(_output1AutoStoptTime, KEY_OUTPUT1_TIME_STOP);
  _daysConfig(_output1AutoStartWDays, KEY_OUTPUT1_DAYS);
  _output1BypassTimeout.onChange([](float value) {
    _output1BypassTimeout.setValue(value);
    uint16_t seconds = static_cast<uint16_t>(value * 3600.0f);
    config.set(KEY_OUTPUT1_BYPASS_TIMEOUT, std::to_string(seconds));
    dashboard.refresh(_output1BypassTimeout);
  });

  // tab: output 2 config

  _output2ConfigSep0.setTab(_output2ConfigTab);
  _output2ResistanceInput.setTab(_output2ConfigTab);
  _output2ResistanceCalibration.setTab(_output2ConfigTab);
  _output2ConfigSep1.setTab(_output2ConfigTab);
  _output2DimmerDutyLimiter.setTab(_output2ConfigTab);
  _output2DimmerTempLimiter.setTab(_output2ConfigTab);
  _output2DimmerExcessLimiter.setTab(_output2ConfigTab);
  _output2ConfigSep2.setTab(_output2ConfigTab);
  _output2AutoStartTemp.setTab(_output2ConfigTab);
  _output2AutoStoptTemp.setTab(_output2ConfigTab);
  _output2AutoStartTime.setTab(_output2ConfigTab);
  _output2AutoStoptTime.setTab(_output2ConfigTab);
  _output2AutoStartWDays.setTab(_output2ConfigTab);
  _output2BypassTimeout.setTab(_output2ConfigTab);
  _numConfig(_output2ResistanceInput, KEY_OUTPUT2_RESISTANCE);
  _sliderConfig(_output2DimmerDutyLimiter, KEY_OUTPUT2_DIMMER_LIMIT);
  _numConfig(_output2DimmerTempLimiter, KEY_OUTPUT2_DIMMER_TEMP_LIMITER);
  _numConfig(_output2DimmerExcessLimiter, KEY_OUTPUT2_EXCESS_LIMITER);
  _numConfig(_output2AutoStartTemp, KEY_OUTPUT2_TEMPERATURE_START);
  _numConfig(_output2AutoStoptTemp, KEY_OUTPUT2_TEMPERATURE_STOP);
  _textConfig(_output2AutoStartTime, KEY_OUTPUT2_TIME_START);
  _textConfig(_output2AutoStoptTime, KEY_OUTPUT2_TIME_STOP);
  _daysConfig(_output2AutoStartWDays, KEY_OUTPUT2_DAYS);
  _output2BypassTimeout.onChange([](float value) {
    _output2BypassTimeout.setValue(value);
    uint16_t seconds = static_cast<uint16_t>(value * 3600.0f);
    config.set(KEY_OUTPUT2_BYPASS_TIMEOUT, std::to_string(seconds));
    dashboard.refresh(_output2BypassTimeout);
  });
#endif

  initCards();
  updateCards();
}

void YaSolR::Website::initCards() {
  LOGI(TAG, "Initialize dashboard cards");

  const Mycila::ESPConnect::Mode mode = espConnect.getMode();

  // statistics

  _appName.setValue(Mycila::AppInfo.name.c_str());
  _appModel.setValue(Mycila::AppInfo.model.c_str());
  _appVersion.setValue(Mycila::AppInfo.version.c_str());
  _appManufacturer.setValue(Mycila::AppInfo.manufacturer.c_str());
  _deviceBootCount.setValue(Mycila::System::getBootCount());
  _deviceBootReason.setValue(Mycila::System::getLastRebootReason());
  _deviceCores.setValue(ESP.getChipCores());
  _deviceID.setValue(Mycila::AppInfo.id.c_str());
  _deviceModel.setValue(ESP.getChipModel());
  _firmwareBuildHash.setValue(Mycila::AppInfo.buildHash.c_str());
  _firmwareBuildTimestamp.setValue(Mycila::AppInfo.buildDate.c_str());
  _firmwareFilename.setValue(Mycila::AppInfo.firmware.c_str());
  _networkHostname.setValue(espConnect.getConfig().hostname.c_str());

  switch (mode) {
    case Mycila::ESPConnect::Mode::AP:
      _networkInterface.setValue("Access Point");
      _networkAPIP.setValue(espConnect.getIPAddress(Mycila::ESPConnect::Mode::AP).toString().c_str());
      _networkAPMAC.setValue(espConnect.getMACAddress(Mycila::ESPConnect::Mode::AP));
      break;

    case Mycila::ESPConnect::Mode::ETH: {
      _networkInterface.setValue("Ethernet");
      _networkEthIP.setValue(espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
      auto ipv6Local = espConnect.getLinkLocalIPv6Address(Mycila::ESPConnect::Mode::ETH);
      _networkEthIPv6Local.setValue(ipv6Local == IN6ADDR_ANY ? "N/A" : ipv6Local.toString().c_str());
      auto ipv6Global = espConnect.getGlobalIPv6Address(Mycila::ESPConnect::Mode::ETH);
      _networkEthIPv6Global.setValue(ipv6Global == IN6ADDR_ANY ? "N/A" : ipv6Global.toString().c_str());
      auto mac = espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH);
      _networkEthMAC.setValue(mac.empty() ? std::string("N/A") : mac);
      break;
    }

    case Mycila::ESPConnect::Mode::STA: {
      _networkInterface.setValue("WiFi");
      _networkWiFiIP.setValue(espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
      auto ipv6Local = espConnect.getLinkLocalIPv6Address(Mycila::ESPConnect::Mode::STA);
      _networkWiFiIPv6Local.setValue(ipv6Local == IN6ADDR_ANY ? "N/A" : ipv6Local.toString().c_str());
      auto ipv6Global = espConnect.getGlobalIPv6Address(Mycila::ESPConnect::Mode::STA);
      _networkWiFiIPv6Global.setValue(ipv6Global == IN6ADDR_ANY ? "N/A" : ipv6Global.toString().c_str());
      auto mac = espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA);
      _networkWiFiMAC.setValue(mac.empty() ? std::string("N/A") : mac);
      _networkWiFiSSID.setValue(espConnect.getWiFiSSID());
      _networkWiFiBSSID.setValue(espConnect.getWiFiBSSID());
      break;
    }

    default:
      _networkInterface.setValue("Unknown");
      break;
  }

  // home

  if (!output1) {
    // initialize values for OSS components which cannot be hidden
    _output1State.setFeedback("DISABLED", dash::Status::INFO);
    _output1DS18State.setValue(NAN);
    _output1DimmerSlider.setValue(0);
    _output1Bypass.setValue(false);
  }

  if (!output2) {
    // initialize values for OSS components which cannot be hidden
    _output2State.setFeedback("DISABLED", dash::Status::INFO);
    _output2DS18State.setValue(NAN);
    _output2DimmerSlider.setValue(0);
    _output2Bypass.setValue(false);
  }

#ifdef APP_MODEL_PRO
  const bool jsyEnabled = config.getBool(KEY_ENABLE_JSY);
  const bool mqttEnabled = config.getBool(KEY_ENABLE_MQTT);
  const bool pidViewEnabled = realTimePIDEnabled();
  const bool displayEnabled = config.getBool(KEY_ENABLE_DISPLAY);
  const bool serverCertExists = LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE);

  const bool dimmer1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_DIMMER);
  const bool output1RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT1_RELAY);
  const bool bypass1Possible = dimmer1Enabled || output1RelayEnabled;
  const bool autoDimmer1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  const bool autoBypass1Activated = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  const bool output1TempReceived = output1 && !output1->temperature().neverUpdated();
  const bool pzem1Enabled = config.getBool(KEY_ENABLE_OUTPUT1_PZEM);

  const bool dimmer2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_DIMMER);
  const bool output2RelayEnabled = config.getBool(KEY_ENABLE_OUTPUT2_RELAY);
  const bool bypass2Possible = dimmer2Enabled || output2RelayEnabled;
  const bool autoDimmer2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  const bool autoBypass2Activated = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  const bool output2TempReceived = output2 && !output2->temperature().neverUpdated();
  const bool pzem2Enabled = config.getBool(KEY_ENABLE_OUTPUT2_PZEM);

  const bool relay1Enabled = config.getBool(KEY_ENABLE_RELAY1);
  const bool relay2Enabled = config.getBool(KEY_ENABLE_RELAY2);

  const bool victronEnabled = config.getBool(KEY_ENABLE_VICTRON_MODBUS);

  // statistics

  _udpMessageRateBuffer.setDisplay(config.getBool(KEY_ENABLE_JSY_REMOTE));
  _networkAPIP.setDisplay(mode == Mycila::ESPConnect::Mode::AP);
  _networkAPMAC.setDisplay(mode == Mycila::ESPConnect::Mode::AP);
  _networkEthIP.setDisplay(mode == Mycila::ESPConnect::Mode::ETH);
  _networkEthIPv6Local.setDisplay(mode == Mycila::ESPConnect::Mode::ETH);
  _networkEthIPv6Global.setDisplay(mode == Mycila::ESPConnect::Mode::ETH);
  _networkEthMAC.setDisplay(mode == Mycila::ESPConnect::Mode::ETH);
  _networkWiFiIP.setDisplay(mode == Mycila::ESPConnect::Mode::STA);
  _networkWiFiIPv6Local.setDisplay(mode == Mycila::ESPConnect::Mode::STA);
  _networkWiFiIPv6Global.setDisplay(mode == Mycila::ESPConnect::Mode::STA);
  _networkWiFiMAC.setDisplay(mode == Mycila::ESPConnect::Mode::STA);
  _networkWiFiSSID.setDisplay(mode == Mycila::ESPConnect::Mode::STA);
  _networkWiFiBSSID.setDisplay(mode == Mycila::ESPConnect::Mode::STA);
  _output1RelaySwitchCount.setDisplay(output1 && output1->isBypassRelayEnabled());
  _output2RelaySwitchCount.setDisplay(output2 && output2->isBypassRelayEnabled());
  _relay1SwitchCount.setDisplay(relay1 && relay1->isEnabled());
  _relay2SwitchCount.setDisplay(relay2 && relay2->isEnabled());

  // tabs

  _output1Tab.setDisplay(dimmer1Enabled || output1RelayEnabled || output1TempReceived);
  _output2Tab.setDisplay(dimmer2Enabled || output2RelayEnabled || output2TempReceived);
  _output1ConfigTab.setDisplay(dimmer1Enabled || output1RelayEnabled);
  _output2ConfigTab.setDisplay(dimmer2Enabled || output2RelayEnabled);

  // overview

  _relay1Switch.setDisplay(relay1Enabled);
  _relay2Switch.setDisplay(relay2Enabled);

  // tab: output 1

  _output1DimmerAuto.setValue(autoDimmer1Activated);
  _output1BypassAuto.setValue(autoBypass1Activated);

  if (!output1) {
    _output1DimmerSliderRO.setValue(0);
    _output1Power.setValue(0);
    _output1ApparentPower.setValue(0);
    _output1PowerFactor.setValue(NAN);
    _output1THDi.setValue(NAN);
    _output1Voltage.setValue(0);
    _output1Current.setValue(0);
    _output1Resistance.setValue(NAN);
    _output1Energy.setValue(0);
  }

  _output1DimmerAuto.setDisplay(dimmer1Enabled);
  _output1DimmerSlider.setDisplay(dimmer1Enabled && !autoDimmer1Activated);
  _output1DimmerSliderRO.setDisplay(dimmer1Enabled && autoDimmer1Activated);
  _output1Power.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1ApparentPower.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1PowerFactor.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1THDi.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Voltage.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Current.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Resistance.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1Energy.setDisplay(dimmer1Enabled && pzem1Enabled);
  _output1BypassAuto.setDisplay(bypass1Possible);
  _output1Bypass.setDisplay(bypass1Possible);

  // tab: output 2

  _output2DimmerAuto.setValue(autoDimmer2Activated);
  _output2BypassAuto.setValue(autoBypass2Activated);

  if (!output2) {
    _output2DimmerSliderRO.setValue(0);
    _output2Power.setValue(0);
    _output2ApparentPower.setValue(0);
    _output2PowerFactor.setValue(NAN);
    _output2THDi.setValue(NAN);
    _output2Voltage.setValue(0);
    _output2Current.setValue(0);
    _output2Resistance.setValue(NAN);
    _output2Energy.setValue(0);
  }

  _output2DimmerAuto.setDisplay(dimmer2Enabled);
  _output2DimmerSlider.setDisplay(dimmer2Enabled && !autoDimmer2Activated);
  _output2DimmerSliderRO.setDisplay(dimmer2Enabled && autoDimmer2Activated);
  _output2Power.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2ApparentPower.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2PowerFactor.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2THDi.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Voltage.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Current.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Resistance.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2Energy.setDisplay(dimmer2Enabled && pzem2Enabled);
  _output2BypassAuto.setDisplay(bypass2Possible);
  _output2Bypass.setDisplay(bypass2Possible);

  // tab: system

  _configBackup.setValue("/api/config/backup");
  _configRestore.setValue("/api/config/restore");
  _safebootUpload.setValue("/api/safeboot/upload");
  _energyReset.setDisplay(jsyEnabled || pzem1Enabled || pzem2Enabled);
  _safebootUpload.setDisplay(true);
  _safebootUploadStatus.setDisplay(false);

  // tab: debug

  _debugMode.setValue(config.getBool(KEY_ENABLE_DEBUG));
  _debugInfo.setValue("/api/debug");
  _startupLogs.setValue("/api" YASOLR_LOG_FILE);
  _consoleLink.setValue("/console");

  // tab: network

  _hostnameWidget.setValue(config.get(KEY_HOSTNAME));
  _wifiSSID.setValue(config.get(KEY_WIFI_SSID));
  _wifiBSSID.setValue(config.get(KEY_WIFI_BSSID));
  _wifiPwd.setValue(config.get(KEY_WIFI_PASSWORD));
  _staticIP.setValue(config.get(KEY_NET_IP));
  _subnetMask.setValue(config.get(KEY_NET_SUBNET));
  _gateway.setValue(config.get(KEY_NET_GATEWAY));
  _dnsServer.setValue(config.get(KEY_NET_DNS));
  _adminPwd.setValue(config.get(KEY_ADMIN_PASSWORD));
  _apMode.setValue(config.getBool(KEY_ENABLE_AP_MODE));

  // tab: ntp

  _ntpServer.setValue(config.get(KEY_NTP_SERVER));
  _ntpTimezone.setValue(config.get(KEY_NTP_TIMEZONE));

  // tab: mqtt

  _mqttServer.setValue(config.get(KEY_MQTT_SERVER));
  _mqttServer.setDisplay(mqttEnabled);
  _mqttPort.setValue(config.getInt(KEY_MQTT_PORT));
  _mqttPort.setDisplay(mqttEnabled);
  _mqttUser.setValue(config.get(KEY_MQTT_USERNAME));
  _mqttUser.setDisplay(mqttEnabled);
  _mqttPwd.setValue(config.get(KEY_MQTT_PASSWORD));
  _mqttPwd.setDisplay(mqttEnabled);
  _mqttSecured.setValue(config.getBool(KEY_MQTT_SECURED));
  _mqttSecured.setDisplay(mqttEnabled);
  _mqttServerCert.setValue("/api/config/mqttServerCertificate");
  _mqttServerCert.setDisplay(mqttEnabled && !serverCertExists);
  _mqttServerCertDelete.setDisplay(mqttEnabled && serverCertExists);
  _mqttTopic.setValue(config.get(KEY_MQTT_TOPIC));
  _mqttTopic.setDisplay(mqttEnabled);
  _mqttPublishInterval.setValue(config.getInt(KEY_MQTT_PUBLISH_INTERVAL));
  _mqttPublishInterval.setDisplay(mqttEnabled);
  
  _mqttSep1.setDisplay(mqttEnabled);
  _mqttGridVoltage.setValue(config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC));
  _mqttGridVoltage.setDisplay(mqttEnabled);
  _mqttGridPower.setValue(config.get(KEY_GRID_POWER_MQTT_TOPIC));
  _mqttGridPower.setDisplay(mqttEnabled);
  _mqttTempO1.setValue(config.get(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC));
  _mqttTempO1.setDisplay(mqttEnabled);
  _mqttTempO2.setValue(config.get(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC));
  _mqttTempO2.setDisplay(mqttEnabled);
  
  _haSep.setDisplay(mqttEnabled);
  _haDiscovery.setValue(config.getBool(KEY_ENABLE_HA_DISCOVERY));
  _haDiscovery.setDisplay(mqttEnabled);
  _haDiscoveryTopic.setValue(config.get(KEY_HA_DISCOVERY_TOPIC));
  _haDiscoveryTopic.setDisplay(mqttEnabled && config.getBool(KEY_ENABLE_HA_DISCOVERY));

  // tab: pid

  switch (config.getInt(KEY_PID_P_MODE)) {
    case 1:
      _pidPMode.setValue(YASOLR_PID_P_MODE_1);
      break;
    default:
      _pidPMode.setValue(YASOLR_PID_P_MODE_2);
      break;
  }

  switch (config.getInt(KEY_PID_IC_MODE)) {
    case 1:
      _pidICMode.setValue(YASOLR_PID_IC_MODE_1);
      break;
    default:
      _pidICMode.setValue(YASOLR_PID_IC_MODE_2);
      break;
  }

  _pidSetpoint.setValue(config.getInt(KEY_PID_SETPOINT));
  _pidKp.setValue(config.getFloat(KEY_PID_KP));
  _pidKi.setValue(config.getFloat(KEY_PID_KI));
  _pidKd.setValue(config.getFloat(KEY_PID_KD));
  _pidOutMin.setValue(config.getInt(KEY_PID_OUT_MIN));
  _pidOutMax.setValue(config.getInt(KEY_PID_OUT_MAX));

  _pidInputHistory.setDisplay(pidViewEnabled);
  _pidOutputHistory.setDisplay(pidViewEnabled);
  _pidErrorHistory.setDisplay(pidViewEnabled);
  _pidSumHistory.setDisplay(pidViewEnabled);
  _pidPTermHistory.setDisplay(pidViewEnabled);
  _pidITermHistory.setDisplay(pidViewEnabled);
  _pidDTermHistory.setDisplay(pidViewEnabled);

  // tab: gpio

  std::unordered_map<int32_t, dash::FeedbackInputCard<int32_t>*> pinout = {};
  _pinout(_pinDimmerO1, KEY_PIN_OUTPUT1_DIMMER, pinout);
  _pinout(_pinDS18O1, KEY_PIN_OUTPUT1_DS18, pinout);
  _pinout(_pinRelayO1, KEY_PIN_OUTPUT1_RELAY, pinout);
  _pinout(_pinDimmerO2, KEY_PIN_OUTPUT2_DIMMER, pinout);
  _pinout(_pinDS18O2, KEY_PIN_OUTPUT2_DS18, pinout);
  _pinout(_pinRelayO2, KEY_PIN_OUTPUT2_RELAY, pinout);
  _pinout(_pinRelay1, KEY_PIN_RELAY1, pinout);
  _pinout(_pinRelay2, KEY_PIN_RELAY2, pinout);
  _pinout(_pinJsyRX, KEY_PIN_JSY_RX, pinout);
  _pinout(_pinJsyTX, KEY_PIN_JSY_TX, pinout);
  _pinout(_pinPZEMRX, KEY_PIN_PZEM_RX, pinout);
  _pinout(_pinPZEMTX, KEY_PIN_PZEM_TX, pinout);
  _pinout(_pinZCD, KEY_PIN_ZCD, pinout);
  _pinout(_pinI2CSCL, KEY_PIN_I2C_SCL, pinout);
  _pinout(_pinI2CSDA, KEY_PIN_I2C_SDA, pinout);
  _pinout(_pinDS18Router, KEY_PIN_ROUTER_DS18, pinout);
  _pinout(_pinLEDGreen, KEY_PIN_LIGHTS_GREEN, pinout);
  _pinout(_pinLEDRed, KEY_PIN_LIGHTS_RED, pinout);
  _pinout(_pinLEDYellow, KEY_PIN_LIGHTS_YELLOW, pinout);

  _serialJsy.setValue(config.get(KEY_JSY_UART));
  _serialPZEM.setValue(config.get(KEY_PZEM_UART));

  // tab: hardware

  // grid
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
  _status(_jsyRemote, KEY_ENABLE_JSY_REMOTE, udp && udp->connected(), YASOLR_LBL_113);
  _victronServer.setValue(config.get(KEY_VICTRON_MODBUS_SERVER));
  _victronPort.setValue(config.getInt(KEY_VICTRON_MODBUS_PORT));
  _victronServer.setDisplay(victronEnabled);
  _victronPort.setDisplay(victronEnabled);

  // output 1 dimmer
  _output1DimmerType.setValue(config.get(KEY_OUTPUT1_DIMMER_TYPE));
  _output1DimmerType.setDisplay(dimmer1Enabled);
  _output1DimmerMapper.setValue({static_cast<uint8_t>(config.getInt(KEY_OUTPUT1_DIMMER_MIN)), static_cast<uint8_t>(config.getInt(KEY_OUTPUT1_DIMMER_MAX))});
  _output1DimmerMapper.setDisplay(dimmer1Enabled);
  _output1PZEM.setDisplay(dimmer1Enabled);
  _output1PZEMSync.setDisplay(dimmer1Enabled && pzem1Enabled);

  // output 1 bypass relay
  _status(_output1Relay, KEY_ENABLE_OUTPUT1_RELAY, output1 && output1->isBypassRelayEnabled());
  _output1RelayType.setValue(config.get(KEY_OUTPUT1_RELAY_TYPE));
  _output1RelayType.setDisplay(output1RelayEnabled);

  // output 2 dimmer
  _output2DimmerType.setValue(config.get(KEY_OUTPUT2_DIMMER_TYPE));
  _output2DimmerType.setDisplay(dimmer2Enabled);
  _output2DimmerMapper.setValue({static_cast<uint8_t>(config.getInt(KEY_OUTPUT2_DIMMER_MIN)), static_cast<uint8_t>(config.getInt(KEY_OUTPUT2_DIMMER_MAX))});
  _output2DimmerMapper.setDisplay(dimmer2Enabled);
  _output2PZEM.setDisplay(dimmer2Enabled);
  _output2PZEMSync.setDisplay(dimmer2Enabled && pzem2Enabled);

  // output 2 bypass relay
  _status(_output2Relay, KEY_ENABLE_OUTPUT2_RELAY, output2 && output2->isBypassRelayEnabled());
  _output2RelayType.setValue(config.get(KEY_OUTPUT2_RELAY_TYPE));
  _output2RelayType.setDisplay(output2RelayEnabled);

  // relay1
  _status(_relay1, KEY_ENABLE_RELAY1, relay1 && relay1->isEnabled());
  _relay1Type.setValue(config.get(KEY_RELAY1_TYPE));
  _relay1Type.setDisplay(relay1Enabled);
  _relay1Load.setValue(config.getInt(KEY_RELAY1_LOAD));
  _relay1Load.setDisplay(relay1Enabled);
  _relay1Tolerance.setValue(config.getInt(KEY_RELAY1_TOLERANCE));
  _relay1Tolerance.setDisplay(relay1Enabled);

  // relay2
  _status(_relay2, KEY_ENABLE_RELAY2, relay2 && relay2->isEnabled());
  _relay2Type.setValue(config.get(KEY_RELAY2_TYPE));
  _relay2Type.setDisplay(relay2Enabled);
  _relay2Load.setValue(config.getInt(KEY_RELAY2_LOAD));
  _relay2Load.setDisplay(relay2Enabled);
  _relay2Tolerance.setValue(config.getInt(KEY_RELAY2_TOLERANCE));
  _relay2Tolerance.setDisplay(relay2Enabled);

  // router led
  _status(_led, KEY_ENABLE_LIGHTS, lights.isEnabled());

  // display
  _status(_display, KEY_ENABLE_DISPLAY, display && display->isEnabled());
  _displayType.setValue(config.get(KEY_DISPLAY_TYPE));
  _displayType.setDisplay(displayEnabled);
  _displayRotation.setValue(config.getInt(KEY_DISPLAY_ROTATION));
  _displayRotation.setDisplay(displayEnabled);
  _displaySpeed.setValue(config.getInt(KEY_DISPLAY_SPEED));
  _displaySpeed.setDisplay(displayEnabled);

  // tab: output 1 config

  _output1ConfigSep0.setDisplay(dimmer1Enabled);
  _output1ResistanceInput.setValue(config.getFloat(KEY_OUTPUT1_RESISTANCE));
  _output1ResistanceInput.setStatus(_output1ResistanceInput.value() > 0 ? dash::Status::SUCCESS : dash::Status::DANGER); // needs to be after set value
  _output1ResistanceInput.setDisplay(dimmer1Enabled);
  _output1ResistanceCalibration.setDisplay((dimmer1Enabled && jsyEnabled) || (dimmer1Enabled && pzem1Enabled));
  _output1ConfigSep1.setDisplay(dimmer1Enabled);
  _output1DimmerDutyLimiter.setValue(config.getInt(KEY_OUTPUT1_DIMMER_LIMIT));
  _output1DimmerDutyLimiter.setDisplay(dimmer1Enabled);
  _output1DimmerTempLimiter.setValue(config.getInt(KEY_OUTPUT1_DIMMER_TEMP_LIMITER));
  _output1DimmerTempLimiter.setDisplay(dimmer1Enabled);
  _output1DimmerExcessLimiter.setValue(config.getInt(KEY_OUTPUT1_EXCESS_LIMITER));
  _output1DimmerExcessLimiter.setDisplay(dimmer1Enabled);
  _output1ConfigSep2.setDisplay(bypass1Possible);
  _output1AutoStartTemp.setValue(config.getInt(KEY_OUTPUT1_TEMPERATURE_START));
  _output1AutoStartTemp.setDisplay(bypass1Possible);
  _output1AutoStoptTemp.setValue(config.getInt(KEY_OUTPUT1_TEMPERATURE_STOP));
  _output1AutoStoptTemp.setDisplay(bypass1Possible);
  _output1AutoStartTime.setValue(config.get(KEY_OUTPUT1_TIME_START));
  _output1AutoStartTime.setDisplay(bypass1Possible);
  _output1AutoStoptTime.setValue(config.get(KEY_OUTPUT1_TIME_STOP));
  _output1AutoStoptTime.setDisplay(bypass1Possible);
  _output1AutoStartWDays.setValue(config.isEqual(KEY_OUTPUT1_DAYS, YASOLR_WEEK_DAYS_EMPTY) ? "" : config.get(KEY_OUTPUT1_DAYS));
  _output1AutoStartWDays.setDisplay(bypass1Possible);
  _output1BypassTimeout.setValue(config.getFloat(KEY_OUTPUT1_BYPASS_TIMEOUT) / 3600.0f);
  _output1BypassTimeout.setDisplay(bypass1Possible);

  // tab: output 2 config

  _output2ConfigSep0.setDisplay(dimmer2Enabled);
  _output2ResistanceInput.setValue(config.getFloat(KEY_OUTPUT2_RESISTANCE));
  _output2ResistanceInput.setStatus(_output2ResistanceInput.value() > 0 ? dash::Status::SUCCESS : dash::Status::DANGER); // needs to be after set value
  _output2ResistanceInput.setDisplay(dimmer2Enabled);
  _output2ResistanceCalibration.setDisplay((dimmer2Enabled && jsyEnabled) || (dimmer2Enabled && pzem2Enabled));
  _output2ConfigSep1.setDisplay(dimmer2Enabled);
  _output2DimmerDutyLimiter.setValue(config.getInt(KEY_OUTPUT2_DIMMER_LIMIT));
  _output2DimmerDutyLimiter.setDisplay(dimmer2Enabled);
  _output2DimmerTempLimiter.setValue(config.getInt(KEY_OUTPUT2_DIMMER_TEMP_LIMITER));
  _output2DimmerTempLimiter.setDisplay(dimmer2Enabled);
  _output2DimmerExcessLimiter.setValue(config.getInt(KEY_OUTPUT2_EXCESS_LIMITER));
  _output2DimmerExcessLimiter.setDisplay(dimmer2Enabled);
  _output2ConfigSep2.setDisplay(bypass2Possible);
  _output2AutoStartTemp.setValue(config.getInt(KEY_OUTPUT2_TEMPERATURE_START));
  _output2AutoStartTemp.setDisplay(bypass2Possible);
  _output2AutoStoptTemp.setValue(config.getInt(KEY_OUTPUT2_TEMPERATURE_STOP));
  _output2AutoStoptTemp.setDisplay(bypass2Possible);
  _output2AutoStartTime.setValue(config.get(KEY_OUTPUT2_TIME_START));
  _output2AutoStartTime.setDisplay(bypass2Possible);
  _output2AutoStoptTime.setValue(config.get(KEY_OUTPUT2_TIME_STOP));
  _output2AutoStoptTime.setDisplay(bypass2Possible);
  _output2AutoStartWDays.setValue(config.isEqual(KEY_OUTPUT2_DAYS, YASOLR_WEEK_DAYS_EMPTY) ? "" : config.get(KEY_OUTPUT2_DAYS));
  _output2AutoStartWDays.setDisplay(bypass2Possible);
  _output2BypassTimeout.setValue(config.getFloat(KEY_OUTPUT2_BYPASS_TIMEOUT) / 3600.0f);
  _output2BypassTimeout.setDisplay(bypass2Possible);
#endif
}

void YaSolR::Website::updateCards() {
  // metrics

  Mycila::Grid::Metrics gridMetrics;
  grid.getGridMeasurements(gridMetrics);
  _gridEnergy.setValue(gridMetrics.energy);
  _gridEnergyReturned.setValue(gridMetrics.energyReturned);
  _routerVoltage.setValue(gridMetrics.voltage);
  _gridPower.setValue(gridMetrics.power);

  Mycila::Router::Metrics routerMetrics;
  router.getRouterMeasurements(routerMetrics);
  _routerPower.setValue(routerMetrics.power);
  _routerApparentPower.setValue(routerMetrics.apparentPower);
  _routerPowerFactor.setValue(routerMetrics.powerFactor);
  _routerTHDi.setValue(routerMetrics.thdi);
  _routerCurrent.setValue(routerMetrics.current);
  _routerResistance.setValue(routerMetrics.resistance);
  _routerEnergy.setValue(routerMetrics.energy);

  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);
  _deviceHeapTotal.setValue(memory.total);
  _deviceHeapUsed.setValue(memory.used);
  _deviceHeapUsage.setValue(memory.usage);
  _deviceHeapMinFree.setValue(memory.minimumFree);

  Mycila::RouterOutput::Metrics output1Measurements;
  if (output1) {
    output1->getOutputMeasurements(output1Measurements);
  }

  Mycila::RouterOutput::Metrics output2Measurements;
  if (output2) {
    output2->getOutputMeasurements(output2Measurements);
  }

  // statistics

  _gridFrequency.setValue(yasolr_frequency());
  _udpMessageRateBuffer.setValue(udpMessageRateBuffer ? udpMessageRateBuffer->rate() : 0);
  _networkWiFiRSSI.setValue(espConnect.getWiFiRSSI());
  _networkWiFiSignal.setValue(espConnect.getWiFiSignalQuality());
  _output1RelaySwitchCount.setValue(output1 ? output1->getBypassRelaySwitchCount() : 0);
  _output2RelaySwitchCount.setValue(output2 ? output2->getBypassRelaySwitchCount() : 0);
  _relay1SwitchCount.setValue(relay1 ? relay1->getSwitchCount() : 0);
  _relay2SwitchCount.setValue(relay2 ? relay2->getSwitchCount() : 0);
  _time.setValue(Mycila::Time::getLocalStr());
  _uptime.setValue(Mycila::Time::toDHHMMSS(Mycila::System::getUptime()));
#ifdef APP_MODEL_TRIAL
  _trialRemainingTime.setValue(Mycila::Time::toDHHMMSS(Mycila::Trial.getRemaining()));
#endif

  // home

  if (output1) {
    switch (output1->getState()) {
      case Mycila::RouterOutput::State::OUTPUT_DISABLED:
      case Mycila::RouterOutput::State::OUTPUT_IDLE:
        _output1State.setFeedback(output1->getStateName(), dash::Status::INFO);
        break;
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_AUTO:
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_MANUAL:
        _output1State.setFeedback(output1->getStateName(), dash::Status::WARNING);
        break;
      case Mycila::RouterOutput::State::OUTPUT_ROUTING:
        _output1State.setFeedback(output1->getStateName() + std::string(" ") + std::to_string(static_cast<uint16_t>(output1->getDimmerDutyCycleLive() * 100.0f)) + " %", dash::Status::SUCCESS);
        break;
      default:
        _output1State.setFeedback(YASOLR_LBL_109, dash::Status::DANGER);
        break;
    }
    _output1DS18State.setValue(output1->temperature().orElse(NAN));
    _output1DimmerSlider.setValue(output1->getDimmerDutyCycle() * 100.0f);
    _output1Bypass.setValue(output1->isBypassOn());
  }

  if (output2) {
    switch (output2->getState()) {
      case Mycila::RouterOutput::State::OUTPUT_DISABLED:
      case Mycila::RouterOutput::State::OUTPUT_IDLE:
        _output2State.setFeedback(output2->getStateName(), dash::Status::INFO);
        break;
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_AUTO:
      case Mycila::RouterOutput::State::OUTPUT_BYPASS_MANUAL:
        _output2State.setFeedback(output2->getStateName(), dash::Status::WARNING);
        break;
      case Mycila::RouterOutput::State::OUTPUT_ROUTING:
        _output2State.setFeedback(output2->getStateName() + std::string(" ") + std::to_string(static_cast<uint16_t>(output2->getDimmerDutyCycleLive() * 100.0f)) + " %", dash::Status::SUCCESS);
        break;
      default:
        _output2State.setFeedback(YASOLR_LBL_109, dash::Status::DANGER);
        break;
    }
    _output2DS18State.setValue(output2->temperature().orElse(NAN));
    _output2DimmerSlider.setValue(output2->getDimmerDutyCycle() * 100.0f);
    _output2Bypass.setValue(output2->isBypassOn());
  }

  _routerDS18State.setValue(ds18Sys ? ds18Sys->getTemperature().value_or(0.0f) : 0);

  if (relay1) {
    _relay1Switch.setValue(relay1->isOn());
#ifdef APP_MODEL_PRO
    uint16_t load = relay1->getLoad(gridMetrics.voltage);
    _relay1Switch.setMessage(relay1->isOn() && load ? std::to_string(load) + " W" : "");
#endif
  }

  if (relay2) {
    _relay2Switch.setValue(relay2->isOn());
#ifdef APP_MODEL_PRO
    uint16_t load = relay2->getLoad(gridMetrics.voltage);
    _relay2Switch.setMessage(relay2->isOn() && load ? std::to_string(load) + " W" : "");
#endif
  }

#ifdef APP_MODEL_PRO
  if (pzemO1PairingTask && pzemO1PairingTask->scheduled()) {
    _output1PZEMSync.setIndicator(true, dash::Status::WARNING);
  } else if (pzemO1 && pzemO1->isConnected()) {
    _output1PZEMSync.setIndicator(true, dash::Status::SUCCESS);
  } else {
    _output1PZEMSync.setIndicator(true, dash::Status::DANGER);
  }

  if (pzemO2PairingTask && pzemO2PairingTask->scheduled()) {
    _output2PZEMSync.setIndicator(true, dash::Status::WARNING);
  } else if (pzemO2 && pzemO2->isConnected()) {
    _output2PZEMSync.setIndicator(true, dash::Status::SUCCESS);
  } else {
    _output2PZEMSync.setIndicator(true, dash::Status::DANGER);
  }

  if (router.isCalibrationRunning()) {
    _output1ResistanceCalibration.setIndicator(true, dash::Status::WARNING);
  } else if (output1 && output1->config.calibratedResistance > 0) {
    _output1ResistanceCalibration.setIndicator(true, dash::Status::SUCCESS);
  } else {
    _output1ResistanceCalibration.setIndicator(true, dash::Status::DANGER);
  }

  if (router.isCalibrationRunning()) {
    _output2ResistanceCalibration.setIndicator(true, dash::Status::WARNING);
  } else if (output2 && output2->config.calibratedResistance > 0) {
    _output2ResistanceCalibration.setIndicator(true, dash::Status::SUCCESS);
  } else {
    _output2ResistanceCalibration.setIndicator(true, dash::Status::DANGER);
  }
#else
  _output1PZEMSync.setValue(pzemO1PairingTask && pzemO1PairingTask->scheduled());
  _output2PZEMSync.setValue(pzemO2PairingTask && pzemO2PairingTask->scheduled());
  _output1ResistanceCalibration.setValue(router.isCalibrationRunning());
  _output2ResistanceCalibration.setValue(router.isCalibrationRunning());
#endif

#ifdef APP_MODEL_PRO
  // tab: output 1

  if (output1) {
    _output1DimmerSliderRO.setValue(output1->getDimmerDutyCycleLive() * 100.0f);
    _output1Power.setValue(output1Measurements.power);
    _output1ApparentPower.setValue(output1Measurements.apparentPower);
    _output1PowerFactor.setValue(output1Measurements.powerFactor);
    _output1THDi.setValue(output1Measurements.thdi);
    _output1Voltage.setValue(output1Measurements.dimmedVoltage);
    _output1Current.setValue(output1Measurements.current);
    _output1Resistance.setValue(output1Measurements.resistance);
    _output1Energy.setValue(output1Measurements.energy);

    const uint32_t bypassUptime = output1->getBypassUptime();
    _output1Bypass.setMessage(bypassUptime && output1->isBypassOn() ? Mycila::Time::toDHHMMSS(bypassUptime) : "");
  }

  // tab: output 2

  if (output2) {
    _output2DimmerSliderRO.setValue(output2->getDimmerDutyCycleLive() * 100.0f);
    _output2Power.setValue(output2Measurements.power);
    _output2ApparentPower.setValue(output2Measurements.apparentPower);
    _output2PowerFactor.setValue(output2Measurements.powerFactor);
    _output2THDi.setValue(output2Measurements.thdi);
    _output2Voltage.setValue(output2Measurements.dimmedVoltage);
    _output2Current.setValue(output2Measurements.current);
    _output2Resistance.setValue(output2Measurements.resistance);
    _output2Energy.setValue(output2Measurements.energy);

    const uint32_t bypassUptime = output2->getBypassUptime();
    _output2Bypass.setMessage(bypassUptime && output2->isBypassOn() ? Mycila::Time::toDHHMMSS(bypassUptime) : "");
  }

  // tab: mqtt

  _status(_mqtt, KEY_ENABLE_MQTT, mqtt && mqtt->isEnabled(), mqtt && mqtt->isConnected(), mqtt && mqtt->getLastError() ? mqtt->getLastError() : YASOLR_LBL_113);

  // tab: hardware

  _status(_jsy, KEY_ENABLE_JSY, jsy && jsy->isEnabled(), jsy && jsy->isConnected(), YASOLR_LBL_110);
  _status(_zcd, KEY_ENABLE_ZCD, pulseAnalyzer && pulseAnalyzer->isEnabled(), pulseAnalyzer && pulseAnalyzer->isOnline(), YASOLR_LBL_110);
  _status(_output1Dimmer, KEY_ENABLE_OUTPUT1_DIMMER, output1 && output1->isDimmerEnabled(), output1 && output1->isDimmerOnline(), YASOLR_LBL_146);
  _status(_output1PZEM, KEY_ENABLE_OUTPUT1_PZEM, pzemO1 && pzemO1->isEnabled(), pzemO1 && pzemO1->isConnected() && pzemO1->getDeviceAddress() == YASOLR_PZEM_ADDRESS_OUTPUT1, pzemO1 && pzemO1->isConnected() ? YASOLR_LBL_180 : YASOLR_LBL_110);
  _status(_output1DS18, KEY_ENABLE_OUTPUT1_DS18, ds18O1 && ds18O1->isEnabled(), ds18O1 && ds18O1->getLastTime() > 0, YASOLR_LBL_114);
  _status(_output2Dimmer, KEY_ENABLE_OUTPUT2_DIMMER, output2 && output2->isDimmerEnabled(), output2 && output2->isDimmerOnline(), YASOLR_LBL_146);
  _status(_output2PZEM, KEY_ENABLE_OUTPUT2_PZEM, pzemO2 && pzemO2->isEnabled(), pzemO2 && pzemO2->isConnected() && pzemO2->getDeviceAddress() == YASOLR_PZEM_ADDRESS_OUTPUT2, pzemO2 && pzemO2->isConnected() ? YASOLR_LBL_180 : YASOLR_LBL_110);
  _status(_output2DS18, KEY_ENABLE_OUTPUT2_DS18, ds18O2 && ds18O2->isEnabled(), ds18O2 && ds18O2->getLastTime() > 0, YASOLR_LBL_114);
  _status(_routerDS18, KEY_ENABLE_DS18_SYSTEM, ds18Sys && ds18Sys->isEnabled(), ds18Sys && ds18Sys->getLastTime() > 0, YASOLR_LBL_114);
  _status(_victron, KEY_ENABLE_VICTRON_MODBUS, victron, victron && !victron->hasError(), victron && victron->hasError() ? "Com. Error" : "");
#endif
}

void YaSolR::Website::updateCharts() {
  // shift array
  memmove(&_gridPowerHistoryY[0], &_gridPowerHistoryY[1], sizeof(_gridPowerHistoryY) - sizeof(*_gridPowerHistoryY));
  memmove(&_routedPowerHistoryY[0], &_routedPowerHistoryY[1], sizeof(_routedPowerHistoryY) - sizeof(*_routedPowerHistoryY));
  memmove(&_routerTHDiHistoryY[0], &_routerTHDiHistoryY[1], sizeof(_routerTHDiHistoryY) - sizeof(*_routerTHDiHistoryY));

  // set new value
  _gridPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(grid.getPower().value_or(0));

  Mycila::Router::Metrics* routerMetrics = new Mycila::Router::Metrics();
  router.getRouterMeasurements(*routerMetrics);
  _routedPowerHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(routerMetrics->power);
  _routerTHDiHistoryY[YASOLR_GRAPH_POINTS - 1] = std::isnan(routerMetrics->thdi) ? 0 : std::round(routerMetrics->thdi);
  delete routerMetrics;
  routerMetrics = nullptr;

  // update charts
  _gridPowerHistory.setY(_gridPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routedPowerHistory.setY(_routedPowerHistoryY, YASOLR_GRAPH_POINTS);
  _routerTHDiHistory.setY(_routerTHDiHistoryY, YASOLR_GRAPH_POINTS);
}

void YaSolR::Website::updatePIDCharts() {
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
  _pidInputHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getInput());
  _pidOutputHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getOutput());
  _pidErrorHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getError());
  _pidSumHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getSum());
  _pidPTermHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getPTerm());
  _pidITermHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getITerm());
  _pidDTermHistoryY[YASOLR_GRAPH_POINTS - 1] = std::round(pidController.getDTerm());

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

void YaSolR::Website::resetPIDCharts() {
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

bool YaSolR::Website::realTimePIDEnabled() const {
#ifdef APP_MODEL_PRO
  return _pidView.optional().value_or(false);
#else
  return false;
#endif
}

void YaSolR::Website::setSafeBootUpdateStatus(const char* msg, dash::Status status) {
#ifdef APP_MODEL_PRO
  if (status == dash::Status::DANGER) {
    LOGE(TAG, "SafeBoot Update: %s", msg);
  } else if (status == dash::Status::WARNING) {
    LOGW(TAG, "SafeBoot Update: %s", msg);
  } else {
    LOGI(TAG, "SafeBoot Update: %s", msg);
  }
  _safebootUploadStatus.setFeedback(msg, status);
  _safebootUploadStatus.setDisplay(true);
  _safebootUpload.setDisplay(false);
  dashboard.refresh(_safebootUpload);
  dashboard.refresh(_safebootUploadStatus);
#endif
}
