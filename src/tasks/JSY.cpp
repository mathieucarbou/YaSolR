// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task jsyConfigTask("JSY Config", Mycila::TaskType::ONCE, [](void* params) {
  jsy.end();
  if (config.getBool(KEY_ENABLE_JSY)) {
    jsy.begin(YASOLR_JSY_SERIAL, config.get(KEY_PIN_JSY_RX).toInt(), config.get(KEY_PIN_JSY_TX).toInt());
    if (jsy.isEnabled() && jsy.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400)
      jsy.setBaudRate(Mycila::JSYBaudRate::BAUD_38400);
  }
});

Mycila::Task jsyTask("JSY", [](void* params) { jsy.read(); });
