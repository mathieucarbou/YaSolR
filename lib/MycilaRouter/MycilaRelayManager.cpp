// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaRelayManager.h>

#include <MycilaLogger.h>
#include <MycilaRouter.h>
#include <algorithm>

#ifndef MYCILA_RELAY_TOLERANCE
// in percentage
// => 30W for a tri-phase 3000W resistance
// => 21W for a tri-phase 2100W resistance
#define MYCILA_RELAY_TOLERANCE 3
#endif

#define TAG "RELAYS"

extern Mycila::Logger logger;

void Mycila::RelayManagerClass::autoCommute() {
  const float grid = Router.getVirtualGridPower();

  // check if we need to start a relay
  {
    for (const auto& [name, relay] : _relays) {
      if (relay->isEnabled() && relay->isOff()) {
        const float load = RelayManagerConfig.getRelayLoad(name);
        // reminder: grid wil be negative in case of excess
        if (load > 0 && grid + load <= -load * MYCILA_RELAY_TOLERANCE / 100.0) {
          logger.info(TAG, "Auto starting relay '%s'...", name);
          relay->setState(true);
          // we stop the loop and let time for routing to adjust before checking again
          return;
        }
      }
    }
  }

  // check if we need to stop a relay
  {
    for (auto rit = _relays.rbegin(); rit != _relays.rend(); ++rit) {
      const char* name = rit->first;
      Relay* relay = rit->second;
      if (relay->isEnabled() && relay->isOn()) {
        const float load = RelayManagerConfig.getRelayLoad(name);
        // reminder: grid wil be positive in case of import
        if (load > 0 && grid >= load * MYCILA_RELAY_TOLERANCE / 100.0) {
          logger.info(TAG, "Auto stopping relay '%s'...", name);
          relay->setState(false);
          // we stop the loop and let time for routing to adjust before checking again
          return;
        }
      }
    }
  }
}

bool Mycila::RelayManagerClass::tryRelayState(const char* name, bool switchOn, uint32_t duration) {
  Relay* relay = _relays[name];
  if (!relay->isEnabled())
    return false;

  if (RelayManagerConfig.getRelayLoad(name) > 0) {
    logger.warn(TAG, "Relay '%s' cannot be activated since it is set in automatic mode", name);
    return false;
  }

  relay->setState(switchOn, duration);
  return true;
}

namespace Mycila {
  RelayManagerClass RelayManager;
  RelayManagerConfigClass RelayManagerConfig;
} // namespace Mycila
