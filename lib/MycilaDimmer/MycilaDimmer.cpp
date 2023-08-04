// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaDimmer.h>
#include <MycilaLogger.h>

// #include "MycilaDimmerV1.h"
// #include "MycilaDimmerV2.h"
// #include "MycilaDimmerV3.h"

#ifndef MYCILA_DIMMER_NOT_LUT
#if MYCILA_DIMMER_MAX_LEVEL == 100
// #include "MycilaDimmerV1_100_LUT.h"
// #include "MycilaDimmerV2_100_LUT.h"
#include "MycilaDimmerV3_100_LUT.h"
#elif MYCILA_DIMMER_MAX_LEVEL == 255
// #include "MycilaDimmerV1_255_LUT.h"
// #include "MycilaDimmerV2_255_LUT.h"
#include "MycilaDimmerV3_255_LUT.h"
#else
#error "Invalid MYCILA_DIMMER_MAX_LEVEL"
#endif
#endif

#define TAG "DIMMER"

#define MICROS_BY_TWO_PI 159154.94309189533576888376 // 1000000 / TWO_PI
#define FOUR_PI          12.56637061435917295385
#define SQRT_2           1.4142135623730950488

static const char* DimmerTypeNames[] = {
  "Robodyn / TRIAC-based",
  "Random SSR",
  "Zero-Cross SSR"};

void Mycila::Dimmer::begin(const uint32_t pin, const DimmerType type) {
  if (_enabled)
    return;

  _type = type;

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    Logger.error(TAG, "Disable Dimmer: Invalid pin: %u", _pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  Logger.info(TAG, "Enable Dimmer %s on pin %u...", DimmerTypeNames[static_cast<int>(_type)], _pin);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);

  // TODO: Bust mode (cf circuitar)
  _dimmer = new Thyristor(_pin);
  _level = 0;
  _enabled = true;
}

void Mycila::Dimmer::end() {
  if (_enabled) {
    Logger.info(TAG, "Disable Dimmer %s on pin %u", DimmerTypeNames[static_cast<int>(_type)], _pin);
    _enabled = false;
    _level = 0;
    _dimmer->turnOff();
    delete _dimmer;
    _dimmer = nullptr;
    _pin = GPIO_NUM_NC;
    digitalWrite(_pin, LOW);
  }
}

void Mycila::Dimmer::toJson(const JsonObject& root) const {
  const float angle = getPhaseAngle();
  const float vrmsF = _lookupVrmsFactor(_level, Thyristor::getFrequency());
  root["angle_deg"] = static_cast<uint8_t>(angle * RAD_TO_DEG);
  root["angle_rad"] = angle;
  root["delay"] = _dimmer->getDelay();
  root["enabled"] = _enabled;
  root["level"] = _level;
  root["state"] = _level > 0 ? "on" : "off";
  root["vrms_factor"] = vrmsF;
  root["vrms_230V"] = vrmsF * 230;
}

void Mycila::Dimmer::setLevel(uint8_t newLevel) {
  if (!_enabled)
    return;

  if (newLevel > MYCILA_DIMMER_MAX_LEVEL)
    newLevel = MYCILA_DIMMER_MAX_LEVEL;

  if (_level == newLevel)
    return;

  const uint8_t oldLevel = _level;
  _level = newLevel;

  _dimmer->setDelay(_lookupFiringDelay(_level, Thyristor::getFrequency()));

  if (_callback && (oldLevel == 0 || oldLevel == MYCILA_DIMMER_MAX_LEVEL || newLevel == 0 || newLevel == MYCILA_DIMMER_MAX_LEVEL))
    _callback(newLevel == 0 ? DimmerLevel::OFF : (newLevel == MYCILA_DIMMER_MAX_LEVEL ? DimmerLevel::FULL : DimmerLevel::DIM));
}

float Mycila::Dimmer::getPhaseAngle() const { return _delayToPhaseAngle(_dimmer->getDelay(), Thyristor::getFrequency()); }

float Mycila::Dimmer::computeDimmedVoltage(float inputVrms) const {
  return _level == 0 ? 0 : (_level == MYCILA_DIMMER_MAX_LEVEL ? inputVrms : inputVrms * _lookupVrmsFactor(_level, Thyristor::getFrequency()));
}

void Mycila::Dimmer::generateLUT(Print* out) {
  out->print("static const uint16_t delay50HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= MYCILA_DIMMER_MAX_LEVEL; i++)
    out->printf("%u%s", _lookupFiringDelay(i, 50), i < MYCILA_DIMMER_MAX_LEVEL ? "," : "};\n");
  out->print("static const uint16_t delay60HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= MYCILA_DIMMER_MAX_LEVEL; i++)
    out->printf("%u%s", _lookupFiringDelay(i, 60), i < MYCILA_DIMMER_MAX_LEVEL ? "," : "};\n");
  out->print("static const float vrms50HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= MYCILA_DIMMER_MAX_LEVEL; i++)
    out->printf("%f%s", _lookupVrmsFactor(i, 50), i < MYCILA_DIMMER_MAX_LEVEL ? "," : "};\n");
  out->print("static const float vrms60HzLUT[] PROGMEM = {");
  for (size_t i = 0; i <= MYCILA_DIMMER_MAX_LEVEL; i++)
    out->printf("%f%s", _lookupVrmsFactor(i, 60), i < MYCILA_DIMMER_MAX_LEVEL ? "," : "};\n");
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
