// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRelayManager.h>

#include <MycilaLogger.h>
#include <MycilaRouter.h>
#include <algorithm>

#ifndef MYCILA_RELAY_THRESHOLD_TOLERANCE_START
#define MYCILA_RELAY_THRESHOLD_TOLERANCE_START 20
#endif

#ifndef MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP
#define MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP 20
#endif

#define TAG "RELAY-M"

void Mycila::RelayManagerClass::toJson(const JsonObject& root) const {
  for (const auto& relay : _relays) {
    relay->toJson(root[relay->getName()].to<JsonObject>());
  }
}

void Mycila::RelayManagerClass::autoCommute() {
  const float grid = Router.getVirtualGridPower();

  // check if we need to start a relay
  {
    for (auto& relay : _relays) {
      if (relay->isEnabled() && relay->isOff()) {
        const float threshold = RelayManagerConfig.getPowerThreshold(relay->getName());
        if (threshold > 0 && grid <= -(threshold + round(threshold * MYCILA_RELAY_THRESHOLD_TOLERANCE_START / 100.0))) {
          Logger.info(TAG, "Auto starting relay '%s'...", relay->getName());
          relay->setState(true);
          // we stop the loop and let time for routing to adjust before checking again
          return;
        }
      }
    }
  }

  // check if we need to stop a relay
  {
    for (int i = _relays.size() - 1; i >= 0; i--) {
      auto& relay = _relays[i];
      if (relay->isEnabled() && relay->isOn()) {
        const float threshold = RelayManagerConfig.getPowerThreshold(relay->getName());
        if (threshold > 0 && grid - threshold >= -(threshold - round(threshold * MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP / 100.0))) {
          Logger.info(TAG, "Auto stopping relay '%s'...", relay->getName());
          relay->setState(false);
          // we stop the loop and let time for routing to adjust before checking again
          return;
        }
      }
    }
  }
}

bool Mycila::RelayManagerClass::tryRelayState(Relay* relay, bool switchOn, uint32_t duration) {
  if (!relay->isEnabled())
    return false;

  if (RelayManagerConfig.getPowerThreshold(relay->getName()) > 0) {
    Logger.info(TAG, "Auto Mode '%s' is activated: unable to switch Relay...", relay->getName());
    return false;
  }

  relay->setState(switchOn, duration);
  return true;
}

namespace Mycila {
  RelayManagerClass RelayManager;
  RelayManagerConfigClass RelayManagerConfig;
} // namespace Mycila
