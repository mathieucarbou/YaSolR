// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaDimmer.h>
#include <MycilaLogger.h>

// #include "MycilaDimmerV1.h"
// #include "MycilaDimmerV1_100_LUT.h"
// #include "MycilaDimmerV1_255_LUT.h"
// #include "MycilaDimmerV2.h"
// #include "MycilaDimmerV2_100_LUT.h"
// #include "MycilaDimmerV2_255_LUT.h"
// #include "MycilaDimmerV3.h"
// #include "MycilaDimmerV3_100_LUT.h"
#include "MycilaDimmerV3_255_LUT.h"

#define TAG "DIMMER"

#define MICROS_BY_TWO_PI 159154.94309189533576888376 // 1000000 / TWO_PI
#define FOUR_PI          12.56637061435917295385
#define SQRT_2           1.4142135623730950488

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

#ifndef GPIO_IS_VALID_GPIO
  #define GPIO_IS_VALID_GPIO(gpio_num) ((gpio_num >= 0) && \
                                        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0))
#endif

extern Mycila::Logger logger;

void Mycila::Dimmer::begin(const int8_t pin, const uint8_t frequency) {
  if (_enabled)
    return;

  if (!frequency) {
    logger.error(TAG, "Disable Dimmer: Frequency not set");
    return;
  }
  _frequency = frequency;

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    logger.error(TAG, "Disable Dimmer: Invalid pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  logger.info(TAG, "Enable Dimmer on pin %" PRId8 "...", _pin);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);

  _dimmer = new Thyristor(_pin);
  _level = 0;
  _enabled = true;
}

void Mycila::Dimmer::end() {
  if (_enabled) {
    logger.info(TAG, "Disable Dimmer on pin %" PRId8, _pin);
    _enabled = false;
    _level = 0;
    _vrms = 0;
    _dimmer->turnOff();
    delete _dimmer;
    _dimmer = nullptr;
    _pin = GPIO_NUM_NC;
    digitalWrite(_pin, LOW);
  }
}

void Mycila::Dimmer::toJson(const JsonObject& root) const {
  const float angle = getPhaseAngle();
  root["angle_deg"] = static_cast<uint8_t>(angle * RAD_TO_DEG);
  root["angle_rad"] = angle;
  root["delay"] = _dimmer->getDelay();
  root["enabled"] = _enabled;
  root["level"] = _level;
  root["state"] = _level > 0 ? "on" : "off";
  root["vrms"] = _vrms;
  root["vrms_230V"] = _vrms * 230;
}

void Mycila::Dimmer::setLevel(uint16_t newLevel) {
  if (!_enabled)
    return;

  // ensure newLevel is within bounds
  if (newLevel > MYCILA_DIMMER_MAX_LEVEL)
    newLevel = MYCILA_DIMMER_MAX_LEVEL;

  // nothing to do ?
  if (_level == newLevel)
    return;

  // save old level
  const uint16_t oldLevel = _level;

  // map new level to firing delay
  const uint8_t lutLevel = map(newLevel, 0, MYCILA_DIMMER_MAX_LEVEL, 0, MYCILA_DIMMER_LUT_MAX_LEVEL);
  uint16_t vrms = _lookupVrms(lutLevel, _frequency);
  uint16_t delay = _lookupFiringDelay(lutLevel, _frequency);

  if (MYCILA_DIMMER_MAX_LEVEL != MYCILA_DIMMER_LUT_MAX_LEVEL && lutLevel < MYCILA_DIMMER_LUT_MAX_LEVEL) {
    const uint16_t prevLevel = map(lutLevel, 0, MYCILA_DIMMER_LUT_MAX_LEVEL, 0, MYCILA_DIMMER_MAX_LEVEL);
    if (newLevel != prevLevel) {
      const uint16_t nextLevel = map(lutLevel + 1, 0, MYCILA_DIMMER_LUT_MAX_LEVEL, 0, MYCILA_DIMMER_MAX_LEVEL);
      vrms = map(newLevel, prevLevel, nextLevel, vrms, _lookupVrms(lutLevel + 1, _frequency));
      delay = map(newLevel, prevLevel, nextLevel, delay, _lookupFiringDelay(lutLevel + 1, _frequency));
    }
  }

  // set new level
  _level = newLevel;
  _vrms = vrms;
  _dimmer->setDelay(delay);

  if (_callback && (oldLevel == 0 || oldLevel == MYCILA_DIMMER_MAX_LEVEL || newLevel == 0 || newLevel == MYCILA_DIMMER_MAX_LEVEL))
    _callback(newLevel == 0 ? DimmerLevel::OFF : (newLevel == MYCILA_DIMMER_MAX_LEVEL ? DimmerLevel::FULL : DimmerLevel::DIM));
}

void Mycila::Dimmer::generateLUT(Print& out, size_t lutSize) {
  out.print("static const uint16_t delay50HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= lutSize; i++)
    out.printf("%" PRIu16 "%s", _lookupFiringDelay(i, 50), i < lutSize ? "," : "};\n");
  out.print("static const uint16_t delay60HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= lutSize; i++)
    out.printf("%" PRIu16 "%s", _lookupFiringDelay(i, 60), i < lutSize ? "," : "};\n");
  out.print("static const float vrms50HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= lutSize; i++)
    out.printf("%f%s", _lookupVrms(i, 50), i < lutSize ? "," : "};\n");
  out.print("static const float vrms60HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= lutSize; i++)
    out.printf("%f%s", _lookupVrms(i, 60), i < lutSize ? "," : "};\n");
}

float Mycila::Dimmer::_delayToPhaseAngle(uint16_t delay, float frequency) {
  // https://www.quora.com/How-do-you-calculate-the-phase-angle-from-time-delay
  return delay * frequency / MICROS_BY_TWO_PI;
}

uint16_t Mycila::Dimmer::_phaseAngleToDelay(float angle, float frequency) {
  // https://www.quora.com/How-do-you-calculate-the-phase-angle-from-time-delay
  return angle * MICROS_BY_TWO_PI / frequency;
}

float Mycila::Dimmer::_vrmsFactor(float angle) {
  // https://www.physicsforums.com/threads/how-to-calculate-rms-voltage-from-triac-phase-angle.572668/
  return angle >= PI ? 0 : SQRT_2 * sqrt((TWO_PI - 2 * angle + sin(2 * angle)) / FOUR_PI);
}
