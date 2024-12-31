// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

#include <string>

Mycila::PID pidController;
Mycila::Router router(pidController);
Mycila::Dimmer dimmerO1;
Mycila::Dimmer dimmerO2;
Mycila::Relay bypassRelayO1;
Mycila::Relay bypassRelayO2;
Mycila::RouterOutput output1("output1", dimmerO1, bypassRelayO1);
Mycila::RouterOutput output2("output2", dimmerO2, bypassRelayO2);

Mycila::Task calibrationTask("Calibration", [](void* params) { router.calibrate(); });

Mycila::Task routerTask("Router", [](void* params) {
  std::optional<float> voltage = grid.getVoltage();

  if (!voltage.has_value() || grid.getPower().isAbsent())
    router.noDivert();

  output1.applyTemperatureLimit();
  output2.applyTemperatureLimit();

  output1.applyAutoBypass();
  output2.applyAutoBypass();
});

void yasolr_divert() {
  if (router.isCalibrationRunning())
    return;

  if (!output1.isAutoDimmerEnabled() && !output2.isAutoDimmerEnabled())
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
  logger.info(TAG, "Configuring %s", Mycila::AppInfo.nameModelVersion.c_str());

  // PID Controller
  pidController.setReverse(false);
  pidController.setProportionalMode((Mycila::PID::ProportionalMode)config.getLong(KEY_PID_P_MODE));
  pidController.setDerivativeMode((Mycila::PID::DerivativeMode)config.getLong(KEY_PID_D_MODE));
  pidController.setIntegralCorrectionMode((Mycila::PID::IntegralCorrectionMode)config.getLong(KEY_PID_IC_MODE));
  pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
  pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
  pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));

  //////////////
  // HARDWARE //
  //////////////

  router.localMetrics().setExpiration(10000);  // local is fast
  router.remoteMetrics().setExpiration(10000); // remote JSY is fast

  // output1
  output1.config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);
  output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
  output1.config.dimmerTempLimit = config.getInt(KEY_OUTPUT1_DIMMER_TEMP_LIMITER);
  output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);
  output1.config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);
  output1.config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);
  output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);
  output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);
  output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);
  output1.config.excessPowerLimiter = config.getInt(KEY_OUTPUT1_EXCESS_LIMITER);
  output1.temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt
  output1.localMetrics().setExpiration(10000);                             // local is fast

  // output2
  output2.config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);
  output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
  output2.config.dimmerTempLimit = config.getInt(KEY_OUTPUT2_DIMMER_TEMP_LIMITER);
  output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);
  output2.config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);
  output2.config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);
  output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);
  output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);
  output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);
  output2.config.excessPowerLimiter = config.getInt(KEY_OUTPUT2_EXCESS_LIMITER);
  output2.temperature().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // local or through mqtt
  output2.localMetrics().setExpiration(10000);                             // local is fast

  // Electricity: Relays
  if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY))
    bypassRelayO1.begin(config.getLong(KEY_PIN_OUTPUT1_RELAY), config.isEqual(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);
  if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY))
    bypassRelayO2.begin(config.getLong(KEY_PIN_OUTPUT2_RELAY), config.isEqual(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

  // Router
  router.addOutput(output1);
  router.addOutput(output2);

  bypassRelayO1.listen([](bool state) {
    logger.info(TAG, "Output 1 Relay changed to %s", state ? "ON" : "OFF");
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });
  bypassRelayO2.listen([](bool state) {
    logger.info(TAG, "Output 2 Relay changed to %s", state ? "ON" : "OFF");
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });

  // coreTaskManager
  calibrationTask.setEnabledWhen([]() { return router.isCalibrationRunning(); });
  calibrationTask.setInterval(1 * Mycila::TaskDuration::SECONDS);
  calibrationTask.setManager(coreTaskManager);

  routerTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  routerTask.setInterval(500 * Mycila::TaskDuration::MILLISECONDS);
  routerTask.setManager(coreTaskManager);

  if (config.getBool(KEY_ENABLE_DEBUG))
    calibrationTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
}
