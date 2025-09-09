// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::TrafficLight lights;

static Mycila::Task lightsTask("Lights", [](void* params) {
  if (safeBootTask.scheduled()) {
    lights.setAllOn();
    return;
  }

  if (restartTask.scheduled()) {
    lights.set(Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::ON);
    return;
  }

  if (resetTask.scheduled()) {
    lights.set(Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::ON, Mycila::TrafficLight::State::ON);
    return;
  }

  lights.setGreen(true);

  lights.setYellow((relay1 && relay1->isOn()) || (relay2 && relay2->isOn()) || (output1 && output1->isOn()) || (output2 && output2->isOn()));

  if (!grid.isConnected()) {
    lights.setRed(true);
    return;
  }

  switch (espConnect.getState()) {
    case Mycila::ESPConnect::State::NETWORK_CONNECTED:
    case Mycila::ESPConnect::State::AP_STARTED:
      lights.setRed(false);
      break;
    default:
      lights.setRed(true);
      break;
  }
});

void yasolr_init_lights() {
  LOGI(TAG, "Initialize system lights");

  if (config.getBool(KEY_ENABLE_LIGHTS)) {
    lights.begin(config.getLong(KEY_PIN_LIGHTS_GREEN), config.getLong(KEY_PIN_LIGHTS_YELLOW), config.getLong(KEY_PIN_LIGHTS_RED));
  }

  lights.set(Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::ON, Mycila::TrafficLight::State::OFF);

  lightsTask.setInterval(200);
  coreTaskManager.addTask(lightsTask);
}
