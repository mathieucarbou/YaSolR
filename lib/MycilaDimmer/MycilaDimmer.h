// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <WString.h>
#include <esp32-hal-gpio.h>
#include <thyristor.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <functional>

namespace Mycila {
  class Dimmer {
    public:
      ~Dimmer() { end(); }

      void begin(const int8_t pin, const uint16_t semiPeriod);
      void end();

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _dimmer != nullptr; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        const float angle = getPhaseAngle();
        root["enabled"] = _dimmer != nullptr;
        root["state"] = isOn() ? "on" : "off";
        root["angle_d"] = angle * RAD_TO_DEG;
        root["angle"] = angle;
        root["delay"] = getFiringDelay();
        root["duty_cycle"] = _dutyCycle;
        root["duty_cycle_limit"] = _dutyCycleLimit;
        root["duty_cycle_min"] = _dutyCycleMin;
        root["duty_cycle_max"] = _dutyCycleMax;
        root["semi_period"] = _semiPeriod;
      }
#endif

      void on() { setDutyCycle(1); }
      void off() { setDutyCycle(0); }
      bool isOff() const { return _dutyCycle <= _dutyCycleMin; }
      bool isOn() const { return _dutyCycle > _dutyCycleMin; }
      bool isOnAtFullPower() const { return _dutyCycle >= _dutyCycleMax; }

      // Power Duty Cycle [0, 1]
      // At 0% power, duty == 0
      // At 100% power, duty == 1
      void setDutyCycle(float dutyCycle);

      // set the maximum duty [0, 1]
      void setDutyCycleLimit(float limit);

      // Duty remapping (equivalent to Shelly Dimmer remapping feature)
      // remap the duty minimum and maximum values to be a new ones
      // useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V
      void setDutyCycleMin(float min);
      void setDutyCycleMax(float max);

      float getDutyCycle() const { return _dutyCycle; }
      float getDutyCycleLimit() const { return _dutyCycleLimit; }
      float getDutyCycleMin() const { return _dutyCycleMin; }
      float getDutyCycleMax() const { return _dutyCycleMax; }

      // Delay [0, semi-period] us
      // Where semi-period = 1000000 / 2 / frequency (50h: 10000 us, 60Hz: 8333 us)
      // At 0% power, delay is equal to the semi-period
      // At 100% power, the delay is 0 us
      uint16_t getFiringDelay() const { return isEnabled() ? _dimmer->getDelay() : _semiPeriod; }

      // Phase angle [0, PI] rad
      // At 0% power, the phase angle is equal to PI
      // At 100% power, the phase angle is equal to 0
      float getPhaseAngle() const {
        // angle_rad = PI * delay_us / period_us
        return _semiPeriod == 0 ? PI : PI * getFiringDelay() / _semiPeriod;
      }

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      Thyristor* _dimmer = nullptr;
      uint16_t _semiPeriod = 0;
      float _dutyCycleMin = 0;
      float _dutyCycleMax = 1;
      float _dutyCycleLimit = 1;
      float _dutyCycle = 0;
  };
} // namespace Mycila
