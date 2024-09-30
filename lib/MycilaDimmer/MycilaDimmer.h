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

/**
 * Optional resolution, 15bits max
 */
#ifndef MYCILA_DIMMER_RESOLUTION
  #define MYCILA_DIMMER_RESOLUTION 12
#endif

#ifndef MYCILA_DIMMER_MAX_COUNT
  #define MYCILA_DIMMER_MAX_COUNT 1
#endif

namespace Mycila {
  class Dimmer {
    public:
      Dimmer() {}
      ~Dimmer() { end(); }

      /**
       * @brief Enable a dimmer on a specific GPIO pin
       *
       * @param pin: the GPIO pin to use for the dimmer
       *
       * @warning Dimmer won't be enabled if pin is invalid
       * @warning Dimmer won't be activated until the ZCD is enabled
       */
      void begin(int8_t pin, uint32_t semiPeriod);

      /**
       * @brief Disable the dimmer
       *
       * @warning Dimmer won't be destroyed but won't turn on anymore even is a duty cycle is set.
       */
      void end();

      /**
       * @brief Check if the dimmer is enabled
       */
      bool isEnabled() const { return _enabled; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

      /**
       * @brief Get the semi-period of the dimmer in us
       */
      uint32_t getSemiPeriod() const { return _semiPeriod; }

#ifdef MYCILA_JSON_SUPPORT
      /**
       * @brief Serialize Dimmer information to a JSON object
       *
       * @param root: the JSON object to serialize to
       */
      void dimmerToJson(const JsonObject& root) const {
        const float angle = getPhaseAngle();
        root["enabled"] = _enabled;
        root["state"] = isOn() ? "on" : "off";
        root["angle_d"] = angle * RAD_TO_DEG;
        root["angle"] = angle;
        root["delay"] = _delay;
        root["duty_cycle"] = _dutyCycle;
        root["duty_cycle_limit"] = _dutyCycleLimit;
        root["duty_cycle_min"] = _dutyCycleMin;
        root["duty_cycle_max"] = _dutyCycleMax;
        root["semi_period"] = _semiPeriod;
      }
#endif

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
      void setDutyCycle(float dutyCycle);

      /**
       * @brief Set the power duty cycle limit of the dimmer. The duty cycle will be clamped to this limit.
       *
       * @param limit: the power duty cycle limit in the range [0.0, 1.0]
       */
      void setDutyCycleLimit(float limit);

      /**
       * @brief Duty remapping (equivalent to Shelly Dimmer remapping feature).
       * Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V.
       *
       * @param min: Set the new "0" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max].
       */
      void setDutyCycleMin(float min);

      /**
       * @brief Duty remapping (equivalent to Shelly Dimmer remapping feature).
       * Useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V.
       *
       * @param max: Set the new "1" value for the power duty cycle. The duty cycle in the range [0.0, 1.0] will be remapped to [min, max].
       */
      void setDutyCycleMax(float max);

      /**
       * @brief Get the power duty cycle of the dimmer
       */
      float getDutyCycle() const { return _enabled ? _dutyCycle : 0; }

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
      uint32_t getFiringDelay() const { return _delay > _semiPeriod ? _semiPeriod : _delay; }

      /**
       * @brief Get the phase angle in radians of the dimmer in the range [0, PI]
       * At 0% power, the phase angle is equal to PI
       * At 100% power, the phase angle is equal to 0
       */
      float getPhaseAngle() const { return _delay >= _semiPeriod ? PI : PI * _delay / _semiPeriod; }

    private:
      bool _enabled = false;
      gpio_num_t _pin = GPIO_NUM_NC;
      uint32_t _semiPeriod = 0;
      float _dutyCycle = 0;
      float _dutyCycleLimit = 1;
      float _dutyCycleMin = 0;
      float _dutyCycleMax = 1;
      uint32_t _delay = UINT32_MAX;

      uint32_t _lookupPhaseDelay(float dutyCycle);

      Thyristor* _dimmer = nullptr;
  };
} // namespace Mycila
