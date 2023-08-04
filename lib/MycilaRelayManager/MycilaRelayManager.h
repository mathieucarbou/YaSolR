// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRelay.h>

#include <ArduinoJson.h>
#include <vector>

#ifndef MYCILA_RELAY_THRESHOLD_TOLERANCE_START
#define MYCILA_RELAY_THRESHOLD_TOLERANCE_START 20
#endif

#ifndef MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP
#define MYCILA_RELAY_THRESHOLD_TOLERANCE_STOP 20
#endif

#ifndef MYCILA_RELAY_PAUSE_DURATION
#define MYCILA_RELAY_PAUSE_DURATION 60
#endif

namespace Mycila {

  class RelayManagerConfigClass {
    public:
      uint16_t getPowerThreshold(const char* name) const;
  };

  class RelayManagerClass {
    public:
      void begin();
      void loop();
      void end();

      void toJson(const JsonObject& root) const;

      bool tryRelayState(int idx, bool state, uint32_t duration = 0);

    public:
      std::vector<Relay> relays;

    private:
      uint32_t _lastCheck = 0;
  };

  extern RelayManagerConfigClass RelayManagerConfig;
  extern RelayManagerClass RelayManager;
} // namespace Mycila
