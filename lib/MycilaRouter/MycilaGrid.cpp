// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <MycilaGrid.h>

bool Mycila::Grid::updatePower() {
  float update = NAN;

  // try in order of priority
  // - if MQTT is connected, it has priority
  // - if JSY remote is connected, it has second priority
  // - if JSY is connected, it has lowest priority
  if (_mqttPower.isPresent()) {
    update = _mqttPower.get();
  } else if (_remoteMetrics.isPresent()) {
    update = _remoteMetrics.get().power;
  } else if (_localMetrics.isPresent()) {
    update = _localMetrics.get().power;
  }

  if (isnan(update) && _power.neverUpdated()) {
    return false;
  }

  // all became unavailable ?
  if (isnan(update)) {
    _power.reset();
    return true;
  }

  // one became available ?
  if (_power.neverUpdated()) {
    _power.update(update);
    return true;
  }

  // check if update is significant
  if (update != _power.get()) {
    _power.update(update);
    return true;
  }

  return false;
}

// get the current grid voltage
// - if JSY are connected, they have priority
// - if JSY remote is connected, it has second priority
// - if PZEM is connected, it has third priority
// - if MQTT is connected, it has lowest priority
std::optional<float> Mycila::Grid::getVoltage() const {
  if (_localMetrics.isPresent() && _localMetrics.get().voltage > 0)
    return _localMetrics.get().voltage;
  if (_remoteMetrics.isPresent() && _remoteMetrics.get().voltage > 0)
    return _remoteMetrics.get().voltage;
  if (_pzemMetrics.isPresent() && _pzemMetrics.get().voltage > 0)
    return _pzemMetrics.get().voltage;
  if (_mqttVoltage.isPresent() && _mqttVoltage.get() > 0)
    return _mqttVoltage.get();
  return std::nullopt;
}

// get the grid frequency
// - if JSY are connected, they have priority
// - if JSY remote is connected, it has second priority
// - if PZEM is connected, it has third priority
std::optional<float> Mycila::Grid::getFrequency() const {
  if (_localMetrics.isPresent() && _localMetrics.get().frequency > 0)
    return _localMetrics.get().frequency;
  if (_remoteMetrics.isPresent() && _remoteMetrics.get().frequency > 0)
    return _remoteMetrics.get().frequency;
  if (_pzemMetrics.isPresent() && _pzemMetrics.get().frequency > 0)
    return _pzemMetrics.get().frequency;
  return std::nullopt;
}

// get the current grid measurements
// returns false if no measurements are available
bool Mycila::Grid::getGridMeasurements(Metrics& metrics) const {
  if (_mqttPower.isPresent()) {
    metrics.power = _mqttPower.get();
    metrics.voltage = getVoltage().value_or(NAN);
    metrics.frequency = getFrequency().value_or(NAN);
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

  if (_localMetrics.isPresent()) {
    metrics.apparentPower = _localMetrics.get().apparentPower;
    metrics.current = _localMetrics.get().current;
    metrics.energy = _localMetrics.get().energy;
    metrics.energyReturned = _localMetrics.get().energyReturned;
    metrics.frequency = _localMetrics.get().frequency;
    metrics.power = _localMetrics.get().power;
    metrics.powerFactor = _localMetrics.get().powerFactor;
    metrics.voltage = _localMetrics.get().voltage;
    return true;
  }

  return false;
}

#ifdef MYCILA_JSON_SUPPORT
void Mycila::Grid::toJson(const JsonObject& root) const {
  root["online"] = isConnected();

  if (_power.isPresent()) {
    root["power"] = _power.get();
  }

  std::optional<float> voltage = getVoltage();
  if (voltage.has_value()) {
    root["voltage"] = voltage.value();
  }

  Metrics measurements;
  getGridMeasurements(measurements);
  toJson(root["measurements"].to<JsonObject>(), measurements);

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

  JsonObject pzem = root["source"]["pzem"].to<JsonObject>();
  if (_pzemMetrics.isPresent()) {
    remote["enabled"] = true;
    remote["time"] = _pzemMetrics.getLastUpdateTime();
    toJson(pzem, _pzemMetrics.get());
  } else {
    remote["enabled"] = false;
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
  if (!isnanf(metrics.apparentPower))
    dest["apparent_power"] = metrics.apparentPower;
  if (!isnanf(metrics.current))
    dest["current"] = metrics.current;
  if (!isnanf(metrics.energy))
    dest["energy"] = metrics.energy;
  if (!isnanf(metrics.energyReturned))
    dest["energy_returned"] = metrics.energyReturned;
  if (!isnanf(metrics.frequency))
    dest["frequency"] = metrics.frequency;
  if (!isnanf(metrics.power))
    dest["power"] = metrics.power;
  if (!isnanf(metrics.powerFactor))
    dest["power_factor"] = metrics.powerFactor;
  if (!isnanf(metrics.voltage))
    dest["voltage"] = metrics.voltage;
}
#endif
