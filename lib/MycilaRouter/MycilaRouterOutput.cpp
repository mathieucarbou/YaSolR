// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRouterOutput.h>

#include <MycilaNTP.h>
#include <MycilaTime.h>

#ifdef MYCILA_LOGGER_SUPPORT
  #include <MycilaLogger.h>
extern Mycila::Logger logger;
  #define LOGD(tag, format, ...) logger.debug(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) logger.info(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) logger.warn(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) logger.error(tag, format, ##__VA_ARGS__)
#else
  #define LOGD(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
  #define LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
  #define LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
  #define LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

#define TAG "OUTPUT"

static const char* RouterOutputStateNames[] = {
  "Disabled",
  "Idle",
  "Routing",
  "Bypass",
  "Auto Bypass",
};

static const char* DaysOfWeek[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};

const char* Mycila::RouterOutput::getStateName() const { return RouterOutputStateNames[static_cast<int>(getState())]; }

// dimmer

bool Mycila::RouterOutput::tryDimmerDuty(uint16_t duty) {
  if (!_dimmer->isEnabled())
    return false;

  if (_autoBypassEnabled) {
    LOGW(TAG, "Auto Bypass '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (config.autoDimmer) {
    LOGW(TAG, "Auto Dimmer '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (duty > config.dimmerLimit) {
    duty = config.dimmerLimit;
  }

  LOGD(TAG, "Setting Dimmer '%s' duty to %" PRIu16 "...", _name, duty);
  _setBypass(false, duty);
  return true;
}

void Mycila::RouterOutput::applyDimmerLimit() {
  if (!_dimmer->isEnabled())
    return;
  if (_autoBypassEnabled)
    return;
  if (_bypassEnabled)
    return;
  if (_dimmer->isOff())
    return;
  if (_dimmer->getPowerDuty() > config.dimmerLimit) {
    LOGW(TAG, "Dimmer '%s' reached its limit at %" PRIu16, _name, config.dimmerLimit);
    _dimmer->setPowerDuty(config.dimmerLimit);
  }
}

// bypass

bool Mycila::RouterOutput::tryBypassState(bool switchOn) {
  if (_autoBypassEnabled && !switchOn) {
    LOGW(TAG, "Auto Bypass '%s' is activated: unable to turn of bypass relay", _name);
    return false;
  }
  _setBypass(switchOn);
  return true;
}

void Mycila::RouterOutput::applyAutoBypass() {
  if (!config.autoBypass) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Auto Bypass disabled: stopping Auto Bypass '%s'...", _name);
      _autoBypassEnabled = false;
      _setBypass(false);
    }
    return;
  }

  // dimmer & relay checks

  if (!_relay->isEnabled() && !_dimmer->isEnabled()) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Relay and dimmer disabled: stopping Auto Bypass '%s'...", _name);
      _autoBypassEnabled = false;
      _setBypass(false);
    }
    return;
  }

  // time checks

  if (!NTP.isSynced()) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "NTP not available: stopping Auto Bypass '%s'...", _name);
      _autoBypassEnabled = false;
      _setBypass(false);
    }
    return;
  }

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 5)) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Unable to get time: stopping Auto Bypass '%s'...", _name);
      _autoBypassEnabled = false;
      _setBypass(false);
    }
    return;
  }

  // temperature checks

  const float temp = _temperatureSensor->getLastTemperature();

  if (_temperatureSensor->isEnabled()) {
    if (_temperatureSensor->getLastTime() == 0) {
      if (_autoBypassEnabled) {
        LOGW(TAG, "Temperature sensor is not answering: stopping Auto Bypass '%s'...", _name);
        _autoBypassEnabled = false;
        _setBypass(false);
      }
      return;
    }

    if (temp >= config.autoStopTemperature) {
      if (_autoBypassEnabled) {
        LOGI(TAG, "Temperature reached %.02f °C: stopping Auto Bypass '%s'...", temp, _name);
        _autoBypassEnabled = false;
        _setBypass(false);
      }
      return;
    }

    if (temp >= config.autoStartTemperature) {
      // temperature OK, no need to start
      return;
    }
  }

  const int inRange = Time::timeInRange(timeInfo, config.autoStartTime, config.autoStopTime);
  if (inRange == -1) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Time range %s to %s is invalid: stopping Auto Bypass '%s'...", config.autoStartTime.c_str(), config.autoStopTime.c_str(), _name);
      _autoBypassEnabled = false;
      _setBypass(false);
    }
    return;
  }

  if (!inRange) {
    if (_autoBypassEnabled) {
      LOGI(TAG, "Time reached %s: stopping Auto Bypass '%s'...", config.autoStopTime.c_str(), _name);
      _autoBypassEnabled = false;
      _setBypass(false);
    }
    return;
  }

  // time and temp OK, let's start
  if (!_autoBypassEnabled) {
    // auto bypass is not enabled, let's start it
    const char* wday = DaysOfWeek[timeInfo.tm_wday];
    if (config.weekDays.indexOf(wday) >= 0) {
      LOGI(TAG, "Time within %s-%s on %s: starting Auto Bypass '%s' at %.02f °C...", config.autoStartTime.c_str(), config.autoStopTime.c_str(), wday, _name, temp);
      _autoBypassEnabled = true;
      _setBypass(true);
    }
    return;
  }

  // auto bypass is enabled

  // relay is on ?
  if (_relay->isOn())
    return;

  // or relay is disabled and dimmer at full power tu replace it ?
  if (!_relay->isEnabled() && _dimmer->isOnAtFullPower())
    return;

  // start bypass
  LOGI(TAG, "Auto Bypass '%s' is activated: restarting Relay...", _name);
  _setBypass(true);
}

void Mycila::RouterOutput::_setBypass(bool state, uint16_t dimmerDutyWhenRelayOff) {
  if (_relay->isEnabled()) {
    if (state)
      _dimmer->off();
    if (state ^ _relay->isOn()) {
      LOGD(TAG, "Turning %s Bypass Relay '%s'...", state ? "on" : "off", _name);
      _relay->setState(state);
    }
    if (!state)
      _dimmer->setPowerDuty(dimmerDutyWhenRelayOff);
  } else {
    LOGD(TAG, "Turning %s Dimmer '%s'...", state ? "on" : "off", _name);
    _dimmer->setPowerDuty(state ? MYCILA_DIMMER_MAX_LEVEL : dimmerDutyWhenRelayOff);
  }
  _bypassEnabled = state;
}
