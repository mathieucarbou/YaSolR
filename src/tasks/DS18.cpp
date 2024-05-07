// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task ds18Task("DS18", [](void* params) {
  ds18Sys.read();
  ds18O1.read();
  ds18O2.read();
});
