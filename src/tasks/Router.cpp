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
  float virtualGridPower = grid.getActivePower() - router.getActivePower();
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
});
