// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"

#define MYCILA_DIMMER_PWM_RESOLUTION 12   // 12 bits resolution => 0-4095 watts
#define MYCILA_DIMMER_PWM_FREQUENCY  1000 // 1 kHz

namespace Mycila {
  class PWMDimmer : public Dimmer {
    public:
      virtual ~PWMDimmer() { end(); }

      /**
       * @brief Set the GPIO pin to use for the dimmer
       */
      void setPin(gpio_num_t pin) { _pin = pin; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

      /**
       * @brief Set the PWM frequency in Hz
       */
      void setFrequency(uint32_t frequency) { this->_frequency = frequency; }

      /**
       * @brief Get the PWM frequency in Hz
       */
      uint32_t getFrequency() const { return _frequency; }

      /**
       * @brief Set the PWM resolution in bits
       */
      void setResolution(uint8_t resolution) { this->_resolution = resolution; }

      /**
       * @brief Get the PWM resolution in bits
       */
      uint8_t getResolution() const { return _resolution; }

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

      virtual const char* type() const { return "pwm"; }

    protected:
      virtual bool apply();

    private:
      gpio_num_t _pin = GPIO_NUM_NC;
      uint32_t _frequency = MYCILA_DIMMER_PWM_FREQUENCY;
      uint8_t _resolution = MYCILA_DIMMER_PWM_RESOLUTION;
  };
} // namespace Mycila
