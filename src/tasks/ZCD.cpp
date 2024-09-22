// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <thyristor.h>

Mycila::Task zcdTask("ZCD", [](void* params) {
  const bool zcdSwitched = config.getBool(KEY_ENABLE_ZCD);
  const bool dimmer1Switched = config.getBool(KEY_ENABLE_OUTPUT1_DIMMER);
  const bool dimmer2Switched = config.getBool(KEY_ENABLE_OUTPUT2_DIMMER);
  const bool online = pulseAnalyzer.isOnline();

  // Check dimmers

  if (!(dimmer1Switched && online) && dimmerO1.isEnabled()) {
    dimmerO1.end();
  }

  if (!(dimmer2Switched && online) && dimmerO2.isEnabled()) {
    dimmerO2.end();
  }

  // Check ZCD

  if (!zcdSwitched) {
    pulseAnalyzer.end();
    dimmerO1.end();
    dimmerO2.end();
    Thyristor::end();
    Thyristor::setSemiPeriod(0);
    return;
  }

  // => ZCD switch turned on

  // turn on Pulse Analyzer if off
  if (!pulseAnalyzer.isEnabled()) {
    logger.info(TAG, "Starting Pulse Analyzer");
    pulseAnalyzer.begin(config.get(KEY_PIN_ZCD).toInt());
    return;
  }

  // => ZCD switch turned on + Pulse Analyzer running

  // check if ZCD is online (connected to the grid)
  // this is required for dimmers to work
  if (!online) {
    logger.debug(TAG, "No electricity detected by ZCD module");
    return;
  }

  // => ZCD switch turned on + Pulse Analyzer online

  if (!Thyristor::getSemiPeriod() || (dimmer1Switched && !dimmerO1.isEnabled()) || (dimmer2Switched && !dimmerO2.isEnabled())) {
    float frequency = detectGridFrequency();
    uint32_t semiPeriod = 1000000 / 2 / frequency;

    if (!Thyristor::getSemiPeriod()) {
      logger.info(TAG, "Starting Thyristor with semi-period: %" PRIu32 " us", semiPeriod);
      Thyristor::setSemiPeriod(semiPeriod);
      Thyristor::begin();
    }

    if (dimmer1Switched && !dimmerO1.isEnabled()) {
      dimmerO1.begin(config.get(KEY_PIN_OUTPUT1_DIMMER).toInt(), semiPeriod);
    }

    if (dimmer2Switched && !dimmerO2.isEnabled()) {
      dimmerO2.begin(config.get(KEY_PIN_OUTPUT2_DIMMER).toInt(), semiPeriod);
    }
  }
});
