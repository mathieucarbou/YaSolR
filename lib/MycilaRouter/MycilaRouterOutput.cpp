// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaRouterOutput.h>

#include <MycilaTime.h>

#include <string>

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

static const char* StateNames[] = {
  "DISABLED",
  "IDLE",
  "ROUTING",
  "BYPASS_MANUAL",
  "BYPASS_AUTO",
};

static const char* DaysOfWeek[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};

const char* Mycila::RouterOutput::getStateName() const { return StateNames[static_cast<int>(getState())]; }

// dimmer

bool Mycila::RouterOutput::setDimmerDutyCycle(float dutyCycle) {
  if (_autoBypassEnabled) {
    LOGW(TAG, "Auto Bypass '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (config.autoDimmer) {
    LOGW(TAG, "Auto Dimmer '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (dutyCycle > 0 && isDimmerTemperatureLimitReached()) {
    LOGW(TAG, "Dimmer '%s' reached its temperature limit of %.02f °C", _name, config.dimmerTempLimit);
    return false;
  }

  if (_manualBypassEnabled) {
    LOGW(TAG, "Bypass '%s' disabled by dimmer activation", _name);
    setBypass(false);
  }

  _dimmer->setDutyCycle(dutyCycle);

  LOGD(TAG, "Set Dimmer '%s' duty to %f", _name, _dimmer->getDutyCycle());

  return true;
}

void Mycila::RouterOutput::applyTemperatureLimit() {
  if (_autoBypassEnabled)
    return;

  if (_manualBypassEnabled)
    return;

  if (_dimmer->isOff())
    return;

  if (isDimmerTemperatureLimitReached()) {
    LOGW(TAG, "Dimmer '%s' reached its temperature limit of %.02f °C", _name, config.dimmerTempLimit);
    _dimmer->off();
    return;
  }
}

// bypass

void Mycila::RouterOutput::setBypass(bool state) {
  if (_autoBypassEnabled) {
    LOGW(TAG, "Auto Bypass '%s' is activated: bypass relay state won't be updated", _name);
  } else {
    _switchBypass(state);
  }
  // Still update manual bypass states to support keep-alive feature.
  // If bypass relay is already on, and setBypass(true) is called again, we update the time.
  // This allows an HA integration to use the keep-alive mechanism, knowing that the forced bypass will timeout if no keep-alive is sent.
  _manualBypassEnabled = state;
  _manualBypassTime = state ? millis() : 0;
}

void Mycila::RouterOutput::applyAutoBypass() {
  if (!isAutoBypassEnabled()) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Auto Bypass disabled: stopping Auto Bypass '%s'", _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  // time checks

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 5)) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Unable to get time: stopping Auto Bypass '%s'", _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  // temperature checks

  if (!_temperature.neverUpdated()) {
    if (!_temperature.isPresent()) {
      if (_autoBypassEnabled) {
        LOGW(TAG, "Invalid temperature sensor value: stopping Auto Bypass '%s'", _name);
        _autoBypassEnabled = false;
        _switchBypass(_manualBypassEnabled);
      }
      return;
    }

    const float temp = _temperature.get();

    if (temp >= config.autoStopTemperature) {
      if (_autoBypassEnabled) {
        LOGI(TAG, "Temperature reached %.02f °C: stopping Auto Bypass '%s'", temp, _name);
        _autoBypassEnabled = false;
        _switchBypass(_manualBypassEnabled);
      }
      return;
    }

    if (temp >= config.autoStartTemperature) {
      // temperature OK, no need to start
      return;
    }
  }

  const int inRange = Time::timeInRange(timeInfo, config.autoStartTime.c_str(), config.autoStopTime.c_str());
  if (inRange == -1) {
    if (_autoBypassEnabled) {
      LOGW(TAG, "Time range %s to %s is invalid: stopping Auto Bypass '%s'", config.autoStartTime.c_str(), config.autoStopTime.c_str(), _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  if (!inRange) {
    if (_autoBypassEnabled) {
      LOGI(TAG, "Time reached %s: stopping Auto Bypass '%s'", config.autoStopTime.c_str(), _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  // time and temp OK, let's start
  if (!_autoBypassEnabled) {
    // auto bypass is not enabled, let's start it
    const char* wday = DaysOfWeek[timeInfo.tm_wday];
    if (config.weekDays.find(wday) != std::string::npos) {
      LOGI(TAG, "Time within %s-%s on %s: starting Auto Bypass '%s' at %.02f °C", config.autoStartTime.c_str(), config.autoStopTime.c_str(), wday, _name, _temperature.orElse(0));
      _switchBypass(true);
      _autoBypassEnabled = true;
    }
    return;
  }

  // auto bypass is enabled (_autoBypassEnabled == true)

  // relay is on ?
  if (isBypassRelayOn())
    return;

  // or relay is disabled and dimmer at full power to replace it ?
  if (!isBypassRelayEnabled() && _dimmer->isOnAtFullPower())
    return;

  // start bypass
  LOGI(TAG, "Auto Bypass '%s' must be restarted", _name);
  _switchBypass(true);
}

// applyBypassTimeout
void Mycila::RouterOutput::applyBypassTimeout() {
  const uint32_t elapsed = getBypassUptime();

  if (!elapsed)
    return;

  if (elapsed >= config.bypassTimeoutSec) {
    LOGI(TAG, "Manual Bypass '%s' timeout!", _name);
    if (_autoBypassEnabled || !_manualBypassEnabled) {
      // auto-bypass is enabled, or we are routing => do not touch the relay states
      _manualBypassTime = 0;
      _manualBypassEnabled = false;
    } else {
      // we are in manual bypass mode => turn off the relay
      setBypass(false);
    }
  }
}

// private

void Mycila::RouterOutput::_switchBypass(bool state, bool log) {
  if (state) {
    // we want to activate bypass
    if (isBypassRelayEnabled()) {
      // we have a relay in-place: use it
      if (log && !isBypassRelayOn()) {
        LOGD(TAG, "Turning Bypass Relay '%s' ON", _name);
      }
      _dimmer->off();
      _relay->setState(true);
    } else {
      // we don't have a relay: use the dimmer
      if (log) {
        LOGD(TAG, "Turning Dimmer '%s' ON", _name);
      }
      _dimmer->on();
    }
  } else {
    // we want to deactivate bypass
    if (isBypassRelayEnabled()) {
      if (log && isBypassRelayOn()) {
        LOGD(TAG, "Turning Bypass Relay '%s' OFF", _name);
      }
      _relay->setState(false);
    } else {
      if (log) {
        LOGD(TAG, "Turning Dimmer '%s' OFF", _name);
      }
      _dimmer->off();
    }
  }
}
