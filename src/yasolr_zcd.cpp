// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

#include <thyristor.h>

Mycila::PulseAnalyzer* pulseAnalyzer;
Mycila::Task* zcdTask;

void yasolr_init_zcd() {
  if (config.getBool(KEY_ENABLE_ZCD)) {
    assert(!pulseAnalyzer);
    assert(!zcdTask);

    logger.info(TAG, "Initialize ZCD pulse analyzer...");

    pulseAnalyzer = new Mycila::PulseAnalyzer();
    pulseAnalyzer->onZeroCross(Mycila::Dimmer::onZeroCross);
    pulseAnalyzer->begin(config.getLong(KEY_PIN_ZCD));

    if (!pulseAnalyzer->isEnabled())
      return;

    zcdTask = new Mycila::Task("ZCD", [](void* params) {
      // check if ZCD is online (connected to the grid)
      // this is required for dimmers to work
      if (!pulseAnalyzer->isOnline()) {
        logger.warn(TAG, "No electricity detected by ZCD module");
        return;
      }

      // => ZCD switch turned on + Pulse Analyzer online
      if (!Thyristor::getSemiPeriod() || (output1 && !output1->isDimmerEnabled()) || (output2 && !output2->isDimmerEnabled())) {
        float frequency = yasolr_frequency();

        if (!frequency) {
          logger.warn(TAG, "No electricity detected by ZCD module");
          return;
        }

        const uint16_t semiPeriod = 1000000 / 2 / frequency;
        logger.info(TAG, "Detected grid frequency: %.2f Hz", frequency);
        zcdTask->setEnabled(false);

        if (!Thyristor::getSemiPeriod()) {
          logger.info(TAG, "Starting Thyristor with semi-period: %" PRIu16 " us", semiPeriod);
          Thyristor::setSemiPeriod(semiPeriod);
          Thyristor::begin();
        }

        if (output1 && !output1->isDimmerEnabled() && config.getBool(KEY_ENABLE_OUTPUT1_DIMMER)) {
          output1->beginDimmer(config.getLong(KEY_PIN_OUTPUT1_DIMMER), semiPeriod);
          if (output1->isDimmerEnabled()) {
            output1->setDimmerDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100);
            output1->setDimmerDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100);
            output1->setDimmerDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100);
          }
        }

        if (output2 && !output2->isDimmerEnabled() && config.getBool(KEY_ENABLE_OUTPUT2_DIMMER)) {
          output2->beginDimmer(config.getLong(KEY_PIN_OUTPUT2_DIMMER), semiPeriod);
          if (output2->isDimmerEnabled()) {
            output2->setDimmerDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100);
            output2->setDimmerDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100);
            output2->setDimmerDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100);
          }
        }

        // refresh dashboard when electricity is back
        dashboardInitTask.resume();

      } else {
        zcdTask->setEnabled(false);
      }
    });

    zcdTask->setInterval(2 * Mycila::TaskDuration::SECONDS);
    zcdTask->setManager(coreTaskManager);
  }
}
