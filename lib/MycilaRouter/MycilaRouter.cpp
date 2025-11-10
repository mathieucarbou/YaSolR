// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaRouter.h>

#define TAG "ROUTER"

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
