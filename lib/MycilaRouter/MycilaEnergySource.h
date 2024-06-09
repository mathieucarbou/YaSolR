// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  class EnergySource {
    public:
      static const int X = 10;

      EnergySource(){};
      ~EnergySource(){};

      // Electricity connected
      virtual bool isConnected() const = 0;

      // Current frequency in Hz
      virtual float getFrequency() const = 0;

      // Active energy in kWh
      virtual float getEnergy() const = 0;
      // Active energy returned in kWh
      virtual float getEnergyReturned() const = 0;

      // Current voltage in V
      virtual float getVoltage() const = 0;
      // Current used in A
      virtual float getCurrent() const = 0;

      // Active power in W (can be negative if energy is returned)
      virtual float getActivePower() const = 0;
      // Apparent power in VA
      virtual float getApparentPower() const = 0;
      // Power factor (0 - 1)
      virtual float getPowerFactor() const = 0;

      // Total harmonic distortion of current, if available
      virtual float getTHDi() const = 0;

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["apparent_power"] = getApparentPower();
        root["current"] = getCurrent();
        root["energy_returned"] = getEnergyReturned();
        root["energy"] = getEnergy();
        root["frequency"] = getFrequency();
        root["online"] = isConnected();
        root["power_factor"] = getPowerFactor();
        root["power"] = getActivePower();
        root["thdi"] = getTHDi();
        root["voltage"] = getVoltage();
      }
#endif
  };
} // namespace Mycila
