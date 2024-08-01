// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task zcdTask("ZCD", [](void* params) {
  const bool zcdSwitchedOn = config.getBool(KEY_ENABLE_ZCD);
  const Mycila::PulseAnalyzer::State analyzerState = pulseAnalyzer.getState();

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
      zcd.begin(config.get(KEY_PIN_ZCD).toInt(), config.get(KEY_GRID_FREQUENCY).toInt() == 60 ? 60 : 50);
      return;

    default:
      return;
  }
});
