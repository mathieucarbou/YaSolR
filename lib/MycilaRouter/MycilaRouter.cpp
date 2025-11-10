// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaRouter.h>

#include <MycilaUtilities.h>

#include <string>
#include <utility>

#define TAG "ROUTER"

/////////////
// Metrics //
/////////////

void Mycila::Router::toJson(const JsonObject& dest, const Metrics& metrics) {
  switch (metrics.source) {
    case Source::JSY:
      dest["source"] = "jsy";
      break;
    case Source::JSY_REMOTE:
      dest["source"] = "jsy_remote";
      break;
    case Source::COMPUTED:
      dest["source"] = "computed";
      break;
    case Source::PZEM:
      dest["source"] = "pzem";
      break;
    default:
      dest["source"] = "unknown";
      break;
  }

  dest["energy"] = metrics.energy;
  if (!std::isnan(metrics.voltage))
    dest["voltage"] = metrics.voltage;
  if (!std::isnan(metrics.current))
    dest["current"] = metrics.current;
  if (!std::isnan(metrics.power))
    dest["power"] = metrics.power;
  if (!std::isnan(metrics.apparentPower))
    dest["apparent_power"] = metrics.apparentPower;
  if (!std::isnan(metrics.powerFactor))
    dest["power_factor"] = metrics.powerFactor;
  if (!std::isnan(metrics.resistance))
    dest["resistance"] = metrics.resistance;
  if (!std::isnan(metrics.thdi))
    dest["thdi"] = metrics.thdi;
}

//////////////////
// Router Relay //
//////////////////

bool Mycila::Router::Relay::trySwitchRelay(bool state, uint32_t duration) {
  if (!_relay.isEnabled())
    return false;

  if (duration)
    ESP_LOGI(TAG, "Switching relay on pin %u %s for %u ms", _relay.getPin(), state ? "ON" : "OFF", duration);
  else
    ESP_LOGI(TAG, "Switching relay on pin %u %s", _relay.getPin(), state ? "ON" : "OFF");
  _relay.setState(state, duration);
  return true;
}

bool Mycila::Router::Relay::autoSwitch(float gridVoltage, float gridPower, float routedPower, float setpoint) {
  if (!isAutoRelayEnabled())
    return false;

  // * setpoint is the grid power to be reached, configured in the PID section. Example 1: 1000W | Example 2: 0W
  // * routedPower is the power routed to the load through the dimmer. Example1: 800W | Example 2: 800W
  // * gridPower is the power coming from or going to the grid. Example1: 1000W | Example 2: 0W (800W routing + 200W home consumption)
  // * virtualGridPower is the power that would come from or go to the grid if the routing was off. Example1: 200W | Example 2: -800W
  // * relayRoomPower is the power that is available for a relay. Example1: 800W | Example 2: 800W

  // detects the grid nominal voltage
  const uint16_t nominalVoltage = static_cast<uint8_t>(gridVoltage / 100) == 1 ? 110 : 230;
  // compute the real load with the grid voltage
  const uint16_t adjustedLoad = computeLoad(gridVoltage);

  ESP_LOGD(TAG, "Auto-Switching relay on pin %u ? Nominal load: %" PRIu16 " W @ %" PRIu16 " V, Load: %" PRIu16 " W @ %" PRIu16 " V, Grid: %.1f W, Routed: %.1f W, Setpoint: %.1f W, Tolerance: %.2f %%", _relay.getPin(), _nominalLoad, nominalVoltage, adjustedLoad, static_cast<uint16_t>(gridVoltage), gridPower, routedPower, setpoint, _tolerance * 100.0f);

  if (_relay.isOff()) {
    const float relayRoomPower = setpoint + routedPower - gridPower;
    if (relayRoomPower >= adjustedLoad * (1.0f + _tolerance)) {
      ESP_LOGI(TAG, "Auto-Switching relay on pin %u %s", _relay.getPin(), "ON");
      _relay.setState(true);
      return true;
    }
    return false;
  }

  if (_relay.isOn()) {
    const float relayRoomPower = setpoint + routedPower - gridPower + adjustedLoad;
    if (relayRoomPower <= adjustedLoad * (1.0f - _tolerance)) {
      ESP_LOGI(TAG, "Auto-Switching relay on pin %u %s", _relay.getPin(), "OFF");
      _relay.setState(false);
      return true;
    }
    return false;
  }

  return false;
}

////////////
// Output //
////////////

static const char* StateNames[] = {
  "UNUSED",
  "IDLE",
  "ROUTING",
  "BYPASS_MANUAL",
  "BYPASS_AUTO",
};

static const char* DaysOfWeek[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};

const char* Mycila::Router::Output::getStateName() const { return StateNames[static_cast<int>(getState())]; }

#ifdef MYCILA_JSON_SUPPORT
void Mycila::Router::Output::toJson(const JsonObject& root, float gridVoltage) const {
  root["bypass"] = isBypassOn() ? "on" : "off";
  root["enabled"] = isDimmerOnline();
  root["state"] = getStateName();
  root["elapsed"] = getBypassUptime();
  float t = _temperature.orElse(NAN);
  if (!std::isnan(t)) {
    root["temperature"] = t;
  }

  {
    Metrics* metrics = new Metrics();
    readMeasurements(*metrics);
    Router::toJson(root["measurements"].to<JsonObject>(), *metrics);
    delete metrics;
  }

  JsonArray sources = root["sources"].to<JsonArray>();

  {
    Metrics* metrics = new Metrics();
    computeMetrics(*metrics, gridVoltage);
    Router::toJson(sources.add<JsonObject>(), *metrics);
    delete metrics;
  }

  if (_metrics.isPresent()) {
    JsonObject source = sources.add<JsonObject>();
    Router::toJson(source, _metrics.get());
    source["time"] = _metrics.getLastUpdateTime();
  }

  _dimmer->toJson(root["dimmer"].to<JsonObject>());
  if (_relay)
    _relay->toJson(root["relay"].to<JsonObject>());
}
#endif

bool Mycila::Router::Output::setDimmerDutyCycle(float dutyCycle) {
  if (_autoBypassEnabled) {
    ESP_LOGW(TAG, "Auto Bypass '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (config.autoDimmer) {
    ESP_LOGW(TAG, "Auto Dimmer '%s' is activated: unable to change dimmer level", _name);
    return false;
  }

  if (dutyCycle > 0 && isDimmerTemperatureLimitReached()) {
    ESP_LOGW(TAG, "Dimmer '%s' reached its temperature limit of %.02f °C", _name, static_cast<float>(config.dimmerTempLimit));
    return false;
  }

  if (_manualBypassEnabled) {
    ESP_LOGW(TAG, "Bypass '%s' disabled by dimmer activation", _name);
    setBypass(false);
  }

  _dimmer->setDutyCycle(dutyCycle);

  ESP_LOGI(TAG, "Set Dimmer '%s' duty to %f", _name, _dimmer->getDutyCycle());

  return true;
}

void Mycila::Router::Output::applyTemperatureLimit() {
  if (_autoBypassEnabled)
    return;

  if (_manualBypassEnabled)
    return;

  if (_dimmer->isOff())
    return;

  if (isDimmerTemperatureLimitReached()) {
    ESP_LOGW(TAG, "Dimmer '%s' reached its temperature limit of %.02f °C", _name, config.dimmerTempLimit);
    _dimmer->off();
    return;
  }
}

// bypass

void Mycila::Router::Output::setBypass(bool state) {
  if (_autoBypassEnabled) {
    ESP_LOGW(TAG, "Auto Bypass '%s' is activated: bypass relay state won't be updated", _name);
  } else {
    _switchBypass(state);
  }
  // Still update manual bypass states to support keep-alive feature.
  // If bypass relay is already on, and setBypass(true) is called again, we update the time.
  // This allows an HA integration to use the keep-alive mechanism, knowing that the forced bypass will timeout if no keep-alive is sent.
  _manualBypassEnabled = state;
  _manualBypassTime = state ? millis() : 0;
}

void Mycila::Router::Output::applyAutoBypass() {
  if (!isAutoBypassEnabled()) {
    if (_autoBypassEnabled) {
      ESP_LOGW(TAG, "Auto Bypass disabled: stopping Auto Bypass '%s'", _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  // time checks

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 5)) {
    if (_autoBypassEnabled) {
      ESP_LOGW(TAG, "Unable to get time: stopping Auto Bypass '%s'", _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  // temperature checks

  if (!_temperature.neverUpdated()) {
    if (!_temperature.isPresent()) {
      if (_autoBypassEnabled) {
        ESP_LOGW(TAG, "Invalid temperature sensor value: stopping Auto Bypass '%s'", _name);
        _autoBypassEnabled = false;
        _switchBypass(_manualBypassEnabled);
      }
      return;
    }

    const float temp = _temperature.get();

    if (temp >= config.autoStopTemperature) {
      if (_autoBypassEnabled) {
        ESP_LOGI(TAG, "Temperature reached %.02f °C: stopping Auto Bypass '%s'", temp, _name);
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

  const int inRange = Mycila::Time::timeInRange(timeInfo, config.autoStartTime.c_str(), config.autoStopTime.c_str());
  if (inRange == -1) {
    if (_autoBypassEnabled) {
      ESP_LOGW(TAG, "Time range %s to %s is invalid: stopping Auto Bypass '%s'", config.autoStartTime.c_str(), config.autoStopTime.c_str(), _name);
      _autoBypassEnabled = false;
      _switchBypass(_manualBypassEnabled);
    }
    return;
  }

  if (!inRange) {
    if (_autoBypassEnabled) {
      ESP_LOGI(TAG, "Time reached %s: stopping Auto Bypass '%s'", config.autoStopTime.c_str(), _name);
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
      ESP_LOGI(TAG, "Time within %s-%s on %s: starting Auto Bypass '%s' at %.02f °C", config.autoStartTime.c_str(), config.autoStopTime.c_str(), wday, _name, _temperature.orElse(0));
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
  ESP_LOGI(TAG, "Auto Bypass '%s' must be restarted", _name);
  _switchBypass(true);
}

void Mycila::Router::Output::applyBypassTimeout() {
  const uint32_t elapsed = getBypassUptime();

  if (!elapsed)
    return;

  if (elapsed >= config.bypassTimeoutSec) {
    ESP_LOGI(TAG, "Manual Bypass '%s' timeout!", _name);
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

void Mycila::Router::Output::_switchBypass(bool state, bool log) {
  if (state) {
    // we want to activate bypass
    if (isBypassRelayEnabled()) {
      // we have a relay in-place: use it
      if (log && !isBypassRelayOn()) {
        ESP_LOGI(TAG, "Turning Bypass Relay '%s' ON", _name);
      }
      _dimmer->off();
      _relay->setState(true);
    } else {
      // we don't have a relay: use the dimmer
      if (log) {
        ESP_LOGI(TAG, "Turning Dimmer '%s' ON", _name);
      }
      _dimmer->on();
    }
  } else {
    // we want to deactivate bypass
    if (isBypassRelayEnabled()) {
      if (log && isBypassRelayOn()) {
        ESP_LOGI(TAG, "Turning Bypass Relay '%s' OFF", _name);
      }
      _relay->setState(false);
    } else {
      if (log) {
        ESP_LOGI(TAG, "Turning Dimmer '%s' OFF", _name);
      }
      _dimmer->off();
    }
  }
}

////////////
// Router //
////////////

#ifdef MYCILA_JSON_SUPPORT
void Mycila::Router::toJson(const JsonObject& root, float gridVoltage) const {
  {
    Metrics* routerMeasurements = new Metrics();
    readMeasurements(*routerMeasurements);
    Router::toJson(root["measurements"].to<JsonObject>(), *routerMeasurements);
    delete routerMeasurements;
  }

  JsonArray sources = root["sources"].to<JsonArray>();

  {
    Metrics* metrics = new Metrics();
    computeMetrics(*metrics, gridVoltage);
    Router::toJson(sources.add<JsonObject>(), *metrics);
    delete metrics;
  }

  if (_metrics.isPresent()) {
    JsonObject source = sources.add<JsonObject>();
    Router::toJson(source, _metrics.get());
    source["time"] = _metrics.getLastUpdateTime();
  }
}
#endif

void Mycila::Router::beginCalibration(size_t outputIndex, CalibrationCallback cb) {
  if (_calibrationRunning) {
    ESP_LOGW(TAG, "Calibration already running");
    return;
  }

  if (_outputs.empty()) {
    ESP_LOGW(TAG, "No output to calibrate");
    return;
  }

  if (outputIndex >= _outputs.size()) {
    ESP_LOGW(TAG, "Invalid output index %d for calibration", outputIndex);
    return;
  }

  // find if at least one output is online
  if (!_outputs[outputIndex]->isDimmerOnline()) {
    ESP_LOGW(TAG, "Unable to calibrate: output %s is not online", _outputs[outputIndex]->getName());
    return;
  }

  ESP_LOGI(TAG, "Starting calibration");
  _calibrationStep = 1;
  _calibrationOutputIndex = outputIndex;
  _calibrationCallback = cb;
  _calibrationRunning = true;
}

void Mycila::Router::continueCalibration() {
  if (!_calibrationRunning)
    return;

  switch (_calibrationStep) {
    case 1: {
      ESP_LOGI(TAG, "Disabling Auto Dimmer and Auto Bypass on all outputs and turning them off");
      for (const auto& output : _outputs) {
        output->config.autoBypass = false;
        output->config.autoDimmer = false;
        ESP_LOGI(TAG, "Disabling %s Auto Bypass", output->getName());
        output->applyAutoBypass();
        ESP_LOGI(TAG, "Turing off %s", output->getName());
        output->setDimmerOff();
        output->setBypassOff();
      }
      _calibrationStep++;
      break;
    }
    case 2:
      ESP_LOGI(TAG, "Activating %s dimmer at 50%", _outputs[_calibrationOutputIndex]->getName());
      _outputs[_calibrationOutputIndex]->setDimmerDutyCycle(.5);
      _calibrationStartTime = millis();
      _calibrationStep++;
      break;

    case 3:
      if (millis() - _calibrationStartTime > 5000) {
        ESP_LOGI(TAG, "Measuring %s resistance", _outputs[_calibrationOutputIndex]->getName());

        float resistance = _outputs[_calibrationOutputIndex]->readResistance().value_or(NAN);
        if (std::isnan(resistance)) {
          resistance = readResistance().value_or(NAN);
        }

        _outputs[_calibrationOutputIndex]->config.calibratedResistance = resistance;

        ESP_LOGI(TAG, "Activating %s dimmer at 100%", _outputs[_calibrationOutputIndex]->getName());
        _outputs[_calibrationOutputIndex]->setDimmerDutyCycle(1);
        _calibrationStartTime = millis();
        _calibrationStep++;
      }
      break;

    case 4:
      if (millis() - _calibrationStartTime > 5000) {
        ESP_LOGI(TAG, "Measuring %s resistance", _outputs[_calibrationOutputIndex]->getName());

        float resistance = _outputs[_calibrationOutputIndex]->readResistance().value_or(NAN);
        if (std::isnan(resistance)) {
          resistance = readResistance().value_or(NAN);
        }

        _outputs[_calibrationOutputIndex]->config.calibratedResistance = (_outputs[_calibrationOutputIndex]->config.calibratedResistance + resistance) / 2;

        ESP_LOGI(TAG, "Turning off %s dimmer", _outputs[_calibrationOutputIndex]->getName());
        _outputs[_calibrationOutputIndex]->setDimmerOff();

        _calibrationRunning = false;
        ESP_LOGI(TAG, "Calibration done");
        if (_calibrationCallback)
          _calibrationCallback();
      }
      break;

    default:
      break;
  }
}
