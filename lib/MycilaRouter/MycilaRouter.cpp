// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaRouter.h>

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

#define TAG "ROUTER"

void Mycila::Router::beginCalibration(size_t outputIndex, CalibrationCallback cb) {
  if (_calibrationRunning) {
    LOGW(TAG, "Calibration already running");
    return;
  }

  if (_outputs.empty()) {
    LOGW(TAG, "No output to calibrate");
    return;
  }

  if (outputIndex >= _outputs.size()) {
    LOGW(TAG, "Invalid output index %d for calibration", outputIndex);
    return;
  }

  // find if at least one output is online
  if (!_outputs[outputIndex]->isDimmerOnline()) {
    LOGW(TAG, "Unable to calibrate: output %s is not online", _outputs[outputIndex]->getName());
    return;
  }

  LOGI(TAG, "Starting calibration");
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
      LOGI(TAG, "Disabling Auto Dimmer and Auto Bypass on all outputs and turning them off");
      for (const auto& output : _outputs) {
        output->config.autoBypass = false;
        output->config.autoDimmer = false;
        LOGI(TAG, "Disabling %s Auto Bypass", output->getName());
        output->applyAutoBypass();
        LOGI(TAG, "Turing off %s", output->getName());
        output->setDimmerOff();
        output->setBypassOff();
      }
      _calibrationStep++;
      break;
    }
    case 2:
      LOGI(TAG, "Activating %s dimmer at 50%", _outputs[_calibrationOutputIndex]->getName());
      _outputs[_calibrationOutputIndex]->setDimmerDutyCycle(.5);
      _calibrationStartTime = millis();
      _calibrationStep++;
      break;

    case 3:
      if (millis() - _calibrationStartTime > 5000) {
        LOGI(TAG, "Measuring %s resistance", _outputs[_calibrationOutputIndex]->getName());
        RouterOutput::Metrics outputMetrics;
        _outputs[_calibrationOutputIndex]->readMeasurements(outputMetrics);
        float resistance = outputMetrics.resistance > 0 ? outputMetrics.resistance : 0; // handles nan
        if (!resistance) {
          Router::Metrics measurements;
          readMeasurements(measurements);
          resistance = measurements.resistance;
        }

        _outputs[_calibrationOutputIndex]->config.calibratedResistance = resistance;

        LOGI(TAG, "Activating %s dimmer at 100%", _outputs[_calibrationOutputIndex]->getName());
        _outputs[_calibrationOutputIndex]->setDimmerDutyCycle(1);
        _calibrationStartTime = millis();
        _calibrationStep++;
      }
      break;

    case 4:
      if (millis() - _calibrationStartTime > 5000) {
        LOGI(TAG, "Measuring %s resistance", _outputs[_calibrationOutputIndex]->getName());
        RouterOutput::Metrics outputMetrics;
        _outputs[_calibrationOutputIndex]->readMeasurements(outputMetrics);
        float resistance = outputMetrics.resistance > 0 ? outputMetrics.resistance : 0; // handles nan

        if (!resistance) {
          Router::Metrics measurements;
          readMeasurements(measurements);
          resistance = measurements.resistance;
        }

        _outputs[_calibrationOutputIndex]->config.calibratedResistance = (_outputs[_calibrationOutputIndex]->config.calibratedResistance + resistance) / 2;

        LOGI(TAG, "Turning off %s dimmer", _outputs[_calibrationOutputIndex]->getName());
        _outputs[_calibrationOutputIndex]->setDimmerOff();

        _calibrationRunning = false;
        LOGI(TAG, "Calibration done");
        if (_calibrationCallback)
          _calibrationCallback();
      }
      break;

    default:
      break;
  }
}
