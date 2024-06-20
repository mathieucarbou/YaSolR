/******************************************************************************
 *  This file is part of Dimmable Light for Arduino, a library to control     *
 *  dimmers.                                                                  *
 *                                                                            *
 *  Copyright (C) 2018-2023  Fabiano Riccardi                                 *
 *                                                                            *
 *  Dimmable Light for Arduino is free software; you can redistribute         *
 *  it and/or modify it under the terms of the GNU Lesser General Public      *
 *  License as published by the Free Software Foundation; either              *
 *  version 2.1 of the License, or (at your option) any later version.        *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
 *  Lesser General Public License for more details.                           *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/
#ifndef THYRISTOR_H
#define THYRISTOR_H

#include <Arduino.h>

/**
 * This is the core class of this library, that provides the finest control on thyristors.
 *
 * NOTE: Design Principle for this library: There are 2 main abstraction levels: the first one,
 * represented by Thyristor class, is agnostic about the controlled load (it doesn't assume a lamp,
 * a heater or a motor). The second one provides a simpler and more concrete interface, presenting
 * simplified APIs to the user as expected by an Arduino library, and it is exemplified by
 * DimmableLight class.
 * Now, I'm aware that this is positive because it allows to write very
 * readable code IF the appliance is a light, but it is limiting and weird if the user is going to
 * use another appliance.
 *
 * About this class, the "core" of the library, the name of the method to control a dimmer is
 * setDelay(..) and not, for example, setPower(..), setBrightness(..), ... This gives a precise idea
 * of what's happening at electrical level, that is controlling the activation time of the
 * thyristor. Secondly, the measurement unit is expressed in microseconds, allowing the finest and
 * feasible control reachable with almost any MCU available on the market (including Arduino UNO
 * based on ATmega328p).
 */
class Thyristor {
  public:
    Thyristor(int pin);
    Thyristor(Thyristor const&) = delete;
    void operator=(Thyristor const& t) = delete;

    /**
     * Set the delay, 10000 (ms, with 50Hz voltage) to turn off the thyristor
     */
    void setDelay(uint16_t delay);

    /**
     * Return the current delay.
     */
    uint16_t getDelay() const {
      return delay;
    }

    /**
     * Turn on the thyristor at full power.
     */
    void turnOn() {
      setDelay(0);
    }

    /**
     * Turn off the thyristor.
     */
    void turnOff() {
     setDelay(getSemiPeriod()); 
    }

    ~Thyristor();

    /**
     * Setup timer and interrupt routine.
     */
    static void begin();
    static void end();

    /**
     * Return the number of instantiated thyristors.
     */
    static uint8_t getThyristorNumber() {
      return nThyristors;
    };

    /**
     * Set the pin dedicated to receive the AC zero cross signal.
     */
    static void setSyncPin(uint8_t pin) {
      syncPin = pin;
    }

    /**
     * Set the pin pullup (true = INPUT_PULLUP, false = INPUT). The internal pullup resistor is not
     * available for each platform and each pin.
     */
    static void setSyncPullup(bool pullup) {
      syncPullup = pullup;
    }

    /**
     * Get frequency.
     */
    static float getFrequency();

    /**
     * Get the semiperiod.
     */
    static uint16_t getSemiPeriod();

    /**
     * Set target frequency. Negative values are ignored;
     * zero set the semi-period to 0.
     */
    static void setFrequency(float frequency);

    static uint16_t getPulseWidth();
    static uint16_t getMaxPulseWidth();
    static uint16_t getMinPulseWidth();
    static uint16_t getLastPulseWidth();

    /**
     * Get the detected frequency on the electrical network, constantly updated.
     * Return 0 if there is no signal or while sampling the first periods.
     *
     * NOTE: when (re)starting, it will take a while before returning a value different from 0.
     */
    static float getDetectedFrequency();

    static const uint8_t N = 2;

  private:
    /**
     * Search if all the values are only on and off.
     * Return true if all are on/off, false otherwise.
     */
    bool areThyristorsOnOff();

    /**
     * Number of instantiated thyristors.
     */
    static uint8_t nThyristors;

    /**
     * Vector of instatiated thyristors.
     */
    static Thyristor* thyristors[];

    /**
     * Variable to tell to interrupt routine to update its internal structures
     */
    static bool newDelayValues;

    /**
     * Variable to avoid concurrency problem between interrupt and threads.
     * In particular, this variable is used to prevent the copy of the memory used by
     * the array of struct during reordering (interrupt can continue because it
     * keeps its own copy of the array).
     * A condition variable does not make sense because interrupt routine cannot be
     * stopped.
     */
    static bool updatingStruct;

    /**
     * This variable tells if the thyristors are completely ON and OFF,
     * mixed configuration are included. If one thyristor has a value between
     * (0; semiPeriodLength), this variable is false. If true, this implies that
     * zero cross interrupt must be enabled to manage the thyristor activation.
     */
    static bool allThyristorsOnOff;

    /**
     * Pin receiving the external Zero Cross signal.
     */
    static uint8_t syncPin;

    /**
     * Pin pullup active.
     */
    static bool syncPullup;

    /**
     * 0) no messages
     * 1) error messages
     * 2) debug messages
     * 3) info messages
     */
    static const uint8_t verbosity = 1;

    /**
     * Pin used to control thyristor's gate.
     */
    uint8_t pin;

    /**
     * Position into the static array, this is used to speed up the research
     * operation while setting the new brightness value.
     */
    uint8_t posIntoArray;

    /**
     * Time to wait before turning on the thryristor.
     */
    uint16_t delay;

    friend void activate_thyristors();
    friend void zero_cross_pulse_int();
    friend void zero_cross_int();
    friend void turn_off_gates_int();

  public:
    // Ignore zero-cross interrupts when they occurs too early w.r.t semi-period ideal length.
    // The constant *semiPeriodShrinkMargin* defines the "too early" margin.
    static int semiPeriodShrinkMargin;
    static int semiPeriodExpandMargin;
};

#endif // END THYRISTOR_H
