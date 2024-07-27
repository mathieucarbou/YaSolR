// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task lightsTask("Lights", [](void* params) {
  if (!otaTask.isPaused()) {
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

  lights.setYellow(relay1.isOn() || relay2.isOn() || bypassRelayO1.isOn() || bypassRelayO2.isOn() || dimmerO1.isOn() || dimmerO2.isOn());

  if (!grid.isConnected()) {
    lights.setRed(true);
    return;
  }

  switch (ESPConnect.getState()) {
    case ESPConnectState::NETWORK_CONNECTED:
    case ESPConnectState::AP_STARTED:
      lights.setRed(false);
      break;
    default:
      lights.setRed(true);
      break;
  }
});
