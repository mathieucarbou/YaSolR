// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task routerTask("Router", [](void* params) {
  grid.applyExpiration();

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
  Mycila::GridMetrics gridMetrics;
  grid.getMeasurements(gridMetrics);
  router.divert(gridMetrics.voltage, gridMetrics.power);
  if (config.getBool(KEY_ENABLE_DEBUG)) {
    String message;
    message.reserve(256);
    message.concat(pidController.getProportionalMode());
    message.concat(",");
    message.concat(pidController.getDerivativeMode());
    message.concat(",");
    message.concat(pidController.getIntegralCorrectionMode());
    message.concat(",");
    message.concat(pidController.isReversed());
    message.concat(",");
    message.concat(pidController.getSetPoint());
    message.concat(",");
    message.concat(pidController.getKp());
    message.concat(",");
    message.concat(pidController.getKi());
    message.concat(",");
    message.concat(pidController.getKd());
    message.concat(",");
    message.concat(pidController.getOutputMin());
    message.concat(",");
    message.concat(pidController.getOutputMax());
    message.concat(",");
    message.concat(pidController.getInput());
    message.concat(",");
    message.concat(pidController.getOutput());
    message.concat(",");
    message.concat(pidController.getError());
    message.concat(",");
    message.concat(pidController.getPTerm());
    message.concat(",");
    message.concat(pidController.getITerm());
    message.concat(",");
    message.concat(pidController.getDTerm());
    message.concat(",");
    message.concat(pidController.getSum());
    wsDebugPID.textAll(message);
  }
});
