// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaRelay.h>

#ifndef MYCILA_RELAY_DEFAULT_TOLERANCE
  // in percentage
  // => 50W for a tri-phase 3000W resistance (1000W per phase)
  // => 35W for a tri-phase 2100W resistance (700W per phase)
  #define MYCILA_RELAY_DEFAULT_TOLERANCE 0.05f
#endif

namespace Mycila {
  class RouterRelay {
    public:
      void setNominalLoad(uint16_t load) { _nominalLoad = load; }
      uint16_t getNominalLoad() const { return _nominalLoad; }

      /**
       * @brief Calculate the load based on the grid voltage and the nominal load.
       */
      uint16_t getLoad(float gridVoltage) const {
        // detects the grid nominal voltage
        const uint16_t nominalVoltage = static_cast<uint8_t>(gridVoltage / 100) == 1 ? 110 : 230;
        // calculate the amperage of the given nominal power of connected load
        const float resistance = static_cast<float>(nominalVoltage * nominalVoltage) / static_cast<float>(_nominalLoad);
        // calculate with the current voltage what the exact power of the load would be
        return static_cast<uint16_t>(gridVoltage * gridVoltage / resistance);
      }

      void setTolerance(float tolerance) { _tolerance = tolerance; }
      float getTolerance() const { return _tolerance; }

      bool isEnabled() const { return _relay.isEnabled(); }
      bool isAutoRelayEnabled() const { return _relay.isEnabled() && _nominalLoad > 0; }

      bool trySwitchRelay(bool state, uint32_t duration = 0);

      bool autoSwitch(float gridVoltage, float gridPower, float routedPower, float setpoint);

      bool isOn() const { return _relay.isOn(); }
      bool isOff() const { return _relay.isOff(); }

      uint64_t getSwitchCount() const { return _relay.getSwitchCount(); }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const { _relay.toJson(root); }
#endif

      Mycila::Relay& relay() { return _relay; }

    private:
      Mycila::Relay _relay;
      uint16_t _nominalLoad = 0;
      float _tolerance = MYCILA_RELAY_DEFAULT_TOLERANCE;
  };
} // namespace Mycila
