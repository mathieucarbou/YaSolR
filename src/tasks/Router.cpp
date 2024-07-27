// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task calibrationTask("Calibration", [](void* params) { router.calibrate(); });

Mycila::Task routerTask("Router", [](void* params) {
  std::optional<float> voltage = grid.getVoltage();
  const Mycila::ExpiringValue<float>& power = grid.power();

  if (!voltage.has_value() || power.isAbsent())
    router.noDivert();

  output1.applyTemperatureLimit();
  output2.applyTemperatureLimit();

  output1.applyAutoBypass();
  output2.applyAutoBypass();
});

Mycila::Task relayTask("Relay", [](void* params) {
  Mycila::Grid::Metrics gridMetrics;
  grid.getMeasurements(gridMetrics);

  Mycila::Router::Metrics routerMetrics;
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
