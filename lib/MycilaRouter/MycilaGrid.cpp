// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaGrid.h>

std::optional<float> Mycila::Grid::getPower() const {
  if (_mqttPower.isPresent() && !std::isnan(_mqttPower.get()))
    return _mqttPower.get();
  if (_remoteMetrics.isPresent() && !std::isnan(_remoteMetrics.get().power))
    return _remoteMetrics.get().power;
  if (_jsyMetrics.isPresent() && !std::isnan(_jsyMetrics.get().power))
    return _jsyMetrics.get().power;
  return std::nullopt;
}

std::optional<float> Mycila::Grid::getVoltage() const {
  if (_jsyMetrics.isPresent() && _jsyMetrics.get().voltage > 0)
    return _jsyMetrics.get().voltage;
  if (_remoteMetrics.isPresent() && _remoteMetrics.get().voltage > 0)
    return _remoteMetrics.get().voltage;
  if (_pzemMetrics.isPresent() && _pzemMetrics.get().voltage > 0)
    return _pzemMetrics.get().voltage;
  if (_mqttVoltage.isPresent() && _mqttVoltage.get() > 0)
    return _mqttVoltage.get();
  return std::nullopt;
}

std::optional<float> Mycila::Grid::getFrequency() const {
  if (_jsyMetrics.isPresent() && _jsyMetrics.get().frequency > 0)
    return _jsyMetrics.get().frequency;
  if (_remoteMetrics.isPresent() && _remoteMetrics.get().frequency > 0)
    return _remoteMetrics.get().frequency;
  if (_pzemMetrics.isPresent() && _pzemMetrics.get().frequency > 0)
    return _pzemMetrics.get().frequency;
  return std::nullopt;
}

// get the current grid measurements
// returns false if no measurements are available
bool Mycila::Grid::readMeasurements(Metrics& metrics) const {
  if (_mqttPower.isPresent()) {
    metrics.power = _mqttPower.orElse(NAN);
    metrics.voltage = _mqttVoltage.orElse(NAN);
    return true;
  }

  if (_remoteMetrics.isPresent()) {
    metrics.apparentPower = _remoteMetrics.get().apparentPower;
    metrics.current = _remoteMetrics.get().current;
    metrics.energy = _remoteMetrics.get().energy;
    metrics.energyReturned = _remoteMetrics.get().energyReturned;
    metrics.frequency = _remoteMetrics.get().frequency;
    metrics.power = _remoteMetrics.get().power;
    metrics.powerFactor = _remoteMetrics.get().powerFactor;
    metrics.voltage = _remoteMetrics.get().voltage;
    return true;
  }

  if (_jsyMetrics.isPresent()) {
    metrics.apparentPower = _jsyMetrics.get().apparentPower;
    metrics.current = _jsyMetrics.get().current;
    metrics.energy = _jsyMetrics.get().energy;
    metrics.energyReturned = _jsyMetrics.get().energyReturned;
    metrics.frequency = _jsyMetrics.get().frequency;
    metrics.power = _jsyMetrics.get().power;
    metrics.powerFactor = _jsyMetrics.get().powerFactor;
    metrics.voltage = _jsyMetrics.get().voltage;
    return true;
  }

  return false;
}

#ifdef MYCILA_JSON_SUPPORT
void Mycila::Grid::toJson(const JsonObject& root) const {
  root["online"] = isConnected();

  std::optional<float> power = getPower();
  if (power.has_value()) {
    root["power"] = power.value();
  }

  std::optional<float> voltage = getVoltage();
  if (voltage.has_value()) {
    root["voltage"] = voltage.value();
  }

  std::optional<float> frequency = getFrequency();
  if (frequency.has_value()) {
    root["frequency"] = frequency.value();
  }

  Metrics* measurements = new Metrics();
  readMeasurements(*measurements);
  toJson(root["measurements"].to<JsonObject>(), *measurements);
  delete measurements;
  measurements = nullptr;

  JsonObject local = root["source"]["local"].to<JsonObject>();
  if (_jsyMetrics.isPresent()) {
    local["enabled"] = true;
    local["time"] = _jsyMetrics.getLastUpdateTime();
    toJson(local, _jsyMetrics.get());
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

  JsonObject pzem = root["source"]["pzem"].to<JsonObject>();
  if (_pzemMetrics.isPresent()) {
    pzem["enabled"] = true;
    pzem["time"] = _pzemMetrics.getLastUpdateTime();
    toJson(pzem, _pzemMetrics.get());
  } else {
    pzem["enabled"] = false;
  }

  JsonObject mqttPower = root["source"]["mqtt_power"].to<JsonObject>();
  if (_mqttPower.isPresent()) {
    mqttPower["enabled"] = true;
    mqttPower["time"] = _mqttPower.getLastUpdateTime();
    mqttPower["value"] = _mqttPower.get();
  } else {
    mqttPower["enabled"] = false;
  }

  JsonObject mqttVoltage = root["source"]["mqtt_voltage"].to<JsonObject>();
  if (_mqttVoltage.isPresent()) {
    mqttVoltage["enabled"] = true;
    mqttVoltage["time"] = _mqttVoltage.getLastUpdateTime();
    mqttVoltage["value"] = _mqttVoltage.get();
  } else {
    mqttVoltage["enabled"] = false;
  }
}

void Mycila::Grid::toJson(const JsonObject& dest, const Metrics& metrics) {
  if (!std::isnan(metrics.apparentPower))
    dest["apparent_power"] = metrics.apparentPower;
  if (!std::isnan(metrics.current))
    dest["current"] = metrics.current;
  dest["energy"] = metrics.energy;
  dest["energy_returned"] = metrics.energyReturned;
  if (!std::isnan(metrics.frequency))
    dest["frequency"] = metrics.frequency;
  if (!std::isnan(metrics.power))
    dest["power"] = metrics.power;
  if (!std::isnan(metrics.powerFactor))
    dest["power_factor"] = metrics.powerFactor;
  if (!std::isnan(metrics.voltage))
    dest["voltage"] = metrics.voltage;
}
#endif
