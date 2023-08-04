// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRouterOutput.h>

#include <MycilaLogger.h>
#include <MycilaNTP.h>
#include <MycilaTime.h>

#define TAG "OUTPUT"

static const char* RouterOutputStateNames[] = {
  "Disabled",
  "Idle",
  "Routing",
  "Manual Bypass",
  "Auto Bypass",
};

static const char* DaysOfWeek[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};

Mycila::RouterOutput::RouterOutput(const char* name) : name(name),
                                                       dimmer(name),
                                                       relay(name),
                                                       temperatureSensor(name) {}

void Mycila::RouterOutput::begin() {
  dimmer.begin();
  relay.begin();
  temperatureSensor.begin();

  if (isEnabled()) {
    Logger.debug(TAG, "Enable Router Output '%s'...", name);
    Logger.debug(TAG, " - Dimmer: %s", RouterOutputConfig.isAutoDimmerEnabled(name) ? "auto" : "manual");
    Logger.debug(TAG, " - Dimmer Level Limit: %u %", RouterOutputConfig.getDimmerLevelLimit(name));
    Logger.debug(TAG, " - Bypass: %s", RouterOutputConfig.isAutoBypassEnabled(name) ? "auto" : "manual");
    Logger.debug(TAG, " - Auto Time: %s to %s %s", RouterOutputConfig.getAutoStartTime(name).c_str(), RouterOutputConfig.getAutoStopTime(name).c_str(), RouterOutputConfig.getWeekDays(name).c_str());
    Logger.debug(TAG, " - Auto Start Temperature: %u °C", RouterOutputConfig.getAutoStartTemperature(name));
    Logger.debug(TAG, " - Auto Stop Temperature: %u °C", RouterOutputConfig.getAutoStopTemperature(name));

  } else {
    Logger.warn(TAG, "Disable Router Output '%s'...", name);
  }
}

void Mycila::RouterOutput::end() {
  _autoBypassEnabled = false;
  dimmer.off();
  relay.off();
  dimmer.end();
  relay.end();
  temperatureSensor.end();
}

void Mycila::RouterOutput::loop() {
  temperatureSensor.loop();

  if (!dimmer.isEnabled())
    return;

  if (millis() - _lastCheck >= 500) {
    // Auto BP rules
    _autoBypassLoop();

    // dimmer limit
    if (!_autoBypassEnabled && dimmer.isOn()) {
      uint8_t limit = RouterOutputConfig.getDimmerLevelLimit(dimmer.name);
      if (dimmer.getLevel() > limit) {
        Logger.warn(TAG, "Dimmer '%s' reached its limit at %u %", dimmer.name, limit);
        dimmer.setLevel(limit);
      }
    }

    _lastCheck = millis();
  }
}

bool Mycila::RouterOutput::tryBypassRelayState(bool switchOn) {
  if (_autoBypassEnabled && !switchOn) {
    Logger.warn(TAG, "Auto Bypass '%s' is activated: unable to turn of bypass relay", name);
    return false;
  }
  _setBypassRelay(switchOn);
  return true;
}

bool Mycila::RouterOutput::tryDimmerLevel(uint8_t level) {
  if (!dimmer.isEnabled())
    return false;

  if (_autoBypassEnabled) {
    Logger.warn(TAG, "Auto Bypass '%s' is activated: unable to change dimmer level", name);
    return false;
  }

  if (RouterOutputConfig.isAutoDimmerEnabled(name)) {
    Logger.warn(TAG, "Auto Dimmer '%s' is activated: unable to change dimmer level", name);
    return false;
  }

  Logger.debug(TAG, "Setting Dimmer '%s' level to %u %...", dimmer.name, level);
  relay.off();
  dimmer.setLevel(level);
  return true;
}

Mycila::RouterOutputState Mycila::RouterOutput::getState() const {
  if (!dimmer.isEnabled())
    return RouterOutputState::OUTPUT_DISABLED;
  if (dimmer.getLevel() > 0)
    return RouterOutputState::OUTPUT_ROUTING;
  // dimmer level is 0
  if (_autoBypassEnabled)
    return RouterOutputState::OUTPUT_BYPASS_AUTO;
  if (relay.isOn())
    return RouterOutputState::OUTPUT_BYPASS_MANUAL;
  return RouterOutputState::OUTPUT_IDLE;
}

const char* Mycila::RouterOutput::getStateString() const { return RouterOutputStateNames[static_cast<int>(getState())]; }

void Mycila::RouterOutput::toJson(const JsonObject& root) const {
  root["enabled"] = dimmer.isEnabled();
  root["state"] = RouterOutputStateNames[static_cast<int>(getState())];
  JsonObject dimmerJson = root["dimmer"].to<JsonObject>();
  dimmer.toJson(dimmerJson);
  JsonObject relayJson = root["bypass_relay"].to<JsonObject>();
  relay.toJson(relayJson);
  JsonObject temperatureSensorJson = root["temp_sensor"].to<JsonObject>();
  temperatureSensor.toJson(temperatureSensorJson);
}

void Mycila::RouterOutput::_autoBypassLoop() {
  if (!RouterOutputConfig.isAutoBypassEnabled(name)) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "Auto Bypass disabled: stopping Auto Bypass '%s'...", name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  if (!relay.isEnabled() && !dimmer.isEnabled()) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "Relay disabled: stopping Auto Bypass '%s'...", name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  if (!temperatureSensor.isEnabled()) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "Temperature Sensor disabled: stopping Auto Bypass '%s'...", name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  if (!temperatureSensor.isValid()) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "Temperature sensor is not answering: stopping Auto Bypass '%s'...", name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  const float temp = temperatureSensor.getTemperature();

  if (temp >= RouterOutputConfig.getAutoStopTemperature(name)) {
    if (_autoBypassEnabled) {
      Logger.info(TAG, "Temperature reached %.02f °C: stopping Auto Bypass '%s'...", temp, name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  if (temp >= RouterOutputConfig.getAutoStartTemperature(name)) {
    // temperature OK, no need to start
    return;
  }

  if (!NTP.isSynced()) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "NTP not available: stopping Auto Bypass '%s'...", name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 5)) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "Unable to get time: stopping Auto Bypass '%s'...", name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  const String autoStart = RouterOutputConfig.getAutoStartTime(name);
  const String autoStop = RouterOutputConfig.getAutoStopTime(name);
  const int inRange = Time::timeInRange(&timeInfo, autoStart, autoStop);
  if (inRange == -1) {
    if (_autoBypassEnabled) {
      Logger.warn(TAG, "Time range %s to %s is invalid: stopping Auto Bypass '%s'...", autoStart.c_str(), autoStop.c_str(), name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  if (!inRange) {
    if (_autoBypassEnabled) {
      Logger.info(TAG, "Time reached %s: stopping Auto Bypass '%s'...", autoStop.c_str(), name);
      _autoBypassEnabled = false;
      _setBypassRelay(false);
    }
    return;
  }

  // time and temp OK, let's start
  if (!_autoBypassEnabled) {
    // auto bypass is not enabled, let's start it
    const char* wday = DaysOfWeek[timeInfo.tm_wday];
    if (RouterOutputConfig.getWeekDays(name).indexOf(wday) >= 0) {
      Logger.info(TAG, "Time within %s-%s on %s: starting Auto Bypass '%s' at %.02f °C...", autoStart.c_str(), autoStop.c_str(), wday, name, temp);
      _autoBypassEnabled = true;
      _setBypassRelay(true);
    }
    return;
  }

  // auto bypass is enabled

  // relay is on ?
  if (relay.isOn())
    return;

  // or relay is disabled and dimmer at full power tu replace it ?
  if (!relay.isEnabled() && dimmer.isOnAtFullPower())
    return;

  // start bypass
  Logger.info(TAG, "Auto Bypass '%s' is activated: restarting Relay...", name);
  _setBypassRelay(true);
}

// _setBypassRelay
void Mycila::RouterOutput::_setBypassRelay(bool state) {
  if (relay.isEnabled()) {
    Logger.debug(TAG, "Turning %s Bypass Relay '%s'...", state ? "on" : "off", relay.name);
    if (state)
      dimmer.off();
    relay.setState(state);

  } else {
    Logger.debug(TAG, "Turning %s Dimmer '%s'...", state ? "on" : "off", dimmer.name);
    dimmer.setLevel(state ? 100 : 0);
  }
}

namespace Mycila {
  RouterOutputConfigClass RouterOutputConfig;
} // namespace Mycila
