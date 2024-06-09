// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

/*
  Dimmer library for ESP32, based on Dimmable Light library (https://github.com/fabianoriccardi/dimmable-light/).

  This library supports multiple implementations and LUT table to speed up the computation of phase angle, firing delay and RMS voltage.

  * Some LUT Graphs and stats:
    - https://docs.google.com/spreadsheets/d/1dCpAydu3WHekbXEdUZt53acCa6aSjtCBxW6lvv89Ku0/edit?usp=sharing
    - https://www.desmos.com/calculator/llwqitrjck
  
  * LUT Generation can be done using the `generateLUT` method

  References:
  * https://electronics.stackexchange.com/questions/414370/calculate-angle-for-triac-phase-control
  * https://www.quora.com/How-do-you-calculate-the-phase-angle-from-time-delay
  * https://www.physicsforums.com/threads/how-to-calculate-rms-voltage-from-triac-phase-angle.572668/
  * https://github.com/fabianoriccardi/dimmable-light/blob/main/src/dimmable_light.h
  * https://github.com/fabianoriccardi/dimmable-light/blob/main/src/dimmable_light_linearized.h
  * https://www.quora.com/How-can-I-calculate-this-question-A-10-ohm-load-is-connected-through-a-half-wave-S-C-R-circuit-to-220v-50Hz-single-phase-source-calculate-power-delivered-to-the-load-for-firing-angle-of-60-and-the-power-of-input/answer/Arivayutham-Periasamy
  * https://electronics.stackexchange.com/questions/225875/triac-firing-angle-vs-power-delivered-to-load-how-calculate
*/

#include <ArduinoJson.h>
#include <WString.h>
#include <esp32-hal-gpio.h>
#include <thyristor.h>

#include <functional>

#define MYCILA_DIMMER_MAX_LEVEL 100

namespace Mycila {
  enum class DimmerLevel { OFF,
                           FULL,
                           DIM };

  typedef std::function<void(DimmerLevel event)> DimmerLevelCallback;

  class Dimmer {
    public:
      ~Dimmer() { end(); }

      void begin(const int8_t pin);
      void end();

      void listen(DimmerLevelCallback callback) { _callback = callback; }

      void off() { setLevel(0); }
      bool isOff() const { return _level == 0; }
      bool isOn() const { return _level > 0; }
      bool isOnAtFullPower() const { return _level >= MYCILA_DIMMER_MAX_LEVEL; }

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _enabled; }

      void toJson(const JsonObject& root) const;

      // Returns the dimmer level: 0 - MYCILA_DIMMER_MAX_LEVEL
      uint8_t getLevel() const { return _level; }

      // Set dimmer level: 0 - MYCILA_DIMMER_MAX_LEVEL
      // Depending on the implementation, this can be a linear or non-linear dimming curve
      // Example: Whether 50% means 50% power or 50% of the firing delay is implementation dependent
      void setLevel(uint8_t level);

      // Returns the current dimmer firing delay in microseconds
      // At 0% power, the firing delay is equal to the semi-period (i.e. 10000 us for 50Hz, 8333 us for 60Hz)
      // At 100% power, the firing delay is 0 us
      uint16_t getFiringDelay() const { return _dimmer->getDelay(); }

      // Returns the dimmer phase angle in radians from 0 - PI.
      // At 0% power, the phase angle is equal to PI
      // At 100% power, the phase angle is equal to 0
      float getPhaseAngle() const;

      // Returns the dimmed RMS voltage based on the current phase angle.
      // This is a theoretical value. In reality, the real Vrms value will be more or less depending on the hardware and software speed.
      float getDimmedVoltage(float inputVrms) const;

      static void generateLUT(Print& out); // NOLINT

    private:
      bool _enabled = false;
      uint8_t _level = 0;
      gpio_num_t _pin = GPIO_NUM_NC;
      DimmerLevelCallback _callback = nullptr;
      Thyristor* _dimmer = nullptr;

    private:
      static float _delayToPhaseAngle(uint16_t delay, float frequency);
      static uint16_t _phaseAngleToDelay(float angle, float frequency);
      static float _vrmsFactor(float angle);

    private:
      // Can be implemented using algorithm or LUT table
      static uint16_t _lookupFiringDelay(uint8_t level, float frequency);
      // Can be implemented using algorithm or LUT table
      static float _lookupVrmsFactor(uint8_t level, float frequency);
  };
} // namespace Mycila
