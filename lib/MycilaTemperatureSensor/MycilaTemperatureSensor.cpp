// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaLogger.h>
#include <MycilaTemperatureSensor.h>

#define TAG "DS18B20"

void Mycila::TemperatureSensor::begin(const uint8_t pin, uint32_t expirationDelay) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    Logger.error(TAG, "Disable Temperature Sensor: Invalid pin: %u", _pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  _oneWire.begin(_pin);
  _dallas.setOneWire(&_oneWire);
  _dallas.begin();
  if (_dallas.getDS18Count() == 0) {
    Logger.error(TAG, "No DS18B20 sensor found on pin: %u", _pin);
    return;
  } else {
    _dallas.setWaitForConversion(false);
    _dallas.requestTemperatures();
  }

  Logger.info(TAG, "Enable Temperature Sensor on pin %u...", _pin);
  Logger.debug(TAG, "- Expiration: %u seconds", expirationDelay);

  _expirationDelay = expirationDelay;
  _enabled = true;
}

void Mycila::TemperatureSensor::end() {
  if (_enabled) {
    Logger.info(TAG, "Disable Temperature Sensor in pin %u...", _pin);
    _enabled = false;
    _temperature = 0;
    _lastUpdate = 0;
    _pin = GPIO_NUM_NC;
  }
}

float Mycila::TemperatureSensor::read() {
  if (!_enabled)
    return 0;
  float read = _dallas.getTempCByIndex(0);
  _dallas.requestTemperatures();
  if (!isnan(read) && read > 0) {
    read = round(read * 100) / 100;
    _lastUpdate = millis();
    if (abs(read - _temperature) > 0.2 || !isValid()) {
      _temperature = read;
      if (_callback)
        _callback(_temperature);
    }
  }
  return _temperature;
}

String Mycila::TemperatureSensor::getTemperatureAsString() const {
  if (!_enabled || !isValid())
    return "??.??";
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%5.2f", _temperature);
  return buffer;
}

void Mycila::TemperatureSensor::toJson(const JsonObject& root) const {
  root["enabled"] = _enabled;
  root["temperature"] = _temperature;
  root["valid"] = isValid();
}
