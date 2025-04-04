// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaRelay.h>

namespace Mycila {
  class RouterRelay {
    public:
      explicit RouterRelay(Relay& relay) : _relay(&relay) {}

      void setLoad(uint16_t load) { _load = load; }

      bool isEnabled() const { return _relay->isEnabled(); }
      bool isAutoRelayEnabled() const { return _relay->isEnabled() && _load > 0; }

      bool trySwitchRelay(bool state, uint32_t duration = 0);

      bool autoSwitch(float gridPower, float routedPower, float setpoint);

      bool isOn() const { return _relay->isOn(); }
      bool isOff() const { return _relay->isOff(); }

      uint64_t getSwitchCount() const { return _relay->getSwitchCount(); }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const { _relay->toJson(root); }
#endif

    private:
      Mycila::Relay* _relay;
      uint16_t _load = 0;
  };
} // namespace Mycila
