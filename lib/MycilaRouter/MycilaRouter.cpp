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

#ifdef MYCILA_JSON_SUPPORT
void Mycila::Router::toJson(const JsonObject& root, float voltage) const {
  Metrics routerMeasurements;
  getRouterMeasurements(routerMeasurements);
  toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

  Metrics Metrics;
  getRouterMetrics(Metrics, voltage);
  toJson(root["metrics"].to<JsonObject>(), Metrics);

  JsonObject local = root["source"]["local"].to<JsonObject>();
  if (_localMetrics.isPresent()) {
    local["enabled"] = true;
    local["time"] = _localMetrics.getLastUpdateTime();
    toJson(local, _localMetrics.get());
  } else {
    local["enabled"] = false;
  }

  JsonObject remote = root["source"]["remote"].to<JsonObject>();
  if (_remoteMetrics.isPresent()) {
    remote["enabled"] = true;
    remote["time"] = _remoteMetrics.getLastUpdateTime();
    toJson(remote, _remoteMetrics.get());
  } else {
    remote["enabled"] = false;
  }
}

void Mycila::Router::toJson(const JsonObject& dest, const Metrics& metrics) {
  if (!std::isnan(metrics.apparentPower))
    dest["apparent_power"] = metrics.apparentPower;
  if (!std::isnan(metrics.current))
    dest["current"] = metrics.current;
  dest["energy"] = metrics.energy;
  if (!std::isnan(metrics.power))
    dest["power"] = metrics.power;
  if (!std::isnan(metrics.powerFactor))
    dest["power_factor"] = metrics.powerFactor;
  if (!std::isnan(metrics.resistance))
    dest["resistance"] = metrics.resistance;
  if (!std::isnan(metrics.thdi))
    dest["thdi"] = metrics.thdi;
  if (!std::isnan(metrics.voltage))
    dest["voltage"] = metrics.voltage;
}
#endif

// get router theoretical metrics based on the dimmer states and the grid voltage
void Mycila::Router::getRouterMetrics(Metrics& metrics, float voltage) const {
  metrics.voltage = voltage;

  for (const auto& output : _outputs) {
    RouterOutput::Metrics outputMetrics;
    output->getOutputMetrics(outputMetrics, voltage);
    metrics.energy += outputMetrics.energy;
    metrics.apparentPower += outputMetrics.apparentPower;
    metrics.current += outputMetrics.current;
    metrics.power += outputMetrics.power;
  }

  metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
  metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
  metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);

  if (_localMetrics.isPresent()) {
    metrics.energy = _localMetrics.get().energy;
  } else if (_remoteMetrics.isPresent()) {
    metrics.energy = _remoteMetrics.get().energy;
  }
}

void Mycila::Router::getRouterMeasurements(Metrics& metrics) const {
  bool routing = false;

  for (size_t i = 0; i < _outputs.size(); i++) {
    // check if we have a PZEM output
    RouterOutput::Metrics pzemMetrics;
    const bool pzem = _outputs[i]->getOutputMeasurements(pzemMetrics);

    // pzem output found: get voltage and energy
    if (pzem) {
      metrics.voltage = pzemMetrics.voltage;
      metrics.energy += pzemMetrics.energy;
    }

    bool r = _outputs[i]->getState() == RouterOutput::State::OUTPUT_ROUTING;
    routing |= r;

    // pzem found and routing => we have all the metrics
    if (pzem && r) {
      metrics.apparentPower += pzemMetrics.apparentPower;
      metrics.current += pzemMetrics.current;
      metrics.power += pzemMetrics.power;

      metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
      metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
      metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);
    }
  }

  // we found some pzem ? we are done
  if (metrics.voltage > 0)
    return;

  // no pzem found, let's check if we have a local JSY or remote JSY
  if (_localMetrics.isPresent()) {
    if (routing) {
      memcpy(&metrics, &_localMetrics.get(), sizeof(Metrics));
    } else {
      metrics.voltage = _localMetrics.get().voltage;
      metrics.energy = _localMetrics.get().energy;
    }
  } else if (_remoteMetrics.isPresent()) {
    if (routing) {
      memcpy(&metrics, &_remoteMetrics.get(), sizeof(Metrics));
    } else {
      metrics.voltage = _remoteMetrics.get().voltage;
      metrics.energy = _remoteMetrics.get().energy;
    }
  }
}

void Mycila::Router::beginCalibration(CalibrationCallback cb) {
  if (_calibrationRunning) {
    LOGW(TAG, "Calibration already running");
    return;
  }

  if (_outputs.empty()) {
    LOGW(TAG, "No output to calibrate");
    return;
  }

  LOGI(TAG, "Starting calibration");
  _calibrationStep = 1;
  _calibrationOutputIndex = 0;
  _calibrationCallback = cb;
  _calibrationRunning = true;
}

void Mycila::Router::continueCalibration() {
  if (!_calibrationRunning)
    return;

  switch (_calibrationStep) {
    case 1:
      for (const auto& output : _outputs) {
        output->config.autoBypass = false;
        output->config.autoDimmer = false;
        LOGI(TAG, "Disabling %s Auto Bypass", output->getName());
        output->applyAutoBypass();
        LOGI(TAG, "Turing off %s", output->getName());
        output->setDimmerOff();
        output->setBypassOff();
      }
      _calibrationOutputIndex = 0;
      _calibrationStep++;
      break;

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
        _outputs[_calibrationOutputIndex]->getOutputMeasurements(outputMetrics);
        float resistance = outputMetrics.resistance > 0 ? outputMetrics.resistance : 0; // handles nan
        if (!resistance) {
          Router::Metrics routerMetrics;
          getRouterMeasurements(routerMetrics);
          resistance = routerMetrics.resistance;
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
        _outputs[_calibrationOutputIndex]->getOutputMeasurements(outputMetrics);
        float resistance = outputMetrics.resistance > 0 ? outputMetrics.resistance : 0; // handles nan
        if (!resistance) {
          Router::Metrics routerMetrics;
          getRouterMeasurements(routerMetrics);
          resistance = routerMetrics.resistance;
        }

        _outputs[_calibrationOutputIndex]->config.calibratedResistance = (_outputs[_calibrationOutputIndex]->config.calibratedResistance + resistance) / 2;

        LOGI(TAG, "Turning off %s dimmer", _outputs[_calibrationOutputIndex]->getName());
        _outputs[_calibrationOutputIndex]->setDimmerOff();

        _calibrationOutputIndex++;
        if (_calibrationOutputIndex < _outputs.size()) {
          _calibrationStep = 2;
        } else {
          _calibrationRunning = false;
          LOGI(TAG, "Calibration done");
          if (_calibrationCallback)
            _calibrationCallback();
        }
      }
      break;

    default:
      break;
  }
}
