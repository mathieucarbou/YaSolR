// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <MycilaDimmer.h>

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

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

#define TAG "DIMMER"

#define MYCILA_DIMMER_RESOLUTION 12   // bits
#define MYCILA_DIMMER_MAX_DUTY   4095 // ((1 << MYCILA_DIMMER_RESOLUTION) - 1)

#define TABLE_PHASE_LEN   (80)
#define TABLE_PHASE_SCALE ((TABLE_PHASE_LEN - 1) * (1UL << (16 - MYCILA_DIMMER_RESOLUTION)))

static const uint16_t TABLE_PHASE_DELAY[TABLE_PHASE_LEN] PROGMEM{0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094, 0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2, 0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9, 0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8, 0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf, 0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2, 0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606, 0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686, 0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296, 0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016};

void Mycila::Dimmer::begin(int8_t pin, uint32_t semiPeriod) {
  if (_enabled)
    return;

  if (!semiPeriod) {
    LOGE(TAG, "Disable Dimmer on pin %" PRId8 ": Invalid semi-period: %" PRIu32 " us", pin, semiPeriod);
    return;
  }

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    LOGE(TAG, "Disable Dimmer: Invalid pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  LOGI(TAG, "Enable Dimmer on pin %" PRId8 " with semi-period %" PRIu32 " us", pin, semiPeriod);

  _semiPeriod = semiPeriod;
  _dimmer = new Thyristor(_pin);
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _enabled = true;

  setDutyCycle(_dutyCycleMin);
}

void Mycila::Dimmer::end() {
  if (!_enabled)
    return;

  LOGI(TAG, "Disable Dimmer on pin %" PRId8, _pin);
  _enabled = false;
  _disable();
}

void Mycila::Dimmer::setDutyCycle(float newDutyCycle) {
  if (!_enabled)
    return;

  if (_semiPeriod == 0)
    return;

  // ensure newDuty is within bounds
  _dutyCycle = constrain(newDutyCycle, 0, _dutyCycleLimit);
  if (_dutyCycle == 0) {
    _dimmer->setDelay(_semiPeriod);

  } else if (_dutyCycle >= 1) {
    _dimmer->setDelay(0);

  } else {
    // duty remapping (equivalent to Shelly Dimmer remapping feature)
    const float remappedDutyCycle = _dutyCycleMin + _dutyCycle * (_dutyCycleMax - _dutyCycleMin);
    const uint16_t remappedDuty = remappedDutyCycle * MYCILA_DIMMER_MAX_DUTY;
    // ESP_LOGD(TAG, "Dimmer %d duty cycle set to %f (remapped to %f). Duty: %d", _pin, _dutyCycle, remappedDutyCycle, remappedDuty);

    // map new level to firing delay (LUT + linear interpolation)
    const uint32_t slot = remappedDuty * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 1);
    const uint16_t index = slot >> 16;
    const uint32_t a = TABLE_PHASE_DELAY[index];
    const uint32_t b = TABLE_PHASE_DELAY[index + 1];
    const uint32_t delay = a - (((a - b) * (slot & 0xffff)) >> 16);
    _delay = (delay * _semiPeriod) >> 16;

    _dimmer->setDelay(_delay);
  }
}

void Mycila::Dimmer::setDutyCycleLimit(float limit) {
  _dutyCycleLimit = constrain(limit, 0, 1);
  LOGD(TAG, "Set dimmer %" PRId8 " duty cycle limit to %f", _pin, _dutyCycleLimit);
  if (_dutyCycle > _dutyCycleLimit)
    setDutyCycle(_dutyCycleLimit);
}

void Mycila::Dimmer::setDutyCycleMin(float min) {
  _dutyCycleMin = constrain(min, 0, _dutyCycleMax);
  LOGD(TAG, "Set dimmer %" PRId8 " duty cycle min to %f", _pin, _dutyCycleMin);
  setDutyCycle(_dutyCycle);
}

void Mycila::Dimmer::setDutyCycleMax(float max) {
  _dutyCycleMax = constrain(max, _dutyCycleMin, 1);
  LOGD(TAG, "Set dimmer %" PRId8 " duty cycle max to %f", _pin, _dutyCycleMax);
  setDutyCycle(_dutyCycle);
}

void Mycila::Dimmer::_disable() {
  if (_enabled) {
    _dutyCycle = 0;
    _delay = UINT32_MAX;
    digitalWrite(_pin, LOW);
    delete _dimmer;
    _dimmer = nullptr;
    _pin = GPIO_NUM_NC;
  }
}
