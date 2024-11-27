// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task routingTask("Routing", Mycila::TaskType::ONCE, [](void* params) {
  std::optional<float> voltage = grid.getVoltage();
  const Mycila::ExpiringValue<float>& power = grid.power();

  if (voltage.has_value() && power.isPresent()) {
    router.divert(voltage.value(), power.get());
    if (config.getBool(KEY_ENABLE_PID_VIEW)) {
      dashboardUpdateTask.requestEarlyRun();
    }
  }
});
