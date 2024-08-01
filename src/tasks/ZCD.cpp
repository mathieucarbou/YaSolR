// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

static uint16_t findBestSemiPeriod() {
  uint8_t frequency = 0;

  // first get frequency from the measurement tools

  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);

  if (!frequency)
    frequency = round(gridMetrics.frequency);
  if (!frequency)
    frequency = round(pzemO1.getFrequency());
  if (!frequency)
    frequency = round(pzemO2.getFrequency());

  // otherwise try get frequency from the configuration

  if (!frequency)
    frequency = config.get(KEY_GRID_FREQUENCY).toInt();

  // if a frequency is found, we return its semi-period

  if (frequency) {
    logger.debug(TAG, "Semi-period detected from a measurement device: %" PRIu16 " us", static_cast<uint16_t>(1000000 / 2 / frequency));
    return 1000000 / 2 / frequency;
  }

  // otherwise use auto-detection
  logger.debug(TAG, "Semi-period detected from pulse analyzer: %" PRIu16 " us", static_cast<uint16_t>(pulseAnalyzer.getPeriod()));
  return pulseAnalyzer.getPeriod();
}

Mycila::Task zcdTask("ZCD", [](void* params) {
  const bool zcdSwitchedOn = config.getBool(KEY_ENABLE_ZCD);
  const Mycila::PulseAnalyzer::State analyzerState = pulseAnalyzer.getState();
  uint16_t semiPeriod = 0;

  // if the user is asking to disable ZCD, and it is starting or running, we stop it
  if (!zcdSwitchedOn) {
    if (zcd.isEnabled() || analyzerState != Mycila::PulseAnalyzer::State::IDLE) {
      logger.warn(TAG, "ZCD disabled, stopping ZCD and PulseAnalyzer");
      zcd.end();
      pulseAnalyzer.end();
    }
    return;
  }

  switch (analyzerState) {
    //  ZCD switched on, and no analysis is in progress, we start a new one if ZCD is not enabled yet
    // Otherwise if ZCD switch is on and ZCd is enabled, system is running and we do nothing
    case Mycila::PulseAnalyzer::State::IDLE:
      if (!zcd.isEnabled()) {
        logger.info(TAG, "ZCD enabled, starting PulseAnalyzer");
        pulseAnalyzer.record(config.get(KEY_PIN_ZCD).toInt());
      }
      return;

    // if an analysis is in progress, we wait for it to finish
    case Mycila::PulseAnalyzer::State::RECORDING:
      logger.warn(TAG, "PulseAnalyzer is waiting for ZCD pulses...");
      return;

    case Mycila::PulseAnalyzer::State::RECORDED:
      logger.info(TAG, "PulseAnalyzer analysis finished, starting ZCD");
      pulseAnalyzer.analyze();
      pulseAnalyzer.end();
      semiPeriod = findBestSemiPeriod();
      if (!semiPeriod) {
        logger.error(TAG, "Cannot start ZCD and Dimmers: unable to determine semi-period");
        return;
      }
      zcd.begin(config.get(KEY_PIN_ZCD).toInt(), semiPeriod);
      if (zcd.isEnabled()) {
        logger.info(TAG, "ZCD started");
        dimmer1Task.resume();
        dimmer2Task.resume();
      }
      return;

    default:
      return;
  }
});
