// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRelay.h>

namespace Mycila {
  class RouterRelay {
    public:
      explicit RouterRelay(Relay& relay) : _relay(&relay) {}

      void setLoad(uint16_t load) { _load = load; }

      bool isAutoRelayEnabled() const { return _relay->isEnabled() && _load > 0; }

      bool tryRelayState(bool state, uint32_t duration = 0);
      bool tryRelayStateAuto(bool state, float virtualGridPower);

      bool isOn() const { return _relay->isOn(); }
      bool isOff() const { return _relay->isOff(); }

    private:
      Mycila::Relay* _relay = nullptr;
      uint16_t _load = 0;
  };
} // namespace Mycila
