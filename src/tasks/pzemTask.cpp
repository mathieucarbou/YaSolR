// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task pzemTask("PZEM", [](void* params) {
  // PZEM are on the same serial so cannot be read concurrently
  pzemO1.read();
  yield();
  pzemO2.read();
});
