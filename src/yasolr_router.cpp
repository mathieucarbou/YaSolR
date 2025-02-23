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
Mycila::PulseAnalyzer* pulseAnalyzer = nullptr;

// Internal dimmers
static Mycila::Dimmer* dimmer1 = nullptr;
static Mycila::Dimmer* dimmer2 = nullptr;

// tasks

static Mycila::Task calibrationTask("Calibration", [](void* params) { router.continueCalibration(); });

static Mycila::Task routerTask("Router", [](void* params) {
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

static Mycila::Task frequencyMonitorTask("Frequency", [](void* params) {
  float frequency = yasolr_frequency();
  const uint16_t semiPeriod = frequency ? 500000.0f / frequency : 0;

  if (semiPeriod) {
    if (!Thyristor::getSemiPeriod() || (dimmer1 && !dimmer1->getSemiPeriod()) || (dimmer2 && !dimmer2->getSemiPeriod())) {
      logger.info(TAG, "Detected grid frequency: %.2f Hz with semi-period: %" PRIu16 " us", frequency, semiPeriod);

      if (!Thyristor::getSemiPeriod()) {
        logger.info(TAG, "Starting Thyristor...");
        Thyristor::setSemiPeriod(semiPeriod);
        Thyristor::begin();
      }

      if (dimmer1 && !dimmer1->getSemiPeriod()) {
        dimmer1->setSemiPeriod(semiPeriod);
        dimmer1->off();
      }

      if (dimmer2 && !dimmer2->getSemiPeriod()) {
        dimmer2->setSemiPeriod(semiPeriod);
        dimmer2->off();
      }

      dashboardInitTask.resume();
    } else {
      Thyristor::setSemiPeriod(semiPeriod);
      if (dimmer1)
        dimmer1->setSemiPeriod(semiPeriod);
      if (dimmer2)
        dimmer2->setSemiPeriod(semiPeriod);
    }

  } else {
    logger.warn(TAG, "Unknown grid frequency!");

    if (Thyristor::getSemiPeriod() || (dimmer1 && dimmer1->getSemiPeriod()) || (dimmer2 && dimmer2->getSemiPeriod())) {
      if (Thyristor::getSemiPeriod()) {
        logger.info(TAG, "Stopping Thyristor...");
        Thyristor::setSemiPeriod(0);
        Thyristor::end();
      }

      if (dimmer1 && dimmer1->getSemiPeriod()) {
        logger.info(TAG, "Setting dimmer 1 semi-period to 0");
        dimmer1->setSemiPeriod(0);
      }

      if (dimmer2 && dimmer2->getSemiPeriod()) {
        logger.info(TAG, "Setting dimmer 2 semi-period to 0");
        dimmer2->setSemiPeriod(0);
      }

      dashboardInitTask.resume();
    }
  }
});

// functions

static bool isZeroCrossBased(const char* type) {
  return strcmp(type, YASOLR_DIMMER_LSA_PWM_ZCD) == 0 ||
         strcmp(type, YASOLR_DIMMER_ROBODYN) == 0 ||
         strcmp(type, YASOLR_DIMMER_RANDOM_SSR) == 0 ||
         strcmp(type, YASOLR_DIMMER_TRIAC) == 0;
}

static bool isPWMBased(const char* type) {
  return strcmp(type, YASOLR_DIMMER_LSA_PWM) == 0;
}

static bool isDACBased(const char* type) {
  return strcmp(type, YASOLR_DIMMER_LSA_GP8211S) == 0 ||
         strcmp(type, YASOLR_DIMMER_LSA_GP8403) == 0 ||
         strcmp(type, YASOLR_DIMMER_LSA_GP8413) == 0;
}

static Mycila::Dimmer* createDimmer(const char* keyEnable, const char* keyType, const char* keyPin) {
  if (config.getBool(keyEnable)) {
    const char* type = config.get(keyType);

    logger.info(TAG, "Initializing dimmer type: %s...", type);

    if (isZeroCrossBased(type)) {
      Mycila::ZeroCrossDimmer* zcDimmer = new Mycila::ZeroCrossDimmer();
      zcDimmer->setPin((gpio_num_t)config.getInt(keyPin));
      zcDimmer->begin();

      if (zcDimmer->isEnabled()) {
        return zcDimmer;

      } else {
        logger.error(TAG, "Dimmer failed to initialize!");
        delete zcDimmer;
        return nullptr;
      }
    }

    if (isPWMBased(type)) {
      Mycila::PWMDimmer* pwmDimmer = new Mycila::PWMDimmer();
      pwmDimmer->setPin((gpio_num_t)config.getInt(keyPin));
      pwmDimmer->begin();

      if (pwmDimmer->isEnabled()) {
        return pwmDimmer;

      } else {
        logger.error(TAG, "Dimmer failed to initialize!");
        delete pwmDimmer;
        return nullptr;
      }
    }

    if(isDACBased(type)) {
      // TODO: implement DAC based dimmer
      logger.warn(TAG, "DAC based dimmer not implemented yet!");
      return nullptr;
    }

    logger.error(TAG, "Dimmer type not supported!");
    return nullptr;
  }

  return nullptr;
}

static Mycila::Relay* createBypassRelay(const char* keyEnable, const char* keyType, const char* keyPin) {
  if (config.getBool(keyEnable)) {
    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(keyPin), config.isEqual(keyType, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      relay->listen([](bool state) {
        logger.info(TAG, "Output Relay changed to %s", state ? "ON" : "OFF");
        if (mqttPublishTask)
          mqttPublishTask->requestEarlyRun();
      });
      return relay;

    } else {
      logger.error(TAG, "Output 1 Bypass Relay failed to initialize!");
      delete relay;
      return nullptr;
    }
  }

  return nullptr;
}

static void initOutput1(uint16_t semiPeriod) {
  dimmer1 = createDimmer(KEY_ENABLE_OUTPUT1_DIMMER, KEY_OUTPUT1_DIMMER_TYPE, KEY_PIN_OUTPUT1_DIMMER);
  Mycila::Relay* bypassRelay = createBypassRelay(KEY_ENABLE_OUTPUT1_RELAY, KEY_OUTPUT1_RELAY_TYPE, KEY_PIN_OUTPUT1_RELAY);

  // output 1 is only a bypass relay ?
  if (!dimmer1 && bypassRelay) {
    logger.warn(TAG, "Output 1 has no dimmer and is only a bypass relay");
    // we do not call begin so that the virtual dimmer remains disabled
    dimmer1 = new Mycila::VirtualDimmer();
  }

  if (dimmer1) {
    output1 = new Mycila::RouterOutput("output1", *dimmer1, bypassRelay);

    dimmer1->setSemiPeriod(semiPeriod);
    dimmer1->setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100.0f);
    dimmer1->setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100.0f);
    dimmer1->setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100.0f);

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

static void initOutput2(uint16_t semiPeriod) {
  dimmer2 = createDimmer(KEY_ENABLE_OUTPUT2_DIMMER, KEY_OUTPUT2_DIMMER_TYPE, KEY_PIN_OUTPUT2_DIMMER);
  Mycila::Relay* bypassRelay = createBypassRelay(KEY_ENABLE_OUTPUT2_RELAY, KEY_OUTPUT2_RELAY_TYPE, KEY_PIN_OUTPUT2_RELAY);

  // output 2 is only a bypass relay ?
  if (!dimmer2 && bypassRelay) {
    logger.warn(TAG, "Output 2 has no dimmer and is only a bypass relay");
    // we do not call begin so that the virtual dimmer remains disabled
    dimmer2 = new Mycila::VirtualDimmer();
  }

  if (dimmer2) {
    output2 = new Mycila::RouterOutput("output2", *dimmer2, bypassRelay);

    dimmer2->setSemiPeriod(semiPeriod);
    dimmer2->setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100.0f);
    dimmer2->setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100.0f);
    dimmer2->setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100.0f);

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
    if (website.realTimePIDEnabled()) {
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
  const uint16_t semiPeriod = frequency ? 500000.0f / frequency : 0;

  initOutput1(semiPeriod);
  initOutput2(semiPeriod);

  // Routing Tasks

  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500);

  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1000);
  if (config.getBool(KEY_ENABLE_DEBUG))
    calibrationTask.enableProfiling();

  frequencyMonitorTask.setInterval(2000);

  coreTaskManager.addTask(routerTask);
  coreTaskManager.addTask(calibrationTask);
  coreTaskManager.addTask(frequencyMonitorTask);

  // Do we need a ZCD ?

  if (config.getBool(KEY_ENABLE_ZCD)) {
    pulseAnalyzer = new Mycila::PulseAnalyzer();
    pulseAnalyzer->onZeroCross(Mycila::ZeroCrossDimmer::onZeroCross);
    pulseAnalyzer->begin(config.getLong(KEY_PIN_ZCD));

    if (!pulseAnalyzer->isEnabled()) {
      logger.error(TAG, "ZCD Pulse Analyzer failed to initialize!");
      delete pulseAnalyzer;
      pulseAnalyzer = nullptr;
    }
  }
}
