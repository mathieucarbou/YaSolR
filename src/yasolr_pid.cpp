// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::PID pidController;

Mycila::Task pidTask("PID", []() {
  if (router.isCalibrationRunning())
    return;

  std::optional<float> voltage = grid.getVoltage();
  std::optional<float> power = grid.getPower();

  if (voltage.has_value() && power.has_value()) {
    float powerToDivert = pidController.compute(power.value());
    float diverted = router.divert(voltage.value(), powerToDivert);

    if (diverted > 0) {
      if (pidTask.lastEndTime() > 0 && millis() - pidTask.lastEndTime() >= 2000) {
        ESP_LOGW(TAG, "Grid measurement system too slow!");
      }
    } else {
      pidController.reset(0);
    }

    if (website.realTimePIDEnabled()) {
      dashboardUpdateTask.requestEarlyRun();
    }

  } else {
    // pause routing if grid voltage or power are not available
    router.noDivert();
  }
});

void yasolr_init_pid() {
  pidController.setReverse(false);
  pidController.setIntegralCorrectionMode(Mycila::PID::IntegralCorrectionMode::CLAMP);

  pidTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  if (config.get<bool>(KEY_ENABLE_DEBUG))
    pidTask.enableProfiling();
  coreTaskManager.addTask(pidTask);
}

void yasolr_configure_pid() {
  if (config.isEqual(KEY_PID_TRIGGER, YASOLR_PID_TRIGGER_MEASURE)) {
    pidController.setTimeSampling(false);
    pidTask.setInterval(2000); // will be triggered each 2 sec at most
  } else {
    pidController.setTimeSampling(true);
    pidTask.setInterval(config.get<uint16_t>(KEY_PID_INTERVAL));
  }

  pidController.setProportionalMode(strcmp(config.getString(KEY_PID_MODE_P), YASOLR_PID_MODE_ERROR) == 0 ? Mycila::PID::ProportionalMode::ON_ERROR : Mycila::PID::ProportionalMode::ON_INPUT);
  pidController.setSetpoint(config.get<int16_t>(KEY_PID_SETPOINT));
  pidController.setTunings(config.get<float>(KEY_PID_KP), config.get<float>(KEY_PID_KI), config.get<float>(KEY_PID_KD));
  pidController.setOutputLimits(config.get<int16_t>(KEY_PID_OUT_MIN), config.get<int16_t>(KEY_PID_OUT_MAX));
  pidController.setFilterAlpha(1.0f - config.get<uint8_t>(KEY_PID_NOISE) / 100.0f);

  ESP_LOGI(TAG, "PID Controller configured");
}
