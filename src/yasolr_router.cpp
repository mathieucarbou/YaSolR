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
Mycila::RouterOutput output1("output1");
Mycila::RouterOutput output2("output2");

// ZCD
Mycila::PulseAnalyzer* pulseAnalyzer = nullptr;

// Internal dimmers
static Mycila::Dimmer* dimmer1 = nullptr;
static Mycila::Dimmer* dimmer2 = nullptr;

// Internal bypass relays
static Mycila::Relay* bypassRelay1 = nullptr;
static Mycila::Relay* bypassRelay2 = nullptr;

// tasks

static Mycila::Task calibrationTask("Calibration", [](void* params) { router.continueCalibration(); });

static Mycila::Task routerTask("Router", [](void* params) {
  if (!grid.getVoltage().has_value() || !grid.getPower().has_value())
    router.noDivert();

  output1.applyBypassTimeout();
  output1.applyTemperatureLimit();
  output1.applyAutoBypass();

  output2.applyBypassTimeout();
  output2.applyTemperatureLimit();
  output2.applyAutoBypass();
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

static Mycila::Dimmer* createDimmer(uint8_t outputID, const char* keyType, const char* keyPin) {
  const char* type = config.get(keyType);
  LOGI(TAG, "Initializing dimmer %s for output %d", type, outputID);

  if (isZeroCrossBased(type)) {
    Mycila::ZeroCrossDimmer* zcDimmer = new Mycila::ZeroCrossDimmer();
    zcDimmer->setPin((gpio_num_t)config.getInt(keyPin));
    return zcDimmer;
  }

  if (isPWMBased(type)) {
    Mycila::PWMDimmer* pwmDimmer = new Mycila::PWMDimmer();
    pwmDimmer->setPin((gpio_num_t)config.getInt(keyPin));
    return pwmDimmer;
  }

  if (isDACBased(type)) {
    Wire.begin(config.getLong(KEY_PIN_I2C_SDA), config.getLong(KEY_PIN_I2C_SCL));
    Mycila::DFRobotDimmer* dfRobotDimmer = new Mycila::DFRobotDimmer();
    dfRobotDimmer->setWire(Wire);
    dfRobotDimmer->setOutput(Mycila::DFRobotDimmer::Output::RANGE_0_10V);
    dfRobotDimmer->setChannel(outputID - 1);
    if (strcmp(type, YASOLR_DIMMER_LSA_GP8211S) == 0) {
      dfRobotDimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR1071_GP8211S);
      return dfRobotDimmer;
    }
    if (strcmp(type, YASOLR_DIMMER_LSA_GP8403) == 0) {
      dfRobotDimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR0971_GP8403);
      return dfRobotDimmer;
    }
    if (strcmp(type, YASOLR_DIMMER_LSA_GP8413) == 0) {
      dfRobotDimmer->setSKU(Mycila::DFRobotDimmer::SKU::DFR1073_GP8413);
      return dfRobotDimmer;
    }
    delete dfRobotDimmer;
  }

  LOGE(TAG, "Dimmer type not supported: %s", type);
  return new Mycila::VirtualDimmer();
}

static Mycila::Relay* createBypassRelay(const char* keyType, const char* keyPin) {
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
    LOGE(TAG, "Bypass Relay failed to initialize!");
    delete relay;
    return nullptr;
  }
}

static void configure_zcd() {
  if ((dimmer1 && strcmp(dimmer1->type(), "zero-cross") == 0) || (dimmer2 && strcmp(dimmer2->type(), "zero-cross") == 0)) {
    if (pulseAnalyzer == nullptr) {
      LOGI(TAG, "Enable ZCD Pulse Analyzer");
      pulseAnalyzer = new Mycila::PulseAnalyzer();
      pulseAnalyzer->onZeroCross(Mycila::ZeroCrossDimmer::onZeroCross);
      pulseAnalyzer->begin(config.getLong(KEY_PIN_ZCD));

      if (!pulseAnalyzer->isEnabled()) {
        LOGE(TAG, "ZCD Pulse Analyzer failed to initialize!");
        delete pulseAnalyzer;
        pulseAnalyzer = nullptr;
      }
    }

  } else {
    if (pulseAnalyzer != nullptr) {
      LOGI(TAG, "Disable ZCD Pulse Analyzer");
      pulseAnalyzer->end();
      delete pulseAnalyzer;
      pulseAnalyzer = nullptr;
    }
  }
}

void yasolr_configure_output1_dimmer() {
  if (config.getBool(KEY_ENABLE_OUTPUT1_DIMMER)) {
    LOGI(TAG, "Enable Output 1 Dimmer");
    Mycila::Dimmer* old_dimmer1 = dimmer1;
    if (old_dimmer1)
      old_dimmer1->end();
    dimmer1 = createDimmer(1, KEY_OUTPUT1_DIMMER_TYPE, KEY_PIN_OUTPUT1_DIMMER);
    dimmer1->setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100.0f);
    dimmer1->setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100.0f);
    dimmer1->setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100.0f);
    dimmer1->begin();
    if (!dimmer1->isEnabled()) {
      LOGE(TAG, "Output 1 Dimmer failed to initialize!");
    } else {
      configure_zcd();
    }
    output1.setDimmer(dimmer1);
    delete old_dimmer1;
  } else {
    LOGI(TAG, "Disable Output 1 Dimmer");
    dimmer1->end();
  }
}

void yasolr_configure_output2_dimmer() {
  if (config.getBool(KEY_ENABLE_OUTPUT2_DIMMER)) {
    LOGI(TAG, "Enable Output 2 Dimmer");
    Mycila::Dimmer* old_dimmer2 = dimmer2;
    if (old_dimmer2)
      old_dimmer2->end();
    dimmer2 = createDimmer(2, KEY_OUTPUT2_DIMMER_TYPE, KEY_PIN_OUTPUT2_DIMMER);
    dimmer2->setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100.0f);
    dimmer2->setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100.0f);
    dimmer2->setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100.0f);
    dimmer2->begin();
    if (!dimmer2->isEnabled()) {
      LOGE(TAG, "Output 2 Dimmer failed to initialize!");
    } else {
      configure_zcd();
    }
    output2.setDimmer(dimmer2);
    delete old_dimmer2;
  } else {
    LOGI(TAG, "Disable Output 2 Dimmer");
    dimmer2->end();
  }
}

void yasolr_configure_output1_bypass_relay() {
  if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY)) {
    LOGI(TAG, "Enable Output 1 Bypass Relay");
    Mycila::Relay* old_relay1 = bypassRelay1;
    if (old_relay1)
      old_relay1->end();
    bypassRelay1 = createBypassRelay(KEY_OUTPUT1_RELAY_TYPE, KEY_PIN_OUTPUT1_RELAY);
    output1.setBypassRelay(bypassRelay1);
    delete old_relay1;
  } else {
    output1.setBypassRelay(nullptr);
    if (bypassRelay1) {
      LOGI(TAG, "Disable Output 1 Bypass Relay");
      bypassRelay1->end();
      delete bypassRelay1;
      bypassRelay1 = nullptr;
    }
  }
}

void yasolr_configure_output2_bypass_relay() {
  if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY)) {
    LOGI(TAG, "Enable Output 2 Bypass Relay");
    Mycila::Relay* old_relay2 = bypassRelay2;
    if (old_relay2)
      old_relay2->end();
    bypassRelay2 = createBypassRelay(KEY_OUTPUT2_RELAY_TYPE, KEY_PIN_OUTPUT2_RELAY);
    output2.setBypassRelay(bypassRelay2);
    delete old_relay2;
  } else {
    output2.setBypassRelay(nullptr);
    if (bypassRelay2) {
      LOGI(TAG, "Disable Output 2 Bypass Relay");
      bypassRelay2->end();
      delete bypassRelay2;
      bypassRelay2 = nullptr;
    }
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

void yasolr_configure_pid() {
  pidController.setReverse(false);
  pidController.setProportionalMode(config.getLong(KEY_PID_P_MODE) == 1 ? Mycila::PID::ProportionalMode::P_ON_ERROR : Mycila::PID::ProportionalMode::P_ON_INPUT);
  pidController.setIntegralCorrectionMode(config.getLong(KEY_PID_IC_MODE) == 1 ? Mycila::PID::IntegralCorrectionMode::IC_CLAMP : Mycila::PID::IntegralCorrectionMode::IC_ADVANCED);
  pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
  pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
  pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));
  LOGI(TAG, "PID Controller configured");
}

void yasolr_configure_frequency() {
  // Do we have a user defined frequency?
  // Note: yasolr_frequency() at boot time will return either the user-defined frequency or NAN
  const float frequency = config.getFloat(KEY_GRID_FREQUENCY);

  if (frequency > 0) {
    // cleanup previous task if any
    if (frequencyMonitorTask != nullptr) {
      coreTaskManager.removeTask(*frequencyMonitorTask);
      delete frequencyMonitorTask;
      frequencyMonitorTask = nullptr;
    }

    const uint16_t semiPeriod = frequency > 0 ? 500000.0f / frequency : 0;

    LOGI(TAG, "Grid frequency forced by user to %.2f Hz with semi-period: %" PRIu16 " us", frequency, semiPeriod);

    if (dimmer1)
      dimmer1->setSemiPeriod(semiPeriod);
    if (dimmer2)
      dimmer2->setSemiPeriod(semiPeriod);

    // until we have our new dimmer impl... Only for ZC based dimmers...
    if ((dimmer1 && strcmp(dimmer1->type(), "zero-cross") == 0) || (dimmer2 && strcmp(dimmer2->type(), "zero-cross") == 0)) {
      if (Thyristor::getSemiPeriod()) {
        // Thyristor already running, just update semi-period
        LOGI(TAG, "Updating Thyristor semi-period");
        Thyristor::setSemiPeriod(semiPeriod);
      } else {
        Thyristor::setSemiPeriod(semiPeriod);
        LOGI(TAG, "Starting Thyristor");
        Thyristor::begin();
        if (dimmer1)
          dimmer1->off();
        if (dimmer2)
          dimmer2->off();
      }
    }

  } else {
    // auto-detect frequency
    LOGI(TAG, "Grid frequency will be auto-detected");

    if (frequencyMonitorTask == nullptr) {
      frequencyMonitorTask = new Mycila::Task("Frequency", [](void* params) {
        const float frequency = yasolr_frequency();
        const uint16_t semiPeriod = frequency > 0 ? 500000.0f / frequency : 0;

        if (semiPeriod) {
          if (Thyristor::getSemiPeriod() != semiPeriod || (dimmer1 && dimmer1->getSemiPeriod() != semiPeriod) || (dimmer2 && dimmer2->getSemiPeriod() != semiPeriod)) {
            LOGI(TAG, "Detected grid frequency: %.2f Hz with semi-period: %" PRIu16 " us", frequency, semiPeriod);

            // until we have our new dimmer impl... Only for ZC based dimmers...
            if (Thyristor::getSemiPeriod()) {
              LOGI(TAG, "Updating Thyristor semi-period");
              Thyristor::setSemiPeriod(semiPeriod);
            } else {
              LOGI(TAG, "Starting Thyristor");
              Thyristor::setSemiPeriod(semiPeriod);
              Thyristor::begin();
            }

            if (dimmer1 && dimmer1->getSemiPeriod() != semiPeriod) {
              if (dimmer1->getSemiPeriod()) {
                LOGI(TAG, "Updating Output 1 Dimmer semi-period");
                dimmer1->setSemiPeriod(semiPeriod);
              } else {
                LOGI(TAG, "Starting Output 1 Dimmer");
                dimmer1->setSemiPeriod(semiPeriod);
                dimmer1->off();
              }
            }

            if (dimmer2 && dimmer2->getSemiPeriod() != semiPeriod) {
              if (dimmer2->getSemiPeriod()) {
                LOGI(TAG, "Updating Output 2 Dimmer semi-period");
                dimmer2->setSemiPeriod(semiPeriod);
              } else {
                LOGI(TAG, "Starting Output 2 Dimmer");
                dimmer2->setSemiPeriod(semiPeriod);
                dimmer2->off();
              }
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
  }
}

void yasolr_init_router() {
  LOGI(TAG, "Initialize router outputs");

  // Router

  router.localMetrics().setExpiration(10000);  // local is fast
  router.remoteMetrics().setExpiration(10000); // remote JSY is fast

  // outputs
  dimmer1 = new Mycila::VirtualDimmer();
  dimmer2 = new Mycila::VirtualDimmer();
  output1.setDimmer(dimmer1);
  output2.setDimmer(dimmer2);
  router.addOutput(output1);
  router.addOutput(output2);

  // configure output 1
  output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  output1.config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);
  output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);
  output1.config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);
  output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);
  output1.config.bypassTimeoutSec = config.getInt(KEY_OUTPUT1_BYPASS_TIMEOUT);
  output1.config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);
  output1.config.dimmerTempLimit = config.getInt(KEY_OUTPUT1_DIMMER_TEMP_LIMITER);
  output1.config.excessPowerLimiter = config.getInt(KEY_OUTPUT1_EXCESS_LIMITER);
  output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);
  output1.localMetrics().setExpiration(10000);                             // local is fast
  output1.temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

  // configure output 2
  output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  output2.config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);
  output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);
  output2.config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);
  output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);
  output2.config.bypassTimeoutSec = config.getInt(KEY_OUTPUT2_BYPASS_TIMEOUT);
  output2.config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);
  output2.config.dimmerTempLimit = config.getInt(KEY_OUTPUT2_DIMMER_TEMP_LIMITER);
  output2.config.excessPowerLimiter = config.getInt(KEY_OUTPUT2_EXCESS_LIMITER);
  output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);
  output2.localMetrics().setExpiration(10000);                             // local is fast
  output2.temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt

  // Routing Tasks

  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500);

  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1000);
  if (config.getBool(KEY_ENABLE_DEBUG))
    calibrationTask.enableProfiling();

  coreTaskManager.addTask(routerTask);
  coreTaskManager.addTask(calibrationTask);
}
