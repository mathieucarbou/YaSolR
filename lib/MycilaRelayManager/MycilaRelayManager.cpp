// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRelayManager.h>

#include <MycilaLogger.h>
#include <MycilaRouter.h>
#include <algorithm>

#define TAG "RELAY-M"

void Mycila::RelayManagerClass::begin() {
  for (auto& relay : relays) {
    relay.begin();
  }
}

void Mycila::RelayManagerClass::end() {
  for (auto& relay : relays) {
    relay.end();
  }
}

void Mycila::RelayManagerClass::toJson(const JsonObject& root) const {
  for (int i = 0; i < relays.size(); i++) {
    auto& relay = relays[i];
    JsonObject outputJson = root[relay.name].to<JsonObject>();
    relay.toJson(outputJson);
  }
}

void Mycila::RelayManagerClass::loop() {
  if (millis() - _lastCheck >= MYCILA_RELAY_PAUSE_DURATION * 1000) {
    const float grid = Router.getVirtualGridPower();

    // check if we need to start a relay
    {
      for (auto& relay : relays) {
        if (relay.isEnabled() && relay.isOff()) {
          const float threshold = RelayManagerConfig.getPowerThreshold(relay.name);
          if (threshold > 0 && grid <= -(threshold + round(threshold * MYCILA_RELAY_THRESHOLD_TOLERANCE_START / 100.0))) {
            Logger.info(TAG, "Excess above %u W +%u%%: starting relay '%s'...", threshold, MYCILA_RELAY_THRESHOLD_TOLERANCE_START, relay.name);
            relay.setState(true);
            // we stop the loop and let time for routing to adjust before checking again
            return;
          }
        }
      }
    }

    // check if we need to stop a relay
    {
      for (int i = relays.size() - 1; i >= 0; i--) {
        auto& relay = relays[i];
        if (relay.isEnabled() && relay.isOn()) {
          const float threshold = RelayManagerConfig.getPowerThreshold(relay.name);
          if (threshold > 0 && grid - threshold >= -(threshold - round(threshold * MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP / 100.0))) {
            Logger.info(TAG, "Excess below %u W -%u%%: stopping relay '%s'...", threshold, MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP, relay.name);
            relay.setState(false);
            // we stop the loop and let time for routing to adjust before checking again
            return;
          }
        }
      }
    }

    _lastCheck = millis();
  }
}

bool Mycila::RelayManagerClass::tryRelayState(int idx, bool switchOn, uint32_t duration) {
  auto& relay = relays[idx];

  if (!relay.isEnabled())
    return false;

  if (RelayManagerConfig.getPowerThreshold(relay.name) > 0) {
    Logger.warn(TAG, "Auto Mode '%s' is activated: unable to switch Relay...", relay.name);
    return false;
  }

  relay.setState(switchOn, duration);
  _lastCheck = millis();
  return true;
}

namespace Mycila {
  RelayManagerClass RelayManager;
  RelayManagerConfigClass RelayManagerConfig;
} // namespace Mycila
