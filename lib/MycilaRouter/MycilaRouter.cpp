// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaRouter.h>

#define TAG "ROUTER"

// Mycila::Router

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

// Mycila::Router::Relay

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
  // calculate the real load with the grid voltage
  const uint16_t adjustedLoad = getLoad(gridVoltage);

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
