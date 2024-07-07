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
  if (!_dimmer->isEnabled()) {
    LOGW(TAG, "Dimmer '%s' is disabled", _name);
    return false;
  }

  if (_autoBypassEnabled) {
    LOGW(TAG, "Auto Bypass '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (config.autoDimmer) {
    LOGW(TAG, "Auto Dimmer '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (isDimmerTemperatureLimitReached()) {
    LOGW(TAG, "Dimmer '%s' reached its temperature limit of %.02f °C", _name, config.dimmerTempLimit);
    return false;
  }

  if (duty > config.dimmerDutyLimit) {
    duty = config.dimmerDutyLimit;
  }

  _setBypass(false);

  LOGD(TAG, "Setting Dimmer '%s' duty to %" PRIu16 "...", _name, duty);
  _dimmer->setDuty(duty);
  return true;
}

void Mycila::RouterOutput::applyDimmerLimits() {
  if (_autoBypassEnabled)
    return;

  if (_bypassEnabled)
    return;

  if (_dimmer->isOff())
    return;

  if (_dimmer->getDuty() > config.dimmerDutyLimit) {
    LOGW(TAG, "Dimmer '%s' reached its duty limit at %" PRIu16, _name, config.dimmerDutyLimit);
    _dimmer->setDuty(config.dimmerDutyLimit);
    return;
  }

  if (isDimmerTemperatureLimitReached()) {
    LOGW(TAG, "Dimmer '%s' reached its temperature limit of %.02f °C", _name, config.dimmerTempLimit);
    _dimmer->off();
    return;
  }
}

// bypass

bool Mycila::RouterOutput::tryBypassState(bool switchOn) {
  if (_autoBypassEnabled && !switchOn) {
    LOGW(TAG, "Auto Bypass '%s' is activated: unable to turn of bypass relay", _name);
    return false;
  }
  _setBypass(switchOn);
  return _bypassEnabled;
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

  const float temp = _temperatureSensor->getValidTemperature();

  if (_temperatureSensor->isEnabled()) {
    if (!_temperatureSensor->isValid()) {
      if (_autoBypassEnabled) {
        LOGW(TAG, "Invalid temperature sensor value: stopping Auto Bypass '%s'...", _name);
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
    if (!_relay->isEnabled() && !_dimmer->isEnabled()) {
      return;
    }
    const char* wday = DaysOfWeek[timeInfo.tm_wday];
    if (config.weekDays.indexOf(wday) >= 0) {
      LOGI(TAG, "Time within %s-%s on %s: starting Auto Bypass '%s' at %.02f °C...", config.autoStartTime.c_str(), config.autoStopTime.c_str(), wday, _name, temp);
      _setBypass(true);
      _autoBypassEnabled = _bypassEnabled;
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

void Mycila::RouterOutput::_setBypass(bool state, bool log) {
  if (state) {
    // we want to activate bypass
    if (_relay->isEnabled()) {
      // we have a relay in-place: use it
      _dimmer->off();
      if (_relay->isOff()) {
        if (log)
          LOGD(TAG, "Turning Bypass Relay '%s' ON...", _name);
        _relay->setState(true);
      }
      _bypassEnabled = true;

    } else {
      // we don't have a relay: use the dimmer
      if (_dimmer->isEnabled()) {
        if (log)
          LOGD(TAG, "Turning Dimmer '%s' ON...", _name);
        _dimmer->setDuty(MYCILA_DIMMER_MAX_DUTY);
        _bypassEnabled = true;
      } else {
        if (log)
          LOGW(TAG, "Dimmer '%s' is not connected to the grid: unable to activate bypass", _name);
        _bypassEnabled = false;
      }
    }

  } else {
    // we want to deactivate bypass
    if (_relay->isEnabled()) {
      if (_relay->isOn()) {
        if (log)
          LOGD(TAG, "Turning Bypass Relay '%s' OFF...", _name);
        _relay->setState(false);
      }
    } else {
      if (log)
        LOGD(TAG, "Turning Dimmer '%s' OFF...", _name);
      _dimmer->off();
    }
    _bypassEnabled = false;
  }
}
