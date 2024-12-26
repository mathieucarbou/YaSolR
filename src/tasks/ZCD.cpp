// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <thyristor.h>

Mycila::Task zcdTask("ZCD", [](void* params) {
  // check if ZCD is online (connected to the grid)
  // this is required for dimmers to work
  if (!pulseAnalyzer.isOnline()) {
    logger.debug(TAG, "No electricity detected by ZCD module");
    zcdTask.resume(2 * Mycila::TaskDuration::SECONDS); // retry in 2 seconds
    return;
  }

  // => ZCD switch turned on + Pulse Analyzer online
  if (!Thyristor::getSemiPeriod() || !dimmerO1.isEnabled() || !dimmerO2.isEnabled()) {
    float frequency = detectGridFrequency();
    uint16_t semiPeriod = 1000000 / 2 / frequency;
    logger.info(TAG, "Detected grid frequency: %.2f Hz", frequency);

    if (!Thyristor::getSemiPeriod()) {
      logger.info(TAG, "Starting Thyristor with semi-period: %" PRIu16 " us", semiPeriod);
      Thyristor::setSemiPeriod(semiPeriod);
      Thyristor::begin();
    }

    if (!dimmerO1.isEnabled() && config.getBool(KEY_ENABLE_OUTPUT1_DIMMER)) {
      dimmerO1.begin(config.getLong(KEY_PIN_OUTPUT1_DIMMER), semiPeriod);
    }

    if (!dimmerO2.isEnabled() && config.getBool(KEY_ENABLE_OUTPUT2_DIMMER)) {
      dimmerO2.begin(config.getLong(KEY_PIN_OUTPUT2_DIMMER), semiPeriod);
    }
  }
});
