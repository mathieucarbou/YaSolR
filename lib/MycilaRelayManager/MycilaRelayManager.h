// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRelay.h>

#include <ArduinoJson.h>
#include <vector>

namespace Mycila {

  class RelayManagerConfigClass {
    public:
      uint16_t getPowerThreshold(const char* name) const;
  };

  class RelayManagerClass {
    public:
      void setRelays(const std::vector<Relay*> relays) { _relays = relays; }

      void toJson(const JsonObject& root) const;

      void autoCommute();
      bool tryRelayState(Relay* relay, bool state, uint32_t duration = 0);

    private:
      std::vector<Relay*> _relays = {};
  };

  extern RelayManagerConfigClass RelayManagerConfig;
  extern RelayManagerClass RelayManager;
} // namespace Mycila
