// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRelay.h>

#include <ArduinoJson.h>
#include <map>

namespace Mycila {

  class RelayManagerConfigClass {
    public:
      uint16_t getPowerThreshold(const char* name) const;
  };

  class RelayManagerClass {
    public:
      void addRelay(const char* name, Relay* relay) { _relays[name] = relay; }

      void toJson(const JsonObject& root) const;

      void autoCommute();
      bool tryRelayState(const char* name, bool state, uint32_t duration = 0);

      const Relay* getRelay(const char* name) const { return _relays.at(name); }

    private:
      std::map<const char*, Relay*> _relays;
  };

  extern RelayManagerConfigClass RelayManagerConfig;
  extern RelayManagerClass RelayManager;
} // namespace Mycila
