// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

#include <thyristor.h>

#include <string>

Mycila::PID pidController;
Mycila::Router router(pidController);
Mycila::RouterOutput* output1 = nullptr;
Mycila::RouterOutput* output2 = nullptr;

// ZCD
Mycila::TriacDimmer* triacDimmer1 = nullptr;
Mycila::TriacDimmer* triacDimmer2 = nullptr;
Mycila::PulseAnalyzer* pulseAnalyzer = nullptr;
Mycila::Task* zcdTask = nullptr;

// tasks

Mycila::Task calibrationTask("Calibration", [](void* params) { router.continueCalibration(); });

Mycila::Task routerTask("Router", [](void* params) {
  std::optional<float> voltage = grid.getVoltage();

  if (!voltage.has_value() || grid.getPower().isAbsent())
    router.noDivert();

  if (output1) {
    output1->applyTemperatureLimit();
    output1->applyAutoBypass();
  }

  if (output2) {
    output2->applyTemperatureLimit();
    output2->applyAutoBypass();
  }
});

// functions

bool isTriacBased(const char* type) {
  return strcmp(type, YASOLR_DIMMER_LSA_PWM) == 0 || strcmp(type, YASOLR_DIMMER_ROBODYN) == 0 || strcmp(type, YASOLR_DIMMER_SSR_RANDOM) == 0 || strcmp(type, YASOLR_DIMMER_TRIAC) == 0;
}

Mycila::Dimmer* createDimmer1(uint16_t semiPeriod) {
  if (config.getBool(KEY_ENABLE_OUTPUT1_DIMMER)) {
    const char* type = config.get(KEY_OUTPUT1_DIMMER_TYPE);
    Mycila::Dimmer* dimmer = nullptr;

    if (isTriacBased(type)) {
      logger.info(TAG, "Initializing Output 1 dimmer type: %s", type);

      triacDimmer1 = new Mycila::TriacDimmer();
      triacDimmer1->setPin((gpio_num_t)config.getInt(KEY_PIN_OUTPUT1_DIMMER));
      triacDimmer1->setSemiPeriod(semiPeriod);
      triacDimmer1->begin();

      if (triacDimmer1->isEnabled()) {
        triacDimmer1->setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100);
        triacDimmer1->setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100);
        triacDimmer1->setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100);
        dimmer = triacDimmer1;

      } else {
        logger.error(TAG, "Output 1 Dimmer failed to initialize!");
        delete triacDimmer1;
        triacDimmer1 = nullptr;
      }

    } else {
      logger.error(TAG, "Output 1 Dimmer type not supported: %s", type);
    }

    return dimmer;
  }

  return nullptr;
}

Mycila::Dimmer* createDimmer2(uint16_t semiPeriod) {
  if (config.getBool(KEY_ENABLE_OUTPUT2_DIMMER)) {
    const char* type = config.get(KEY_OUTPUT2_DIMMER_TYPE);
    Mycila::Dimmer* dimmer = nullptr;

    if (isTriacBased(type)) {
      logger.info(TAG, "Initializing Output 2 dimmer type: %s", type);

      triacDimmer2 = new Mycila::TriacDimmer();
      triacDimmer2->setPin((gpio_num_t)config.getInt(KEY_PIN_OUTPUT2_DIMMER));
      triacDimmer2->setSemiPeriod(semiPeriod);
      triacDimmer2->begin();

      if (triacDimmer2->isEnabled()) {
        triacDimmer2->setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100);
        triacDimmer2->setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100);
        triacDimmer2->setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100);
        dimmer = triacDimmer2;

      } else {
        logger.error(TAG, "Output 2 Dimmer failed to initialize!");
        delete triacDimmer2;
        triacDimmer2 = nullptr;
      }

    } else {
      logger.error(TAG, "Output 2 Dimmer type not supported: %s", type);
    }

    return dimmer;
  }

  return nullptr;
}

Mycila::Relay* createBypassRelay1() {
  if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY)) {
    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(KEY_PIN_OUTPUT1_RELAY), config.isEqual(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      relay->listen([](bool state) {
        logger.info(TAG, "Output 1 Relay changed to %s", state ? "ON" : "OFF");
        if (mqttPublishTask)
          mqttPublishTask->requestEarlyRun();
      });

    } else {
      logger.error(TAG, "Output 1 Bypass Relay failed to initialize!");
      delete relay;
      relay = nullptr;
    }

    return relay;
  }

  return nullptr;
}

Mycila::Relay* createBypassRelay2() {
  if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY)) {
    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(KEY_PIN_OUTPUT2_RELAY), config.isEqual(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      relay->listen([](bool state) {
        logger.info(TAG, "Output 2 Relay changed to %s", state ? "ON" : "OFF");
        if (mqttPublishTask)
          mqttPublishTask->requestEarlyRun();
      });

    } else {
      logger.error(TAG, "Output 2 Bypass Relay failed to initialize!");
      delete relay;
      relay = nullptr;
    }

    return relay;
  }

  return nullptr;
}

void initOutput1(uint16_t semiPeriod) {
  Mycila::Dimmer* dimmer1 = createDimmer1(semiPeriod);
  Mycila::Relay* bypassRelay1 = createBypassRelay1();

  // output 1 is only a bypass relay ?
  if (!dimmer1 && bypassRelay1) {
    logger.warn(TAG, "Output 1 has no dimmer and is only a bypass relay");
    // we do not call begin so that the virtual dimmer remains disabled
    dimmer1 = new Mycila::VirtualDimmer();
  }

  if (dimmer1) {
    output1 = new Mycila::RouterOutput("output1", *dimmer1, bypassRelay1);

    output1->config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
    output1->config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
    output1->config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);
    output1->config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);
    output1->config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);
    output1->config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);
    output1->config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);
    output1->config.dimmerTempLimit = config.getInt(KEY_OUTPUT1_DIMMER_TEMP_LIMITER);
    output1->config.excessPowerLimiter = config.getInt(KEY_OUTPUT1_EXCESS_LIMITER);
    output1->config.weekDays = config.get(KEY_OUTPUT1_DAYS);
    output1->localMetrics().setExpiration(10000);                             // local is fast
    output1->temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

    router.addOutput(*output1);
  }
}

void initOutput2(uint16_t semiPeriod) {
  Mycila::Dimmer* dimmer2 = createDimmer2(semiPeriod);
  Mycila::Relay* bypassRelay2 = createBypassRelay2();

  // output 2 is only a bypass relay ?
  if (!dimmer2 && bypassRelay2) {
    logger.warn(TAG, "Output 2 has no dimmer and is only a bypass relay");
    // we do not call begin so that the virtual dimmer remains disabled
    dimmer2 = new Mycila::VirtualDimmer();
  }

  if (dimmer2) {
    output2 = new Mycila::RouterOutput("output2", *dimmer2, bypassRelay2);

    output2->config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
    output2->config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
    output2->config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);
    output2->config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);
    output2->config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);
    output2->config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);
    output2->config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);
    output2->config.dimmerTempLimit = config.getInt(KEY_OUTPUT2_DIMMER_TEMP_LIMITER);
    output2->config.excessPowerLimiter = config.getInt(KEY_OUTPUT2_EXCESS_LIMITER);
    output2->config.weekDays = config.get(KEY_OUTPUT2_DAYS);
    output2->localMetrics().setExpiration(10000);                             // local is fast
    output2->temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

    router.addOutput(*output2);
  }
}

void yasolr_divert() {
  if (router.isCalibrationRunning())
    return;

  if (!router.isAutoDimmerEnabled())
    return;

  std::optional<float> voltage = grid.getVoltage();

  if (voltage.has_value() && grid.getPower().isPresent()) {
    router.divert(voltage.value(), grid.getPower().get());
    if (website.pidCharts()) {
      dashboardUpdateTask.requestEarlyRun();
    }
  }
}

void yasolr_init_router() {
  logger.info(TAG, "Initialize router outputs...");

  // PID Controller

  pidController.setReverse(false);
  pidController.setProportionalMode((Mycila::PID::ProportionalMode)config.getLong(KEY_PID_P_MODE));
  pidController.setDerivativeMode((Mycila::PID::DerivativeMode)config.getLong(KEY_PID_D_MODE));
  pidController.setIntegralCorrectionMode((Mycila::PID::IntegralCorrectionMode)config.getLong(KEY_PID_IC_MODE));
  pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
  pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
  pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));

  // Router

  router.localMetrics().setExpiration(10000);  // local is fast
  router.remoteMetrics().setExpiration(10000); // remote JSY is fast

  // Do we have a user defined frequency?

  const float frequency = config.getFloat(KEY_GRID_FREQUENCY);
  const uint16_t semiPeriod = frequency ? 1000000 / 2 / frequency : 0;

  initOutput1(semiPeriod);
  initOutput2(semiPeriod);

  // Do we need a ZCD ?

  if (config.getBool(KEY_ENABLE_ZCD)) {
    pulseAnalyzer = new Mycila::PulseAnalyzer();
    pulseAnalyzer->onZeroCross(Mycila::TriacDimmer::onZeroCross);
    pulseAnalyzer->begin(config.getLong(KEY_PIN_ZCD));

    if (!pulseAnalyzer->isEnabled()) {
      logger.error(TAG, "ZCD Pulse Analyzer failed to initialize!");
      delete pulseAnalyzer;
      pulseAnalyzer = nullptr;
    }
  }

  // Tasks

  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500 * Mycila::TaskDuration::MILLISECONDS);
  routerTask.setManager(coreTaskManager);

  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1 * Mycila::TaskDuration::SECONDS);
  calibrationTask.setManager(coreTaskManager);
  if (config.getBool(KEY_ENABLE_DEBUG))
    calibrationTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);

  // ZCD Task that will keep the grid semi-period in sync with the dimmers
  if (pulseAnalyzer || triacDimmer1 || triacDimmer2) {
    logger.info(TAG, "Initialize ZCD sync...");

    zcdTask = new Mycila::Task("ZCD", [](void* params) {
      // check if ZCD is online (connected to the grid)
      // this is required for dimmers to work
      if (pulseAnalyzer->isOnline()) {
        if (!Thyristor::getSemiPeriod() || (triacDimmer1 && !triacDimmer1->getSemiPeriod()) || (triacDimmer2 && !triacDimmer2->getSemiPeriod())) {
          const float frequency = yasolr_frequency();
          const uint16_t semiPeriod = frequency ? 1000000 / 2 / frequency : 0;

          if (semiPeriod) {
            logger.info(TAG, "Detected grid frequency: %.2f Hz with semi-period: %" PRIu16 " us", frequency, semiPeriod);

            if (!Thyristor::getSemiPeriod()) {
              logger.info(TAG, "Starting Thyristor...");
              Thyristor::setSemiPeriod(semiPeriod);
              Thyristor::begin();
            }

            if (triacDimmer1 && !triacDimmer1->getSemiPeriod()) {
              triacDimmer1->setSemiPeriod(semiPeriod);
              triacDimmer1->off();
            }

            if (triacDimmer2 && !triacDimmer2->getSemiPeriod()) {
              triacDimmer2->setSemiPeriod(semiPeriod);
              triacDimmer2->off();
            }

            dashboardInitTask.resume();
          }
        }

      } else {
        if (Thyristor::getSemiPeriod() || (triacDimmer1 && triacDimmer1->getSemiPeriod()) || (triacDimmer2 && triacDimmer2->getSemiPeriod())) {
          logger.warn(TAG, "No electricity detected by ZCD module");

          if (Thyristor::getSemiPeriod()) {
            logger.info(TAG, "Stopping Thyristor...");
            Thyristor::setSemiPeriod(0);
            Thyristor::end();
          }

          if (triacDimmer1 && triacDimmer1->getSemiPeriod()) {
            logger.info(TAG, "Setting dimmer 1 semi-period to 0");
            triacDimmer1->setSemiPeriod(0);
          }

          if (triacDimmer2 && triacDimmer2->getSemiPeriod()) {
            logger.info(TAG, "Setting dimmer 2 semi-period to 0");
            triacDimmer2->setSemiPeriod(0);
          }

          dashboardInitTask.resume();
        }
      }
    });

    zcdTask->setInterval(2 * Mycila::TaskDuration::SECONDS);
    zcdTask->setManager(coreTaskManager);
    if (config.getBool(KEY_ENABLE_DEBUG))
      zcdTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  }
}
