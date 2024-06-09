// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task relaysTask("Relays", [](void* params) {
  Mycila::RelayManager.autoCommute();
});

Mycila::Task routerTask("Router", [](void* params) {
  grid.invalidate();

  output1.updateElectricityStatistics();
  output1.applyDimmerLimit();
  output1.autoBypass();

  output2.updateElectricityStatistics();
  output2.applyDimmerLimit();
  output2.autoBypass();
});
