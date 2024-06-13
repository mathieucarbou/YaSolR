// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task routerTask("Router", [](void* params) {
  grid.applyExpiration();

  output1.applyDimmerLimit();
  output2.applyDimmerLimit();

  output1.applyAutoBypass();
  output2.applyAutoBypass();
});

Mycila::Task relayTask("Relay", [](void* params) {
  Mycila::GridMetrics gridMetrics;
  grid.getMetrics(gridMetrics);

  Mycila::RouterMetrics routerMetrics;
  router.getMetrics(routerMetrics);

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
  Mycila::GridMetrics gridMetrics;
  grid.getMetrics(gridMetrics);

  Mycila::RouterMetrics routerMetrics;
  router.getMetrics(routerMetrics);

  float virtualGridPower = gridMetrics.power - routerMetrics.power;

  // if (virtualGridPower < 0) {
  //   if (output1.isAutoDimmerEnabled() && dimmerO1.getPowerDuty() < output1.config.dimmerLimit) {
  //   }
  //   if (output2.isAutoDimmerEnabled() && dimmerO2.getPowerDuty() < output2.config.dimmerLimit) {
  //   }
  // }
});
