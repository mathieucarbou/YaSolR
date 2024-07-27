// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
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
  getMeasurements(routerMeasurements);
  toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

  Metrics Metrics;
  getMetrics(Metrics, voltage);
  toJson(root["metrics"].to<JsonObject>(), Metrics);

  for (const auto& output : _outputs)
    output->toJson(root[output->getName()].to<JsonObject>(), voltage);

  _pidController->toJson(root["pid"].to<JsonObject>());
}

void Mycila::Router::toJson(const JsonObject& dest, const Metrics& metrics) {
  dest["apparent_power"] = metrics.apparentPower;
  dest["current"] = metrics.current;
  dest["energy"] = metrics.energy;
  dest["power"] = metrics.power;
  dest["power_factor"] = metrics.powerFactor;
  dest["resistance"] = metrics.resistance;
  dest["thdi"] = metrics.thdi;
  dest["voltage"] = metrics.voltage;
  dest["voltage_dimmed"] = metrics.dimmedVoltage;
}
#endif

// get router theoretical metrics based on the dimmer states and the grid voltage
void Mycila::Router::getMetrics(Metrics& metrics, float voltage) const {
  metrics.voltage = voltage;

  for (const auto& output : _outputs) {
    RouterOutput::Metrics outputMetrics;
    output->getDimmerMetrics(outputMetrics, voltage);
    metrics.energy += outputMetrics.energy;
    metrics.apparentPower += outputMetrics.apparentPower;
    metrics.current += outputMetrics.current;
    metrics.power += outputMetrics.power;
  }

  metrics.powerFactor = metrics.apparentPower == 0 ? 0 : metrics.power / metrics.apparentPower;
  metrics.dimmedVoltage = metrics.powerFactor * metrics.voltage;
  metrics.resistance = metrics.current == 0 ? 0 : metrics.power / (metrics.current * metrics.current);
  metrics.thdi = metrics.powerFactor == 0 ? 0 : sqrt(1 / pow(metrics.powerFactor, 2) - 1);

  if (!metrics.energy)
    metrics.energy = _jsy->getEnergy1() + _jsy->getEnergyReturned1();
}

void Mycila::Router::getMeasurements(Metrics& metrics) const {
  metrics.voltage = _jsy->getVoltage1();
  metrics.energy = _jsy->getEnergy1() + _jsy->getEnergyReturned1();
  for (const auto& output : _outputs) {
    if (output->getState() == RouterOutput::State::OUTPUT_ROUTING) {
      metrics.apparentPower = _jsy->getApparentPower1();
      metrics.current = _jsy->getCurrent1();
      metrics.dimmedVoltage = _jsy->getDimmedVoltage1();
      metrics.power = _jsy->getPower1();
      metrics.powerFactor = _jsy->getPowerFactor1();
      metrics.resistance = _jsy->getResistance1();
      metrics.thdi = _jsy->getTHDi1(0);
      break;
    }
  }
}

void Mycila::Router::beginCalibration(CalibrationCallback cb) {
  if (_calibrationRunning)
    return;

  LOGI(TAG, "Starting calibration");
  _calibrationStep = 1;
  _calibrationOutputIndex = 0;
  _calibrationCallback = cb;
  _calibrationRunning = true;
}

void Mycila::Router::calibrate() {
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
        _outputs[_calibrationOutputIndex]->getMeasurements(outputMetrics);
        float resistance = outputMetrics.resistance;
        if (resistance == 0) {
          Router::Metrics routerMetrics;
          getMeasurements(routerMetrics);
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
        _outputs[_calibrationOutputIndex]->getMeasurements(outputMetrics);
        float resistance = outputMetrics.resistance;
        if (resistance == 0) {
          Router::Metrics routerMetrics;
          getMeasurements(routerMetrics);
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
