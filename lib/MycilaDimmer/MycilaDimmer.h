// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <esp32-hal-gpio.h>

namespace Mycila {
  class Dimmer {
    public:
      virtual ~Dimmer() {};

      virtual void begin() = 0;
      virtual void end() = 0;
      virtual const char* type() const = 0;

      /**
       * @brief Set the semi-period of the dimmer in us
       */
      void setSemiPeriod(uint16_t semiPeriod) { _semiPeriod = semiPeriod; }

      /**
       * @brief Get the semi-period of the dimmer in us
       */
      uint16_t getSemiPeriod() const { return _semiPeriod; }

      /**
       * @brief Check if the dimmer is enabled
       */
      bool isEnabled() const { return _enabled; }

      /**
       * @brief Returns true if the dimmer has a valid semi-period and can be used (connected to the grid)
       */
      bool isOnline() const { return _enabled && _semiPeriod; }

      /**
       * @brief Turn on the dimmer at full power
       */
      void on() { setDutyCycle(1); }

      /**
       * @brief Turn off the dimmer
       */
      void off() { setDutyCycle(0); }

      /**
       * @brief Check if the dimmer is off
       */
      bool isOff() const { return !isOn(); }

      /**
       * @brief Check if the dimmer is on
       */
      bool isOn() const { return _enabled && _semiPeriod && _dutyCycle; }

      /**
       * @brief Check if the dimmer is on at full power
       */
      bool isOnAtFullPower() const { return getDutyCycle() >= getDutyCycleMax(); }

      /**
       * @brief Set the power duty
       *
       * @param dutyCycle: the power duty cycle in the range [0.0, 1.0]
       */
      bool setDutyCycle(float dutyCycle) {
        // Apply limit and save the wanted duty cycle.
        // It will only be applied when dimmer will be on.
        _dutyCycle = constrain(dutyCycle, 0, _dutyCycleLimit);

        if (!isOnline() && _dutyCycle) {
          // when not connected to the grid, we only allow setting dimmer off
          return false;
        }

        _delay = _lookupFiringDelay(getMappedDutyCycle());

        return apply();
      }

      /**
       * @brief Set the power duty cycle limit of the dimmer. The duty cycle will be clamped to this limit.
       *
       * @param limit: the power duty cycle limit in the range [0.0, 1.0]
       */
      void setDutyCycleLimit(float limit) {
        _dutyCycleLimit = constrain(limit, 0, 1);
        if (_dutyCycle > _dutyCycleLimit)
          setDutyCycle(_dutyCycleLimit);
      }

      /**
       * @brief Duty remapping (equivalent to Shelly Dimmer remapping feature).
       * Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V.
       *
       * @param min: Set the new "0" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max].
       */
      void setDutyCycleMin(float min) {
        _dutyCycleMin = constrain(min, 0, _dutyCycleMax);
        setDutyCycle(_dutyCycle);
      }

      /**
       * @brief Duty remapping (equivalent to Shelly Dimmer remapping feature).
       * Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V.
       *
       * @param max: Set the new "1" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max].
       */
      void setDutyCycleMax(float max) {
        _dutyCycleMax = constrain(max, _dutyCycleMin, 1);
        setDutyCycle(_dutyCycle);
      }

      /**
       * @brief Get the power duty cycle configured for the dimmer by teh user
       */
      float getDutyCycle() const { return _dutyCycle; }

      /**
       * @brief Get the remapped power duty cycle in effect for the dimmer. This is the value that is actually applied to the dimmer.
       */
      float getMappedDutyCycle() const { return _dutyCycleMin + _dutyCycle * (_dutyCycleMax - _dutyCycleMin); }

      /**
       * @brief Get the runtime (live value) of the power duty cycle in effect for the dimmer. This is the value that is actually applied to the dimmer: if the dimmer is disabled, this value will be 0.
       */
      float getDutyCycleLive() const { return _enabled ? _dutyCycle : 0; }

      /**
       * @brief Get the power duty cycle limit of the dimmer
       */
      float getDutyCycleLimit() const { return _dutyCycleLimit; }

      /**
       * @brief Get the remapped "0" of the dimmer duty cycle
       */
      float getDutyCycleMin() const { return _dutyCycleMin; }

      /**
       * @brief Get the remapped "1" of the dimmer duty cycle
       */
      float getDutyCycleMax() const { return _dutyCycleMax; }

      /**
       * @brief Get the firing delay in us of the dimmer in the range [0, semi-period]
       * At 0% power, delay is equal to the semi-period: the dimmer is kept off
       * At 100% power, the delay is 0 us: the dimmer is kept on
       */
      uint16_t getFiringDelay() const { return _delay > _semiPeriod ? _semiPeriod : _delay; }

      /**
       * @brief Get the firing ratio of the dimmer in the range [0, 1]
       * At 0% power, the ratio is equal to 0.
       * At 100% power, the ratio is equal to 1.
       * This is the ratio of the time the dimmer is on compared to the semi-period.
       */
      float getFiringRatio() const {
        if (_semiPeriod == 0 || _delay >= _semiPeriod)
          return 0.0f;
        if (_delay == 0)
          return 1.0f;
        return 1.0f - static_cast<float>(_delay) / static_cast<float>(_semiPeriod);
      }

      /**
       * @brief Get the phase angle in degrees (°) of the dimmer in the range [0, 180]
       * At 0% power, the phase angle is equal to 180
       * At 100% power, the phase angle is equal to 0
       */
      float getPhaseAngle() const { return _delay >= _semiPeriod ? 180 : 180 * _delay / _semiPeriod; }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      void toJson(const JsonObject& root) const {
        root["type"] = type();
        root["enabled"] = isEnabled();
        root["online"] = isOnline();
        root["semi_period"] = getSemiPeriod();
        root["state"] = isOn() ? "on" : "off";
        root["duty_cycle"] = getDutyCycle();
        root["duty_cycle_mapped"] = getMappedDutyCycle();
        root["duty_cycle_limit"] = getDutyCycleLimit();
        root["duty_cycle_min"] = getDutyCycleMin();
        root["duty_cycle_max"] = getDutyCycleMax();
        root["angle"] = getPhaseAngle();
        root["firing_delay"] = getFiringDelay();
        root["firing_ratio"] = getFiringRatio();
      }
#endif

    protected:
      bool _enabled = false;
      float _dutyCycle = 0;
      float _dutyCycleLimit = 1;
      float _dutyCycleMin = 0;
      float _dutyCycleMax = 1;
      uint16_t _semiPeriod = 0;
      uint16_t _delay = UINT16_MAX; // this is the next firing delay to apply

      uint16_t _lookupFiringDelay(float dutyCycle);

      virtual bool apply() = 0;
  };

  class VirtualDimmer : public Dimmer {
    public:
      virtual ~VirtualDimmer() { end(); }

      virtual void begin() { _enabled = true; }
      virtual void end() { _enabled = false; }
      virtual const char* type() const { return "virtual"; }

    protected:
      virtual bool apply() { return true; }
  };
} // namespace Mycila
