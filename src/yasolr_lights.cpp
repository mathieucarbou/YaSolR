// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::TrafficLight lights;

Mycila::Task lightsTask("Lights", [](void* params) {
  if (!safeBootTask.isPaused()) {
    lights.setAllOn();
    return;
  }

  if (!restartTask.isPaused()) {
    lights.set(Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::ON);
    return;
  }

  if (!resetTask.isPaused()) {
    lights.set(Mycila::TrafficLight::State::OFF, Mycila::TrafficLight::State::ON, Mycila::TrafficLight::State::ON);
    return;
  }

  lights.setGreen(true);

  lights.setYellow((relay1 && relay1->isOn()) || (relay2 && relay2->isOn()) || bypassRelayO1.isOn() || bypassRelayO2.isOn() || dimmerO1.isOn() || dimmerO2.isOn());

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
  logger.info(TAG, "Initialize system lights...");

  if (config.getBool(KEY_ENABLE_LIGHTS))
    lights.begin(config.getLong(KEY_PIN_LIGHTS_GREEN), config.getLong(KEY_PIN_LIGHTS_YELLOW), config.getLong(KEY_PIN_LIGHTS_RED));

  lightsTask.setInterval(200 * Mycila::TaskDuration::MILLISECONDS);
  lightsTask.setManager(coreTaskManager);
}
