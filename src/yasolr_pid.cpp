// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

Mycila::PID pidController;

void yasolr_configure_pid() {
  pidController.setReverse(false);
  pidController.setTimeSampling(false);
  pidController.setIntegralCorrectionMode(Mycila::PID::IntegralCorrectionMode::CLAMP);
  pidController.setProportionalMode(strcmp(config.getString(KEY_PID_MODE_P), YASOLR_PID_MODE_ERROR) == 0 ? Mycila::PID::ProportionalMode::ON_ERROR : Mycila::PID::ProportionalMode::ON_INPUT);
  pidController.setSetpoint(config.get<int16_t>(KEY_PID_SETPOINT));
  pidController.setTunings(config.get<float>(KEY_PID_KP), config.get<float>(KEY_PID_KI), config.get<float>(KEY_PID_KD));
  pidController.setOutputLimits(config.get<int16_t>(KEY_PID_OUT_MIN), config.get<int16_t>(KEY_PID_OUT_MAX));
  ESP_LOGI(TAG, "PID Controller configured");
}
