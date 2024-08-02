// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include "MycilaDimmer.h"
#include <driver/gptimer_types.h>

namespace Mycila {
  class ZeroCrossDimmer : public Dimmer {
    public:
      virtual ~ZeroCrossDimmer() { end(); }

      /**
       * @brief Set the GPIO pin to use for the dimmer
       */
      void setPin(gpio_num_t pin) { _pin = pin; }

      /**
       * @brief Get the GPIO pin used for the dimmer
       */
      gpio_num_t getPin() const { return _pin; }

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

      virtual const char* type() const { return "zero-cross"; }

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
      virtual bool apply();

    private:
      gpio_num_t _pin = GPIO_NUM_NC;

      static ZeroCrossDimmer* _dimmers[2];
      static size_t _dimmerCount;
      static portMUX_TYPE _spinlock;
      static gptimer_handle_t _fireTimer;

      static bool _registerDimmer(ZeroCrossDimmer* dimmer);
      static void _unregisterDimmer(ZeroCrossDimmer* dimmer);

      // Timer ISR to be called as soon as a dimmer needs to be fired
      static bool _fireTimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t* event, void* arg);
  };
} // namespace Mycila
