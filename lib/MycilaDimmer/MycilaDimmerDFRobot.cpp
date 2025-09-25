// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaDimmerDFRobot.h>

// logging
#include <esp32-hal-log.h>

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

#define TAG "DFR_DIMMER"

void Mycila::DFRobotDimmer::begin() {
  if (_enabled)
    return;

  uint8_t resolution = getResolution();
  if (!resolution) {
    LOGE(TAG, "Disable DFRobot Dimmer: SKU not set!");
    return;
  }

  // sanity checks
  if (_sku == SKU::DFR1071_GP8211S) {
    if (_channel > 0) {
      LOGW(TAG, "DFRobot DFR1071 (GP8211S) has only one channel: switching to channel 0");
      _channel = 0;
    }
  }

  if (_channel > 2) {
    LOGE(TAG, "Disable DFRobot Dimmer: invalid channel %d", _channel);
    return;
  }

  // discovery
  bool found = false;
  if (_deviceAddress) {
    LOGI(TAG, "Searching for DFRobot Dimmer @ 0x%02x...", _deviceAddress);
    for (int i = 0; i < 3; i++) {
      uint8_t err = _test(_deviceAddress);
      if (err) {
        LOGW(TAG, "DFRobot Dimmer @ 0x%02x: TwoWire communication error: %d", _deviceAddress, err);
        delay(10);
      } else {
        found = true;
        break;
      }
    }

  } else {
    LOGI(TAG, "Searching for DFRobot Dimmer @ 0x58-0x5F (discovery)...");
    for (uint8_t addr = 0x58; !found && addr <= 0x5F; addr++) {
      for (int i = 0; i < 3; i++) {
        uint8_t err = _test(addr);
        if (err) {
          LOGW(TAG, "DFRobot Dimmer @ 0x%02x: TwoWire communication error: %d", addr, err);
          delay(10);
        } else {
          _deviceAddress = addr;
          found = true;
          break;
        }
      }
    }
  }

  if (found) {
    LOGI(TAG, "Enable DFRobot Dimmer @ 0x%02x and channel %d", _deviceAddress, _channel);
  } else if (_deviceAddress) {
    LOGW(TAG, "DFRobot Dimmer @ 0x%02x: Unable to communicate with device", _deviceAddress);
  } else {
    _deviceAddress = 0x58;
    LOGW(TAG, "DFRobot Dimmer: Discovery failed! Using default address 0x58");
  }

  // set output
  uint8_t err = _sendOutput(_deviceAddress, _output);
  if (err) {
    LOGE(TAG, "Disable DFRobot Dimmer: Unable to set output voltage: TwoWire communication error: %d", err);
    return;
  }

  _enabled = true;

  // restart with last saved value
  setDutyCycle(_dutyCycle);
}

void Mycila::DFRobotDimmer::end() {
  if (!_enabled)
    return;
  _enabled = false;
  LOGI(TAG, "Disable DFRobot Dimmer");
  // Note: do not set _dutyCycle to 0 in order to keep last set user value
  _delay = UINT16_MAX;
  apply();
}

bool Mycila::DFRobotDimmer::apply() {
  uint16_t duty = getFiringRatio() * ((1 << getResolution()) - 1);
  return _sendDutyCycle(_deviceAddress, duty) == ESP_OK;
}

uint8_t Mycila::DFRobotDimmer::_sendDutyCycle(uint8_t address, uint16_t duty) {
  duty = duty << (16 - getResolution());
  switch (_channel) {
    case 0: {
      uint8_t buffer[2] = {uint8_t(duty & 0xff), uint8_t(duty >> 8)};
      return _send(address, 0x02, buffer, 2) == 0;
    }
    case 1: {
      uint8_t buffer[2] = {uint8_t(duty & 0xff), uint8_t(duty >> 8)};
      return _send(address, 0x04, buffer, 2) == 0;
    }
    case 2: {
      uint8_t buffer[4] = {uint8_t(duty & 0xff), uint8_t(duty >> 8), uint8_t(duty & 0xff), uint8_t(duty >> 8)};
      return _send(address, 0x02, buffer, 4) == 0;
    }
    default:
      assert(false); // fail
      return ESP_FAIL;
  }
}

uint8_t Mycila::DFRobotDimmer::_sendOutput(uint8_t address, Output output) {
  switch (output) {
    case Output::RANGE_0_5V: {
      LOGI(TAG, "Set output range to 0-5V");
      uint8_t data = 0x00;
      return _send(address, 0x01, &data, 1);
    }
    case Output::RANGE_0_10V: {
      LOGI(TAG, "Set output range to 0-10V");
      uint8_t data = 0x11;
      return _send(address, 0x01, &data, 1);
    }
    default:
      assert(false); // fail
      return ESP_FAIL;
  }
}

uint8_t Mycila::DFRobotDimmer::_send(uint8_t address, uint8_t reg, uint8_t* buffer, size_t size) {
  _wire->beginTransmission(address);
  _wire->write(reg);
  for (uint16_t i = 0; i < size; i++) {
    _wire->write(buffer[i]);
  }
  return _wire->endTransmission();
}

uint8_t Mycila::DFRobotDimmer::_test(uint8_t address) {
  // return _sendDutyCycle(address, 0);
  _wire->beginTransmission(address);
  delayMicroseconds(100);
  return _wire->endTransmission();
}
