// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task zcdTask("ZCD", [](void* params) {
  // if the user is asking to disable ZCD, and it is starting or running, we stop it
  if (!config.getBool(KEY_ENABLE_ZCD)) {
    if (zcd.isEnabled() || pulseAnalyzer.isEnabled()) {
      logger.warn(TAG, "ZCD disabled, stopping ZCD and Pulse Analyzer");
      zcd.end();
      pulseAnalyzer.end();
    }
    return;
  }

  // ZCD switch turned on

  // if ZCD is already enabled, we do nothing
  if (zcd.isEnabled())
    return;

  // => ZCD disabled.

  // try to find the semiPeriod
  const float frequency = detectGridFrequency();

  if (frequency) {
    const uint32_t semiPeriod = 1000000 / 2 / frequency;
    logger.info(TAG, "Semi-period: %" PRIu32 " us, Grid Frequency: %.2f Hz", semiPeriod, frequency);

    // if we have a semiPeriod, we start ZCD
    pulseAnalyzer.end();
    zcd.begin(config.get(KEY_PIN_ZCD).toInt(), semiPeriod);
    if (zcd.isEnabled()) {
      logger.info(TAG, "ZCD started");
      dimmer1Task.resume();
      dimmer2Task.resume();
    }

  } else if (!pulseAnalyzer.isEnabled()) {
    // => ZCD switch turned + ZCD disabled + no semiPeriod + analyzer off => we need start analyser
    logger.info(TAG, "Starting PulseAnalyzer");
    pulseAnalyzer.begin(config.get(KEY_PIN_ZCD).toInt());

  } else {
    // => ZCD switch turned + ZCD disabled + no semiPeriod + analyzer running => we need to wait
    logger.warn(TAG, "Waiting for ZCD pulse analysis to complete...");
  }
});
