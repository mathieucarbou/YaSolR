// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolRWebsite.h>

#define TAG "WEBSITE"

void YaSolR::WebsiteClass::_update(bool skipWebSocketPush) {
  if (dashboard.isAsyncAccessInProgress()) {
    return;
  }

  // stats

  Mycila::SystemMemory memory = Mycila::System.getMemory();
  ESPConnectMode mode = ESPConnect.getMode();
  _apIPStat.set(ESPConnect.getIPAddress(ESPConnectMode::AP).toString().c_str());
  _apMACStat.set(ESPConnect.getMACAddress(ESPConnectMode::AP).c_str());
  _ethIPStat.set(ESPConnect.getIPAddress(ESPConnectMode::ETH).toString().c_str());
  _ethMACStat.set(ESPConnect.getMACAddress(ESPConnectMode::ETH).c_str());
  _gridFreqStat.set((String(Mycila::Grid.getFrequency()) + " Hz").c_str());
  _gridVoltStat.set((String(Mycila::Grid.getVoltage()) + " V").c_str());
  _heapMemoryUsageStat.set((String(memory.usage) + " %").c_str());
  _heapMemoryUsedStat.set((String(memory.used) + " bytes").c_str());
  _netModeStat.set(mode == ESPConnectMode::AP ? "AP" : (mode == ESPConnectMode::STA ? "WiFi" : (mode == ESPConnectMode::ETH ? "Ethernet" : "")));
  _output1SwitchCountStat.set(String(output1BypassRelay.getSwitchCount()).c_str());
  _output2SwitchCountStat.set(String(output2BypassRelay.getSwitchCount()).c_str());
  _relay1SwitchCountStat.set(String(relay1.getSwitchCount()).c_str());
  _relay2SwitchCountStat.set(String(relay2.getSwitchCount()).c_str());
  _timeStat.set(Mycila::Time::getLocalStr().c_str());
  _uptimeStat.set((String(Mycila::System.getUptime()) + " s").c_str());
  _wifiIPStat.set(ESPConnect.getIPAddress(ESPConnectMode::STA).toString().c_str());
  _wifiMACStat.set(ESPConnect.getMACAddress(ESPConnectMode::STA).c_str());
  _wifiRSSIStat.set((String(ESPConnect.getWiFiRSSI()) + " dBm").c_str());
  _wifiSignalStat.set((String(ESPConnect.getWiFiSignalQuality()) + " %").c_str());
  _wifiSSIDStat.set(ESPConnect.getWiFiSSID().c_str());
#ifdef APP_VERSION_TRIAL
  _trialRemainingStat.set((String(Mycila::Trial.getRemaining()) + " s").c_str());
#endif

  // home

  _relay1Switch.update(relay1.isOn());
  _relay2Switch.update(relay2.isOn());
  _gridPower.update(Mycila::Grid.getActivePower());
  _temperature(&_systemTempState, &systemTemperatureSensor);

  _output1BypassSwitch.update(output1.isBypassRelayOn());
  _output1DimmerSlider.update(static_cast<int>(output1Dimmer.getLevel()));
  switch (output1.getState()) {
    case Mycila::RouterOutputState::OUTPUT_DISABLED:
    case Mycila::RouterOutputState::OUTPUT_IDLE:
      _output1State.update(output1.getStateString(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutputState::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutputState::OUTPUT_BYPASS_MANUAL:
      _output1State.update(output1.getStateString(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutputState::OUTPUT_ROUTING:
      _output1State.update(output1.getStateString(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output1State.update("Unknown", DASH_STATUS_DANGER);
      break;
  }
  _temperature(&_output1TempState, &output1TemperatureSensor);

  _output2BypassSwitch.update(output2.isBypassRelayOn());
  _output2DimmerSlider.update(static_cast<int>(output2Dimmer.getLevel()));
  switch (output2.getState()) {
    case Mycila::RouterOutputState::OUTPUT_DISABLED:
    case Mycila::RouterOutputState::OUTPUT_IDLE:
      _output2State.update(output2.getStateString(), DASH_STATUS_IDLE);
      break;
    case Mycila::RouterOutputState::OUTPUT_BYPASS_AUTO:
    case Mycila::RouterOutputState::OUTPUT_BYPASS_MANUAL:
      _output2State.update(output2.getStateString(), DASH_STATUS_WARNING);
      break;
    case Mycila::RouterOutputState::OUTPUT_ROUTING:
      _output2State.update(output2.getStateString(), DASH_STATUS_SUCCESS);
      break;
    default:
      _output2State.update("Unknown", DASH_STATUS_DANGER);
      break;
  }
  _temperature(&_output2TempState, &output2TemperatureSensor);

  _routerPowerFactor.update(Mycila::Router.getTotalPowerFactor());
  _routerTHDi.update(Mycila::Router.getTotalTHDi() * 100);
  _routerPower.update(Mycila::Router.getTotalRoutedPower());
  _routerEnergy.update(Mycila::Router.getTotalRoutedEnergy());

  _output1Power.update(output1.getActivePower());
  _output1Voltage.update(output1.getOutputVoltage());
  _output1Current.update(output1.getCurrent());
  _output1Resistance.update(output1.getResistance());

  _output1ApparentPower.update(output1.getApparentPower());
  _output1PowerFactor.update(output1.getPowerFactor());
  _output1THDi.update(output1.getTHDi() * 100);
  _output1Energy.update(output1.getEnergy());

  _output2Power.update(output2.getActivePower());
  _output2Voltage.update(output2.getOutputVoltage());
  _output2Current.update(output2.getCurrent());
  _output2Resistance.update(output2.getResistance());

  _output2ApparentPower.update(output2.getApparentPower());
  _output2PowerFactor.update(output2.getPowerFactor());
  _output2THDi.update(output2.getTHDi() * 100);
  _output2Energy.update(output2.getEnergy());

#ifdef APP_VERSION_PRO
  // management tab

  _debugMode.update(Mycila::Config.getBool(KEY_DEBUG_ENABLE));

  // health

  const bool gridOnline = Mycila::Grid.isConnected();
  _status(&_stateBuzzer, KEY_BUZZER_ENABLE, Mycila::Buzzer.isEnabled());
  _status(&_stateDisplay, KEY_DISPLAY_ENABLE, Mycila::EasyDisplay.isEnabled());
  _status(&_stateJSY, KEY_JSY_ENABLE, jsy.isEnabled(), jsy.isConnected(), "No grid voltage");
  _status(&_stateLEDs, KEY_LIGHTS_ENABLE, Mycila::Lights.isEnabled());
  _status(&_stateMQTT, KEY_MQTT_ENABLE, Mycila::MQTT.isEnabled(), Mycila::MQTT.isConnected(), Mycila::MQTT.getLastError() ? Mycila::MQTT.getLastError() : "Disconnected");
  _status(&_stateOutput1AutoBypass, KEY_OUTPUT1_AUTO_BYPASS_ENABLE, true, Mycila::NTP.isSynced(), "Waiting for time sync");
  _status(&_stateOutput1AutoDimmer, KEY_OUTPUT1_DIMMER_AUTO, true, output1Dimmer.isEnabled(), "Dimmer disabled");
  _status(&_stateOutput1Bypass, KEY_OUTPUT1_RELAY_ENABLE, output1.isBypassRelayEnabled());
  _status(&_stateOutput1Dimmer, KEY_OUTPUT1_DIMMER_ENABLE, output1Dimmer.isEnabled(), gridOnline, "No grid voltage");
  _status(&_stateOutput1PZEM, KEY_OUTPUT1_PZEM_ENABLE, output1PZEM.isEnabled(), output1PZEM.isConnected(), "Not routing");
  _status(&_stateOutput1Temp, KEY_OUTPUT1_TEMP_ENABLE, output1TemperatureSensor.isEnabled(), output1TemperatureSensor.isValid(), "Read error");
  _status(&_stateOutput2AutoBypass, KEY_OUTPUT2_AUTO_BYPASS_ENABLE, true, Mycila::NTP.isSynced(), "Waiting for time sync");
  _status(&_stateOutput2AutoDimmer, KEY_OUTPUT2_DIMMER_AUTO, true, output1Dimmer.isEnabled(), "Dimmer disabled");
  _status(&_stateOutput2Bypass, KEY_OUTPUT2_RELAY_ENABLE, output2.isBypassRelayEnabled());
  _status(&_stateOutput2Dimmer, KEY_OUTPUT2_DIMMER_ENABLE, output2Dimmer.isEnabled(), gridOnline, "No grid voltage");
  _status(&_stateOutput2PZEM, KEY_OUTPUT2_PZEM_ENABLE, output2PZEM.isEnabled(), output2PZEM.isConnected(), "Not routing");
  _status(&_stateOutput2Temp, KEY_OUTPUT2_TEMP_ENABLE, output2TemperatureSensor.isEnabled(), output2TemperatureSensor.isValid(), "Read error");
  _status(&_statePushButton, KEY_BUTTON_ENABLE, Mycila::Button.isEnabled());
  _status(&_stateRelay1, KEY_RELAY1_ENABLE, relay1.isEnabled());
  _status(&_stateRelay2, KEY_RELAY2_ENABLE, relay2.isEnabled());
  _status(&_stateSystemTemp, KEY_SYSTEM_TEMP_ENABLE, systemTemperatureSensor.isEnabled(), systemTemperatureSensor.isValid(), "Read error");
  _status(&_stateZCD, KEY_ZCD_ENABLE, Mycila::ZCD.isEnabled(), Mycila::ZCD.isConnected(), "No grid voltage");
  // ntp
  if (Mycila::NTP.isSynced() && !Mycila::NTP.getTimezoneInfo().isEmpty())
    _stateNTP.update("Enabled", DASH_STATUS_SUCCESS);
  else if (Mycila::NTP.getTimezoneInfo().isEmpty())
    _stateNTP.update("Invalid timezone", DASH_STATUS_DANGER);
  else if (!Mycila::NTP.isSynced())
    _stateNTP.update("Waiting for time sync", DASH_STATUS_WARNING);
  // _stateGridDataMQTT
  if (Mycila::Config.get(KEY_GRID_POWER_MQTT_TOPIC).isEmpty())
    _stateGridDataMQTT.update("Disabled", DASH_STATUS_IDLE);
  else if (Mycila::Grid.isMQTTGridDataExpired())
    _stateGridDataMQTT.update("Expired", DASH_STATUS_WARNING);
  else
    _stateGridDataMQTT.update("Enabled", DASH_STATUS_SUCCESS);

  // pinout live

  std::map<int32_t, Card*> pinout = {};
  _pinout(&_pinLiveBuzzer, Mycila::Buzzer.getPin(), &pinout);
  _pinout(&_pinLiveDisplayClock, Mycila::EasyDisplay.getClockPin(), &pinout);
  _pinout(&_pinLiveDisplayData, Mycila::EasyDisplay.getDataPin(), &pinout);
  _pinout(&_pinLiveJsyRX, jsy.getRXPin(), &pinout);
  _pinout(&_pinLiveJsyTX, jsy.getTXPin(), &pinout);
  _pinout(&_pinLiveLEDGreen, Mycila::Lights.getGreenPin(), &pinout);
  _pinout(&_pinLiveLEDRed, Mycila::Lights.getRedPin(), &pinout);
  _pinout(&_pinLiveLEDYellow, Mycila::Lights.getYellowPin(), &pinout);
  _pinout(&_pinLiveOutput1Bypass, output1BypassRelay.getPin(), &pinout);
  _pinout(&_pinLiveOutput1Dimmer, output1Dimmer.getPin(), &pinout);
  _pinout(&_pinLiveOutput1Temp, output1TemperatureSensor.getPin(), &pinout);
  _pinout(&_pinLiveOutput2Bypass, output2BypassRelay.getPin(), &pinout);
  _pinout(&_pinLiveOutput2Dimmer, output2Dimmer.getPin(), &pinout);
  _pinout(&_pinLiveOutput2Temp, output2TemperatureSensor.getPin(), &pinout);
  _pinout(&_pinLivePushButton, Mycila::Button.getPin(), &pinout);
  _pinout(&_pinLivePZEMRX, output1PZEM.getRXPin(), &pinout);
  _pinout(&_pinLivePZEMTX, output1PZEM.getTXPin(), &pinout);
  _pinout(&_pinLiveRelay1, relay1.getPin(), &pinout);
  _pinout(&_pinLiveRelay2, relay2.getPin(), &pinout);
  _pinout(&_pinLiveSystemTemp, systemTemperatureSensor.getPin(), &pinout);
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
  _pinout(&_pinConfigPZEMRX, Mycila::Config.get(KEY_PZEM_RX_PIN).toInt(), &pinout);
  _pinout(&_pinConfigPZEMTX, Mycila::Config.get(KEY_PZEM_TX_PIN).toInt(), &pinout);
  _pinout(&_pinConfigBuzzer, Mycila::Config.get(KEY_BUZZER_PIN).toInt(), &pinout);
  _pinout(&_pinConfigLEDGreen, Mycila::Config.get(KEY_LIGHTS_GREEN_PIN).toInt(), &pinout);
  _pinout(&_pinConfigLEDYellow, Mycila::Config.get(KEY_LIGHTS_YELLOW_PIN).toInt(), &pinout);
  _pinout(&_pinConfigLEDRed, Mycila::Config.get(KEY_LIGHTS_RED_PIN).toInt(), &pinout);
  pinout.clear();

  // display

  _display.update(Mycila::Config.getBool(KEY_DISPLAY_ENABLE));
  _displayClockPin.update(Mycila::Config.get(KEY_DISPLAY_CLOCK_PIN));
  _displayDataPin.update(Mycila::Config.get(KEY_DISPLAY_DATA_PIN));
  _displayType.update(Mycila::Config.get(KEY_DISPLAY_TYPE));
  _displayRotation.update(Mycila::Config.get(KEY_DISPLAY_ROTATION) + "°");
  _displayPowerSaveDelay.update(Mycila::Config.get(KEY_DISPLAY_POWER_SAVE_DELAY));

  // electricity

  _zcd.update(Mycila::Config.getBool(KEY_ZCD_ENABLE));
  _zcdPin.update(Mycila::Config.get(KEY_ZCD_PIN));
  _jsy.update(Mycila::Config.getBool(KEY_JSY_ENABLE));
  _jsyRXPin.update(Mycila::Config.get(KEY_JSY_RX_PIN));
  _jsyTXPin.update(Mycila::Config.get(KEY_JSY_TX_PIN));
  _gridFreq.update(Mycila::Config.get(KEY_GRID_FREQ).toInt() == 60 ? "60 Hz" : "50 Hz");
  _gridPowerMQTT.update(Mycila::Config.get(KEY_GRID_POWER_MQTT_TOPIC));
  _gridVoltageMQTT.update(Mycila::Config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC));

  // mqtt

  _mqtt.update(Mycila::Config.getBool(KEY_MQTT_ENABLE));
  _mqttServer.update(Mycila::Config.get(KEY_MQTT_SERVER));
  _mqttPort.update(Mycila::Config.get(KEY_MQTT_PORT));
  _mqttUser.update(Mycila::Config.get(KEY_MQTT_USERNAME));
  String pwd = Mycila::Config.get(KEY_MQTT_PASSWORD);
  _mqttPwd.update(pwd.isEmpty() ? "" : HIDDEN_PWD);
  _mqttTopic.update(Mycila::Config.get(KEY_MQTT_TOPIC));
  _mqttPublishInterval.update(Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL));
  _haDiscovery.update(Mycila::Config.getBool(KEY_HA_DISCOVERY_ENABLE));
  _haDiscoveryTopic.update(Mycila::Config.get(KEY_HA_DISCOVERY_TOPIC));
  _mqttSecured.update(Mycila::Config.getBool(KEY_MQTT_SECURED));

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

  _output1Dimmer.update(Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_ENABLE));
  _output1DimmerPin.update(Mycila::Config.get(KEY_OUTPUT1_DIMMER_PIN));
  _output1DimmerType.update(Mycila::Config.get(KEY_OUTPUT1_DIMMER_TYPE));
  _output1AutoDimmer.update(Mycila::Config.getBool(KEY_OUTPUT1_DIMMER_AUTO));

  _output1Bypass.update(Mycila::Config.getBool(KEY_OUTPUT1_RELAY_ENABLE));
  _output1BypassPin.update(Mycila::Config.get(KEY_OUTPUT1_RELAY_PIN));
  _output1BypassType.update(Mycila::Config.get(KEY_OUTPUT1_RELAY_TYPE));
  _output1AutoBypass.update(Mycila::Config.getBool(KEY_OUTPUT1_AUTO_BYPASS_ENABLE));

  _output1Temp.update(Mycila::Config.getBool(KEY_OUTPUT1_TEMP_ENABLE));
  _output1TempPin.update(Mycila::Config.get(KEY_OUTPUT1_TEMP_PIN));
  _output1DimmerPowerLimit.update(static_cast<int>(Mycila::Config.get(KEY_OUTPUT1_DIMMER_LEVEL_LIMIT).toInt()));
  _output1AutoStartWDays.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_WEEK_DAYS));

  _output1AutoStartTemp.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_START_TEMPERATURE));
  _output1AutoStartTime.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_START_TIME));
  _output1AutoStoptTemp.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_STOP_TEMPERATURE));
  _output1AutoStoptTime.update(Mycila::Config.get(KEY_OUTPUT1_AUTO_STOP_TIME));

  _output1PZEM.update(Mycila::Config.getBool(KEY_OUTPUT1_PZEM_ENABLE));
  _output1PZEMRXPin.update(Mycila::Config.get(KEY_PZEM_RX_PIN));
  _output1PZEMTXPin.update(Mycila::Config.get(KEY_PZEM_TX_PIN));

  // output2

  _output2Dimmer.update(Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_ENABLE));
  _output2DimmerPin.update(Mycila::Config.get(KEY_OUTPUT2_DIMMER_PIN));
  _output2DimmerType.update(Mycila::Config.get(KEY_OUTPUT2_DIMMER_TYPE));
  _output2AutoDimmer.update(Mycila::Config.getBool(KEY_OUTPUT2_DIMMER_AUTO));

  _output2Bypass.update(Mycila::Config.getBool(KEY_OUTPUT2_RELAY_ENABLE));
  _output2BypassPin.update(Mycila::Config.get(KEY_OUTPUT2_RELAY_PIN));
  _output2BypassType.update(Mycila::Config.get(KEY_OUTPUT2_RELAY_TYPE));
  _output2AutoBypass.update(Mycila::Config.getBool(KEY_OUTPUT2_AUTO_BYPASS_ENABLE));

  _output2Temp.update(Mycila::Config.getBool(KEY_OUTPUT2_TEMP_ENABLE));
  _output2TempPin.update(Mycila::Config.get(KEY_OUTPUT2_TEMP_PIN));
  _output2DimmerPowerLimit.update(static_cast<int>(Mycila::Config.get(KEY_OUTPUT2_DIMMER_LEVEL_LIMIT).toInt()));
  _output2AutoStartWDays.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_WEEK_DAYS));

  _output2AutoStartTemp.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_START_TEMPERATURE));
  _output2AutoStartTime.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_START_TIME));
  _output2AutoStoptTemp.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_STOP_TEMPERATURE));
  _output2AutoStoptTime.update(Mycila::Config.get(KEY_OUTPUT2_AUTO_STOP_TIME));

  _output2PZEM.update(Mycila::Config.getBool(KEY_OUTPUT2_PZEM_ENABLE));
  _output2PZEMRXPin.update(Mycila::Config.get(KEY_PZEM_RX_PIN));
  _output2PZEMTXPin.update(Mycila::Config.get(KEY_PZEM_TX_PIN));

  // relay1

  _relay1.update(Mycila::Config.getBool(KEY_RELAY1_ENABLE));
  _relay1Pin.update(Mycila::Config.get(KEY_RELAY1_PIN));
  _relay1Type.update(Mycila::Config.get(KEY_RELAY1_TYPE));
  _relay1Power.update(static_cast<int>(Mycila::Config.get(KEY_RELAY1_POWER).toInt()));

  // relay2

  _relay2.update(Mycila::Config.getBool(KEY_RELAY2_ENABLE));
  _relay2Pin.update(Mycila::Config.get(KEY_RELAY2_PIN));
  _relay2Type.update(Mycila::Config.get(KEY_RELAY2_TYPE));
  _relay2Power.update(static_cast<int>(Mycila::Config.get(KEY_RELAY2_POWER).toInt()));

  // system

  _systemTemp.update(Mycila::Config.getBool(KEY_SYSTEM_TEMP_ENABLE));
  _pushButton.update(Mycila::Config.getBool(KEY_BUTTON_ENABLE));
  _led.update(Mycila::Config.getBool(KEY_LIGHTS_ENABLE));
  _buzzer.update(Mycila::Config.getBool(KEY_BUZZER_ENABLE));
  _systemTempPin.update(Mycila::Config.get(KEY_SYSTEM_TEMP_PIN));
  _pushButtonPin.update(Mycila::Config.get(KEY_BUTTON_PIN));
  _pushButtonAction.update(Mycila::Config.get(KEY_BUTTON_ACTION));
  _ledGreenPin.update(Mycila::Config.get(KEY_LIGHTS_GREEN_PIN));
  _ledYellowPin.update(Mycila::Config.get(KEY_LIGHTS_YELLOW_PIN));
  _ledRedPin.update(Mycila::Config.get(KEY_LIGHTS_RED_PIN));
  _buzzerPin.update(Mycila::Config.get(KEY_BUZZER_PIN));
#endif

  if (!skipWebSocketPush && dashboard.hasClient()) {
    dashboard.sendUpdates();
  }
}

void YaSolR::WebsiteClass::_temperature(Card* card, Mycila::TemperatureSensor* sensor) {
  if (!sensor->isEnabled()) {
    card->update("Disabled", "");
  } else if (!sensor->isValid()) {
    card->update("Pending...", "");
  } else {
    card->update(sensor->getTemperature(), "°C");
  }
}

void YaSolR::WebsiteClass::_status(Card* card, const char* key, bool enabled, bool active, const char* err) {
  const bool configEnabled = Mycila::Config.getBool(key);
  if (!configEnabled)
    card->update("Disabled", DASH_STATUS_IDLE);
  else if (!enabled)
    card->update("Failed to start", DASH_STATUS_DANGER);
  else if (!active)
    card->update(err, DASH_STATUS_WARNING);
  else
    card->update("Enabled", DASH_STATUS_SUCCESS);
}

void YaSolR::WebsiteClass::_pinout(Card* card, int32_t pin, std::map<int32_t, Card*>* pinout) {
  if (pin == GPIO_NUM_NC) {
    card->update("Disabled", DASH_STATUS_IDLE);
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
