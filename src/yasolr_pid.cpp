// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::PID pidController;

static Mycila::Task pidTask("PID", []() {
  if ((output1.isAutoDimmerEnabled() || output2.isAutoDimmerEnabled()) && micros() - pidController.getLastTime() > 2000000 && yasolr_run_pid()) {
    // Ensure the PID controller is called frequently enough.
    // This keeps the PID running even if the measurements are not frequent enough (like with MQTT).
    ESP_LOGW(TAG, "Grid measurement system too slow!");
  }
});

bool yasolr_run_pid() {
  if (router.isCalibrationRunning())
    return false;

  std::optional<float> voltage = grid.getVoltage();
  std::optional<float> power = grid.getPower();

  if (voltage.has_value() && power.has_value()) {
    float powerToDivert = pidController.compute(power.value());
    float diverted = router.divert(voltage.value(), powerToDivert);

    if (website.realTimePIDEnabled()) {
      dashboardUpdateTask.requestEarlyRun();
    }

    return diverted > 0;

  } else {
    // pause routing if grid voltage or power are not available
    router.noDivert();
    return false;
  }
}

void yasolr_init_pid() {
  pidController.setReverse(false);
  pidController.setTimeSampling(false);
  pidController.setIntegralCorrectionMode(Mycila::PID::IntegralCorrectionMode::CLAMP);

  pidTask.setEnabledWhen([]() { return !router.isCalibrationRunning(); });
  pidTask.setInterval(200);
  if (config.get<bool>(KEY_ENABLE_DEBUG))
    pidTask.enableProfiling();
  coreTaskManager.addTask(pidTask);
}

void yasolr_configure_pid() {
  pidController.setProportionalMode(strcmp(config.getString(KEY_PID_MODE_P), YASOLR_PID_MODE_ERROR) == 0 ? Mycila::PID::ProportionalMode::ON_ERROR : Mycila::PID::ProportionalMode::ON_INPUT);
  pidController.setSetpoint(config.get<int16_t>(KEY_PID_SETPOINT));
  pidController.setTunings(config.get<float>(KEY_PID_KP), config.get<float>(KEY_PID_KI), config.get<float>(KEY_PID_KD));
  pidController.setOutputLimits(config.get<int16_t>(KEY_PID_OUT_MIN), config.get<int16_t>(KEY_PID_OUT_MAX));
  // pidController.setFilterTimeConstant(1.0f, 0.33f);
  pidController.setFilterAlpha(1.0f - config.get<uint8_t>(KEY_PID_NOISE) / 100.0f);
  ESP_LOGI(TAG, "PID Controller configured");
}
