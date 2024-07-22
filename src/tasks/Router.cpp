// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task routerTask("Router", [](void* params) {
  std::optional<float> voltage = grid.getVoltage();
  const Mycila::ExpiringValue<float>& power = grid.power();

  if (!voltage.has_value() || power.isAbsent())
    router.noDivert();

  output1.applyDimmerLimits();
  output2.applyDimmerLimits();

  output1.applyAutoBypass();
  output2.applyAutoBypass();
});

Mycila::Task relayTask("Relay", [](void* params) {
  Mycila::GridMetrics gridMetrics;
  grid.getMeasurements(gridMetrics);

  Mycila::RouterMetrics routerMetrics;
  router.getMeasurements(routerMetrics);

  float virtualGridPower = gridMetrics.power - routerMetrics.power;

  if (routerRelay1.tryRelayStateAuto(true, virtualGridPower))
    return;
  if (routerRelay2.tryRelayStateAuto(true, virtualGridPower))
    return;
  if (routerRelay2.tryRelayStateAuto(false, virtualGridPower))
    return;
  if (routerRelay1.tryRelayStateAuto(false, virtualGridPower))
    return;
});

Mycila::Task routingTask("Routing", Mycila::TaskType::ONCE, [](void* params) {
  std::optional<float> voltage = grid.getVoltage();
  const Mycila::ExpiringValue<float>& power = grid.power();

  if (voltage.has_value() && power.isPresent()) {
    router.divert(voltage.value(), power.get());
    if (config.getBool(KEY_ENABLE_PID_VIEW)) {
      dashboardTask.requestEarlyRun();
    }
  }
});
