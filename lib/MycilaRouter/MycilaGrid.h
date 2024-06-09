// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaEnergySource.h>

namespace Mycila {
  class Grid : public EnergySource {
    public:
      void setExpiration(uint32_t seconds) { _delay = seconds * 1000; }

      void updateVoltage(float voltage) {
        _voltage = voltage;
        _lastUpdate = millis();
      }
      void updatePower(float power) {
        _power = power;
        _lastUpdate = millis();
      }
      void update(float current,
                  float energy,
                  float energyReturned,
                  float frequency,
                  float power,
                  float powerFactor) {
        _current = current;
        _energy = energy;
        _energyReturned = energyReturned;
        _frequency = frequency;
        _power = power;
        _powerFactor = powerFactor;
        _lastUpdate = millis();
      }

      bool isExpired() const { return _delay > 0 && millis() - _lastUpdate > _delay; }
      void invalidate() {
        if (isExpired()) {
          _current = 0;
          _energy = 0;
          _energyReturned = 0;
          _frequency = 0;
          _power = 0;
          _powerFactor = 0;
          _voltage = 0;
        }
      }

      // implement EnergySource
      inline bool isConnected() const override { return getVoltage() > 0; }
      float getFrequency() const override;
      float getEnergy() const override { return _energy; }
      float getEnergyReturned() const override { return _energyReturned; }
      // returns the measured voltage or the nominal voltage if not connected
      float getVoltage() const override { return _voltage; }
      float getCurrent() const override { return _current; }
      float getActivePower() const override { return _power; }
      inline float getApparentPower() const override { return _powerFactor == 0 ? 0 : _power / _powerFactor; }
      float getPowerFactor() const override { return _powerFactor; }
      float getTHDi() const override {
        // requires to know the Displacement Power Factor, cosφ,
        // due to the phase shift between voltage and current
        return 0;
      }

    private:
      // metrics
      volatile float _current = 0;
      volatile float _energy = 0;
      volatile float _energyReturned = 0;
      volatile float _frequency = 0;
      volatile float _power = 0;
      volatile float _powerFactor = 0;
      volatile float _voltage = 0;

      // expiration
      uint32_t _delay = 0;
      volatile uint32_t _lastUpdate = 0;
  };
} // namespace Mycila
