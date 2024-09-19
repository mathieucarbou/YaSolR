// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

static uint32_t findBestSemiPeriod() {
  float frequency = 0;
  uint32_t semiPeriod = 0;

  // first get res from the measurement tools
  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);
  if (!frequency)
    frequency = round(gridMetrics.frequency);
  if (!frequency)
    frequency = round(pzemO1.getFrequency());
  if (!frequency)
    frequency = round(pzemO2.getFrequency());
  if (frequency) {
    semiPeriod = 1000000 / 2 / frequency;
    logger.info(TAG, "Semi-period detected by a measurement device: %" PRIu32 " us", semiPeriod);
    return semiPeriod;
  }

  // otherwise try get res from the configuration
  frequency = config.get(KEY_GRID_FREQUENCY).toInt();
  if (frequency) {
    semiPeriod = 1000000 / 2 / frequency;
    logger.info(TAG, "Semi-period derived from configured frequency: %" PRIu32 " us", semiPeriod);
    return semiPeriod;
  }

  // otherwise use auto-detection
  frequency = round(pulseAnalyzer.getFrequency());
  if (frequency) {
    semiPeriod = 1000000 / frequency;
    logger.info(TAG, "Semi-period detected by pulse analyzer: %" PRIu32 " us", semiPeriod);
    return semiPeriod;
  }

  return 0;
}

Mycila::Task zcdTask("ZCD", [](void* params) {
  // if the user is asking to disable ZCD, and it is starting or running, we stop it
  if (!config.getBool(KEY_ENABLE_ZCD)) {
    if (zcd.isEnabled() || pulseAnalyzer.isEnabled()) {
      logger.warn(TAG, "ZCD disabled, stopping ZCD and PulseAnalyzer");
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
  uint32_t semiPeriod = findBestSemiPeriod();

  // if we have a semiPeriod, we start ZCD
  if (semiPeriod) {
    pulseAnalyzer.end();
    zcd.begin(config.get(KEY_PIN_ZCD).toInt(), semiPeriod);
    if (zcd.isEnabled()) {
      logger.info(TAG, "ZCD started");
      dimmer1Task.resume();
      dimmer2Task.resume();
    }
    return;
  }

  // => ZCD switch turned + ZCD disabled + no semiPeriod => we need start analyser or wait for results

  if (!pulseAnalyzer.isEnabled()) {
    logger.info(TAG, "Starting PulseAnalyzer");
    pulseAnalyzer.begin(config.get(KEY_PIN_ZCD).toInt());
    return;
  }

  logger.warn(TAG, "Waiting for ZCD pulse analysis to complete...");
});
