// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaZCD.h>
#include <WString.h>
#include <esp32-hal-gpio.h>
#include <thyristor.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <functional>

#define MYCILA_DIMMER_RESOLUTION 12   // bits
#define MYCILA_DIMMER_MAX_DUTY   4095 // ((1 << MYCILA_DIMMER_RESOLUTION) - 1)

namespace Mycila {
  class Dimmer {
    public:
      explicit Dimmer(ZCD& zcd) : _zcd(&zcd) {}
      ~Dimmer() { end(); }

      void begin(const int8_t pin);
      void end();

      gpio_num_t getPin() const { return _pin; }
      bool isEnabled() const { return _dimmer != nullptr; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        const float angle = getPhaseAngle();
        root["angle"] = angle;
        root["angle_d"] = angle * RAD_TO_DEG;
        root["delay"] = getFiringDelay();
        root["duty"] = _duty;
        root["duty_cycle"] = getDutyCycle();
        root["enabled"] = _dimmer != nullptr;
        root["state"] = isOn() ? "on" : "off";
      }
#endif

      void setOn() { setDuty(MYCILA_DIMMER_MAX_DUTY); }
      void setOff() { setDuty(0); }
      bool isOff() const { return _duty <= _dutyMin; }
      bool isOn() const { return _duty > _dutyMin; }
      bool isOnAtFullPower() const { return _duty >= _dutyMax; }

      // Power Duty Cycle [0, MYCILA_DIMMER_MAX_DUTY]
      void setDuty(uint16_t duty);
      // Power Duty Cycle [0, 1]
      // At 0% power, duty == 0
      // At 100% power, duty == 1
      void setDutyCycle(float dutyCycle) { setDuty(dutyCycle * MYCILA_DIMMER_MAX_DUTY); }

      // set the maximum duty [0, MYCILA_DIMMER_MAX_DUTY]
      void setDutyLimit(uint16_t limit);
      void setDutyCycleLimit(float limit) { setDutyLimit(limit * MYCILA_DIMMER_MAX_DUTY); }

      // Duty remapping (equivalent to Shelly Dimmer remapping feature)
      // remap the duty minimum and maximum values to be a new ones
      // useful to calibrate the dimmer when using for example a PWM signal to 0-10V analog convertor connected to a voltage regulator which is only working in a specific voltage range like 1-8V
      void setDutyMin(uint16_t min);
      void setDutyMax(uint16_t max);
      void setDutyCycleMin(float min) { setDutyMin(min * MYCILA_DIMMER_MAX_DUTY); }
      void setDutyCycleMax(float max) { setDutyMax(max * MYCILA_DIMMER_MAX_DUTY); }

      uint16_t getDuty() const { return _duty; }
      float getDutyCycle() const { return static_cast<float>(_duty) / MYCILA_DIMMER_MAX_DUTY; }
      uint16_t getDutyLimit() const { return _dutyLimit; }
      float getDutyCycleLimit() const { return static_cast<float>(_dutyLimit) / MYCILA_DIMMER_MAX_DUTY; }
      uint16_t getDutyMin() const { return _dutyMin; }
      float getDutyCycleMin() const { return static_cast<float>(_dutyMin) / MYCILA_DIMMER_MAX_DUTY; }
      uint16_t getDutyMax() const { return _dutyMax; }
      float getDutyCycleMax() const { return static_cast<float>(_dutyMax) / MYCILA_DIMMER_MAX_DUTY; }

      // Delay [0, semi-period] us
      // Where semi-period = 1000000 / 2 / frequency (50h: 10000 us, 60Hz: 8333 us)
      // At 0% power, delay is equal to the semi-period
      // At 100% power, the delay is 0 us
      uint16_t getFiringDelay() const { return isEnabled() ? _dimmer->getDelay() : 0; }

      // Phase angle [0, PI] rad
      // At 0% power, the phase angle is equal to PI
      // At 100% power, the phase angle is equal to 0
      float getPhaseAngle() const {
        // angle_rad = PI * delay_us / period_us
        uint16_t semiPeriod = _zcd->getSemiPeriod();
        return semiPeriod == 0 ? PI : PI * getFiringDelay() / semiPeriod;
      }

    private:
      ZCD* _zcd;
      gpio_num_t _pin = GPIO_NUM_NC;
      Thyristor* _dimmer = nullptr;
      uint16_t _dutyMin = 0;
      uint16_t _dutyMax = MYCILA_DIMMER_MAX_DUTY;
      uint16_t _dutyLimit = MYCILA_DIMMER_MAX_DUTY;
      uint16_t _duty = 0;
  };
} // namespace Mycila
