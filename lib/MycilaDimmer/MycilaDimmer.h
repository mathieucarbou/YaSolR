// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <esp32-hal-gpio.h>
#include <thyristor.h>

namespace Mycila {
  class Dimmer {
    public:
      virtual ~Dimmer() {};

      virtual void begin() = 0;
      virtual void end() = 0;

      virtual bool isOnline() const = 0;

      /**
       * @brief Check if the dimmer is enabled
       */
      bool isEnabled() const { return _enabled; }

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
      bool isOff() const { return getDutyCycle() <= getDutyCycleMin(); }

      /**
       * @brief Check if the dimmer is on
       */
      bool isOn() const { return getDutyCycle() > getDutyCycleMin(); }

      /**
       * @brief Check if the dimmer is on at full power
       */
      bool isOnAtFullPower() const { return getDutyCycle() >= getDutyCycleMax(); }

      /**
       * @brief Set the power duty
       *
       * @param dutyCycle: the power duty cycle in the range [0.0, 1.0]
       */
      void setDutyCycle(float dutyCycle) {
        // apply limit and save the wanted duty cycle.
        // it wil only be applied when dimmer will be on
        _dutyCycle = constrain(dutyCycle, 0, _dutyCycleLimit);
        if (_enabled) {
          // duty remapping (equivalent to Shelly Dimmer remapping feature)
          _applyDutyCycle(_dutyCycleMin + _dutyCycle * (_dutyCycleMax - _dutyCycleMin));
        }
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
       * @brief Get the power duty cycle configured for the dimmer
       */
      float getDutyCycle() const { return _dutyCycle; }

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

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      virtual void toJson(const JsonObject& root) const {
        root["enabled"] = isEnabled();
        root["online"] = isOnline();
        root["state"] = isOn() ? "on" : "off";
        root["duty_cycle"] = getDutyCycle();
        root["duty_cycle_limit"] = getDutyCycleLimit();
        root["duty_cycle_min"] = getDutyCycleMin();
        root["duty_cycle_max"] = getDutyCycleMax();
      }
#endif

    protected:
      bool _enabled = false;
      float _dutyCycle = 0;
      float _dutyCycleLimit = 1;
      float _dutyCycleMin = 0;
      float _dutyCycleMax = 1;
      virtual void _applyDutyCycle(float mappedDutyCycle) = 0;
  };

  class VirtualDimmer : public Dimmer {
    public:
      virtual ~VirtualDimmer() { end(); }

      virtual void begin() { _enabled = true; }
      virtual void end() { _enabled = false; }
      virtual bool isOnline() const { return _enabled && _online; }
      void setOnline(bool online) { _online = online; }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      virtual void toJson(const JsonObject& root) const {
        // call parent
        Dimmer::toJson(root);
        root["type"] = "virtual";
      }
#endif

    protected:
      bool _online = false;
      virtual void _applyDutyCycle(float mappedDutyCycle) {}
  };

  class ZeroCrossDimmer : public Dimmer {
    public:
      virtual ~ZeroCrossDimmer() { end(); }

      /**
       * @brief Set the GPIO pin to use for the dimmer
       */
      void setPin(gpio_num_t pin) { _pin = pin; }

      /**
       * @brief Set the semi-period of the dimmer in us
       */
      void setSemiPeriod(uint16_t semiPeriod) { _semiPeriod = semiPeriod; }

      /**
       * @brief Enable a dimmer on a specific GPIO pin
       *
       * @warning Dimmer won't be enabled if pin is invalid
       * @warning Dimmer won't be activated until the ZCD is enabled
       */
      virtual void begin();

      /**
       * @brief Disable the dimmer
       *
       * @warning Dimmer won't be destroyed but won't turn on anymore even is a duty cycle is set.
       */
      virtual void end();

      virtual bool isOnline() const { return _enabled && _semiPeriod; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

      /**
       * @brief Get the semi-period of the dimmer in us
       */
      uint16_t getSemiPeriod() const { return _semiPeriod; }

      /**
       * @brief Get the firing delay in us of the dimmer in the range [0, semi-period]
       * At 0% power, delay is equal to the semi-period: the dimmer is kept off
       * At 100% power, the delay is 0 us: the dimmer is kept on
       */
      uint16_t getFiringDelay() const { return _delay > _semiPeriod ? _semiPeriod : _delay; }

      /**
       * @brief Get the phase angle in radians of the dimmer in the range [0, PI]
       * At 0% power, the phase angle is equal to PI
       * At 100% power, the phase angle is equal to 0
       */
      float getPhaseAngle() const { return _delay >= _semiPeriod ? PI : PI * _delay / _semiPeriod; }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      virtual void toJson(const JsonObject& root) const {
        // call parent
        Dimmer::toJson(root);
        const float angle = getPhaseAngle();
        root["angle_d"] = angle * RAD_TO_DEG;
        root["angle"] = angle;
        root["delay"] = getFiringDelay();
        root["semi_period"] = getSemiPeriod();
        root["type"] = "triac";
      }
#endif

      /**
       * Callback to be called when a zero-crossing event is detected.
       *
       * - When using MycilaPulseAnalyzer library, this callback can be registered like this:
       *
       * pulseAnalyzer.onZeroCross(Mycila::Dimmer::onZeroCross);
       *
       * - When using your own ISR with the Robodyn ZCD,       you can call this method with delayUntilZero == 200 since the length of the ZCD pulse is about  400 us.
       * - When using your own ISR with the ZCd from Daniel S, you can call this method with delayUntilZero == 550 since the length of the ZCD pulse is about 1100 us.
       */
      static void onZeroCross(int16_t delayUntilZero, void* args);

    protected:
      virtual void _applyDutyCycle(float mappedDutyCycle);

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint16_t _semiPeriod = 0;
      uint16_t _delay = UINT16_MAX; // this is the next firing delay to apply

      uint16_t _lookupPhaseDelay(float dutyCycle);

      Thyristor* _dimmer = nullptr;
  };
} // namespace Mycila
