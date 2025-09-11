// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
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
  if (!grid.getVoltage().has_value() || !grid.getPower().has_value())
    router.noDivert();

  if (output1) {
    output1->applyBypassTimeout();
    output1->applyTemperatureLimit();
    output1->applyAutoBypass();
  }

  if (output2) {
    output2->applyBypassTimeout();
    output2->applyTemperatureLimit();
    output2->applyAutoBypass();
  }
});

static Mycila::Task* frequencyMonitorTask;

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

static Mycila::Dimmer* createDimmer(uint8_t outputID, const char* keyEnable, const char* keyType, const char* keyPin) {
  if (!config.getBool(keyEnable))
    return nullptr;

  Mycila::Dimmer* dimmer = nullptr;
  const char* type = config.get(keyType);

  LOGI(TAG, "Initializing dimmer %s for output %d", type, outputID);

  if (isZeroCrossBased(type)) {
    Mycila::ZeroCrossDimmer* zcDimmer = new Mycila::ZeroCrossDimmer();
    zcDimmer->setPin((gpio_num_t)config.getInt(keyPin));
    dimmer = zcDimmer;

  } else if (isPWMBased(type)) {
    Mycila::PWMDimmer* pwmDimmer = new Mycila::PWMDimmer();
    pwmDimmer->setPin((gpio_num_t)config.getInt(keyPin));
    dimmer = pwmDimmer;

  } else if (isDACBased(type)) {
    Wire.begin(config.getLong(KEY_PIN_I2C_SDA), config.getLong(KEY_PIN_I2C_SCL));
    Mycila::DFRobotDimmer* dfRobotDimmer = new Mycila::DFRobotDimmer();
    dfRobotDimmer->setWire(Wire);
    dfRobotDimmer->setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
    dfRobotDimmer->setChannel(outputID - 1);
    if (strcmp(type, YASOLR_DIMMER_LSA_GP8211S) == 0) {
      dfRobotDimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR1071_GP8211S);
    } else if (strcmp(type, YASOLR_DIMMER_LSA_GP8403) == 0) {
      dfRobotDimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR0971_GP8403);
    } else if (strcmp(type, YASOLR_DIMMER_LSA_GP8413) == 0) {
      dfRobotDimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR1073_GP8413);
    } else {
      delete dfRobotDimmer;
      dfRobotDimmer = nullptr;
    }
    dimmer = dfRobotDimmer;
  }

  if (!dimmer) {
    LOGE(TAG, "Dimmer type not supported: %s", type);
    return nullptr;
  }

  dimmer->begin();

  if (!dimmer->isEnabled()) {
    LOGE(TAG, "Dimmer failed to initialize!");
    delete dimmer;
    dimmer = nullptr;
  }

  return dimmer;
}

static Mycila::Relay* createBypassRelay(const char* keyEnable, const char* keyType, const char* keyPin) {
  if (config.getBool(keyEnable)) {
    Mycila::Relay* relay = new Mycila::Relay();
    relay->begin(config.getLong(keyPin), config.isEqual(keyType, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    if (relay->isEnabled()) {
      relay->listen([](bool state) {
        LOGI(TAG, "Output Relay changed to %s", state ? "ON" : "OFF");
        if (mqttPublishTask)
          mqttPublishTask->requestEarlyRun();
      });
      return relay;

    } else {
      LOGE(TAG, "Output 1 Bypass Relay failed to initialize!");
      delete relay;
      return nullptr;
    }
  }

  return nullptr;
}

static void initOutput1() {
  dimmer1 = createDimmer(1, KEY_ENABLE_OUTPUT1_DIMMER, KEY_OUTPUT1_DIMMER_TYPE, KEY_PIN_OUTPUT1_DIMMER);
  Mycila::Relay* bypassRelay = createBypassRelay(KEY_ENABLE_OUTPUT1_RELAY, KEY_OUTPUT1_RELAY_TYPE, KEY_PIN_OUTPUT1_RELAY);

  // output 1 is only a bypass relay ?
  if (!dimmer1 && bypassRelay) {
    LOGW(TAG, "Output 1 has no dimmer and is only a bypass relay");
    // we do not call begin so that the virtual dimmer remains disabled
    dimmer1 = new Mycila::VirtualDimmer();
  }

  if (dimmer1) {
    output1 = new Mycila::RouterOutput("output1", *dimmer1, bypassRelay);

    dimmer1->setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100.0f);
    dimmer1->setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100.0f);
    dimmer1->setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100.0f);

    output1->config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
    output1->config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
    output1->config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);
    output1->config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);
    output1->config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);
    output1->config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);
    output1->config.bypassTimeoutSec = config.getInt(KEY_OUTPUT1_BYPASS_TIMEOUT);
    output1->config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);
    output1->config.dimmerTempLimit = config.getInt(KEY_OUTPUT1_DIMMER_TEMP_LIMITER);
    output1->config.excessPowerLimiter = config.getInt(KEY_OUTPUT1_EXCESS_LIMITER);
    output1->config.weekDays = config.get(KEY_OUTPUT1_DAYS);
    output1->localMetrics().setExpiration(10000);                             // local is fast
    output1->temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

    router.addOutput(*output1);
  }
}

static void initOutput2() {
  dimmer2 = createDimmer(2, KEY_ENABLE_OUTPUT2_DIMMER, KEY_OUTPUT2_DIMMER_TYPE, KEY_PIN_OUTPUT2_DIMMER);
  Mycila::Relay* bypassRelay = createBypassRelay(KEY_ENABLE_OUTPUT2_RELAY, KEY_OUTPUT2_RELAY_TYPE, KEY_PIN_OUTPUT2_RELAY);

  // output 2 is only a bypass relay ?
  if (!dimmer2 && bypassRelay) {
    LOGW(TAG, "Output 2 has no dimmer and is only a bypass relay");
    // we do not call begin so that the virtual dimmer remains disabled
    dimmer2 = new Mycila::VirtualDimmer();
  }

  if (dimmer2) {
    output2 = new Mycila::RouterOutput("output2", *dimmer2, bypassRelay);

    dimmer2->setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100.0f);
    dimmer2->setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100.0f);
    dimmer2->setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100.0f);

    output2->config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
    output2->config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
    output2->config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);
    output2->config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);
    output2->config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);
    output2->config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);
    output2->config.bypassTimeoutSec = config.getInt(KEY_OUTPUT2_BYPASS_TIMEOUT);
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
  std::optional<float> voltage = grid.getVoltage();
  std::optional<float> power = grid.getPower();
  if (voltage.has_value() && power.has_value()) {
    router.divert(voltage.value(), power.value());
    if (website.realTimePIDEnabled()) {
      dashboardUpdateTask.requestEarlyRun();
    }
  }
}

void yasolr_init_router() {
  LOGI(TAG, "Initialize router outputs");

  // PID Controller

  pidController.setReverse(false);
  pidController.setProportionalMode(config.getLong(KEY_PID_P_MODE) == 1 ? Mycila::PID::ProportionalMode::P_ON_ERROR : Mycila::PID::ProportionalMode::P_ON_INPUT);
  pidController.setDerivativeMode(Mycila::PID::DerivativeMode::D_ON_ERROR);
  pidController.setIntegralCorrectionMode(config.getLong(KEY_PID_IC_MODE) == 1 ? Mycila::PID::IntegralCorrectionMode::IC_CLAMP : Mycila::PID::IntegralCorrectionMode::IC_ADVANCED);
  pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
  pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
  pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));

  // Router

  router.localMetrics().setExpiration(10000);  // local is fast
  router.remoteMetrics().setExpiration(10000); // remote JSY is fast

  // outputs

  initOutput1();
  initOutput2();

  // Do we have a user defined frequency?
  // Note: yasolr_frequency() at boot time will return either the user-defined frequency or NAN

  const float frequency = yasolr_frequency();
  const uint16_t semiPeriod = frequency > 0 ? 500000.0f / frequency : 0;

  if (semiPeriod) {
    LOGW(TAG, "Grid frequency forced by user to %.2f Hz with semi-period: %" PRIu16 " us", frequency, semiPeriod);

    if (dimmer1)
      dimmer1->setSemiPeriod(semiPeriod);
    if (dimmer2)
      dimmer2->setSemiPeriod(semiPeriod);

    // until we have our new dimmer impl... Only for ZC based dimmers...
    if ((dimmer1 && strcmp(dimmer1->type(), "zero-cross") == 0) || (dimmer2 && strcmp(dimmer2->type(), "zero-cross") == 0)) {
      LOGI(TAG, "Starting Thyristor");
      Thyristor::setSemiPeriod(semiPeriod);
      Thyristor::begin();
      if (dimmer1)
        dimmer1->off();
      if (dimmer2)
        dimmer2->off();
    }

  } else {
    LOGW(TAG, "Grid frequency will be auto-detected");

    frequencyMonitorTask = new Mycila::Task("Frequency", [](void* params) {
      const float frequency = yasolr_frequency();
      const uint16_t semiPeriod = frequency > 0 ? 500000.0f / frequency : 0;

      if (semiPeriod) {
        if (!Thyristor::getSemiPeriod() || (dimmer1 && !dimmer1->getSemiPeriod()) || (dimmer2 && !dimmer2->getSemiPeriod())) {
          LOGI(TAG, "Detected grid frequency: %.2f Hz with semi-period: %" PRIu16 " us", frequency, semiPeriod);

          if (!Thyristor::getSemiPeriod()) {
            LOGI(TAG, "Starting Thyristor");
            Thyristor::setSemiPeriod(semiPeriod);
            Thyristor::begin();
          }

          if (dimmer1 && !dimmer1->getSemiPeriod()) {
            LOGI(TAG, "Starting Output 1 Dimmer");
            dimmer1->setSemiPeriod(semiPeriod);
            dimmer1->off();
          }

          if (dimmer2 && !dimmer2->getSemiPeriod()) {
            LOGI(TAG, "Starting Output 2 Dimmer");
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
        LOGW(TAG, "Unknown grid frequency!");

        if (Thyristor::getSemiPeriod() || (dimmer1 && dimmer1->getSemiPeriod()) || (dimmer2 && dimmer2->getSemiPeriod())) {
          if (Thyristor::getSemiPeriod()) {
            LOGI(TAG, "Stopping Thyristor");
            Thyristor::setSemiPeriod(0);
            Thyristor::end();
          }

          if (dimmer1 && dimmer1->getSemiPeriod()) {
            LOGI(TAG, "Setting dimmer 1 semi-period to 0");
            dimmer1->setSemiPeriod(0);
          }

          if (dimmer2 && dimmer2->getSemiPeriod()) {
            LOGI(TAG, "Setting dimmer 2 semi-period to 0");
            dimmer2->setSemiPeriod(0);
          }

          dashboardInitTask.resume();
        }
      }
    });

    frequencyMonitorTask->setInterval(2000);
    coreTaskManager.addTask(*frequencyMonitorTask);
  }

  // Routing Tasks

  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500);

  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1000);
  if (config.getBool(KEY_ENABLE_DEBUG))
    calibrationTask.enableProfiling();

  coreTaskManager.addTask(routerTask);
  coreTaskManager.addTask(calibrationTask);

  // Do we need a ZCD ?

  if ((dimmer1 && strcmp(dimmer1->type(), "zero-cross") == 0) || (dimmer2 && strcmp(dimmer2->type(), "zero-cross") == 0)) {
    pulseAnalyzer = new Mycila::PulseAnalyzer();
    pulseAnalyzer->onZeroCross(Mycila::ZeroCrossDimmer::onZeroCross);
    pulseAnalyzer->begin(config.getLong(KEY_PIN_ZCD));

    if (!pulseAnalyzer->isEnabled()) {
      LOGE(TAG, "ZCD Pulse Analyzer failed to initialize!");
      delete pulseAnalyzer;
      pulseAnalyzer = nullptr;
    }
  }
}
