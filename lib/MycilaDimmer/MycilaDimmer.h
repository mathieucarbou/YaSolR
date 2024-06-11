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

// Dimmer goes from 0 to 10000
#define MYCILA_DIMMER_MAX_LEVEL 10000

namespace Mycila {
  enum class DimmerLevel { OFF,
                           FULL,
                           DIM };

  typedef std::function<void(DimmerLevel event)> DimmerLevelCallback;

  class Dimmer {
    public:
      ~Dimmer() { end(); }

      void begin(const int8_t pin, const uint8_t frequency);
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
      uint16_t getLevel() const { return _level; }

      // Set dimmer level: 0 - MYCILA_DIMMER_MAX_LEVEL
      // Depending on the implementation, this can be a linear or non-linear dimming curve
      // Example: Whether 50% means 50% power or 50% of the firing delay is implementation dependent
      void setLevel(uint16_t level);

      // Returns the current dimmer firing delay in microseconds
      // At 0% power, the firing delay is equal to the semi-period (i.e. 10000 us for 50Hz, 8333 us for 60Hz)
      // At 100% power, the firing delay is 0 us
      uint16_t getFiringDelay() const { return _dimmer->getDelay(); }

      // Returns the dimmer phase angle in radians from 0 - PI.
      // At 0% power, the phase angle is equal to PI
      // At 100% power, the phase angle is equal to 0
      float getPhaseAngle() const { return _delayToPhaseAngle(_dimmer->getDelay(), _frequency); }

      // Returns the dimmed RMS voltage based on the current phase angle bewteen 0 and 1 to be multiplied with the voltage
      float getVrms() const { return _vrms; }

      static void generateLUT(Print& out, size_t lutSize); // NOLINT

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint8_t _frequency;
      bool _enabled = false;
      DimmerLevelCallback _callback = nullptr;
      Thyristor* _dimmer = nullptr;
      uint16_t _level = 0;
      float _vrms = 0;

    private:
      static float _delayToPhaseAngle(uint16_t delay, float frequency);
      static uint16_t _phaseAngleToDelay(float angle, float frequency);
      static float _vrmsFactor(float angle);

    private:
      // Can be implemented using algorithm or LUT table
      static uint16_t _lookupFiringDelay(uint8_t level, float frequency);
      // Can be implemented using algorithm or LUT table
      static float _lookupVrms(uint8_t level, float frequency);
  };
} // namespace Mycila
