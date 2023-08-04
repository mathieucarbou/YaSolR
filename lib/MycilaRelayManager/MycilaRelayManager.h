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
      void begin();
      void end();

      void autoCommute();

      void toJson(const JsonObject& root) const;

      bool tryRelayState(int idx, bool state, uint32_t duration = 0);

    public:
      std::vector<Relay> relays;
  };

  extern RelayManagerConfigClass RelayManagerConfig;
  extern RelayManagerClass RelayManager;
} // namespace Mycila
