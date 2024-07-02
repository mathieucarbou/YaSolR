/**
 * Low level phase control driver implemented with a Timer Group (TIMG).
 * 
 * 12 bits resolution by default (0 to 4095).
 * Detects automatically the frequency of the grid.
 * The output power is linear (sine square distribution).
 * No limitation on the number of output.
 * The zero crossing input is filtered.
 * Designed for minimum CPU usage and low latency.
 * Uses integers only, no floating points, no FPU lock.
 * 
 * Drawbacks:
 * Interrupts adds a 5us delay between ZC input and output.
 * Interrupts may be delayed under heavy load.
 * 
 * Usage:
 *
 *   #include "triac-timer.hpp"
 *
 *   #define PIN_ZEROCROSS   ( 16 )
 *   #define PIN_TRIAC_1     ( 17 )
 *   #define PIN_TRIAC_2     ( 18 )
 *   #define ZC_DELAY_US     ( 170 )
 *
 *   Triac triac1(PIN_TRIAC_1);
 *   Triac triac2(PIN_TRIAC_2);
 *
 *   void setup()
 *   {
 *      Triac::begin(PIN_ZEROCROSS, ZC_DELAY_US);
 *   }
 *
 *   void loop()
 *   {
 *      triac1.set(100 * TRIAC_MAX / 100);  // 100%
 *      triac2.set(75  * TRIAC_MAX / 100);  // 75%
 *      delay(500);
 *      triac1.set(0   * TRIAC_MAX / 100);  // 0%
 *      triac2.set(25  * TRIAC_MAX / 100);  // 25%
 *      delay(500);
 *   }
 * 
 * TODO:
 * check interrupt priority gpio >= timer and occur on same core
 * check watchdog timer
 * 
 */

#pragma once

#include "hal/gpio_types.h"
#include <stdint.h>
#include <stddef.h>

/**
 * Optional timer, 0-3 for most ESP32, 0-1 for ESP32-C3
 */
#ifndef CONFIG_TRIAC_TIMER_NUM
#define CONFIG_TRIAC_TIMER_NUM  ( 0 )
#endif

/**
 * Optional resolution, 15bits max
 */
#ifndef CONFIG_TRIAC_RESOLUTION
#define CONFIG_TRIAC_RESOLUTION  ( 12 )
#endif


#define TRIAC_MAX  ( (1 << CONFIG_TRIAC_RESOLUTION) - 1 )


struct Triac
{
    Triac*     _next;   // next triac instance
    gpio_num_t _pin;    // output pin number
    uint64_t   _mask;   // output mask
    uint32_t   _delay;  // triac firing delay, us

    volatile uint32_t _alarm;      // alarm tick for pulse start or pulse end
    volatile bool     _triggered;  // ON/OFF flag

    uint32_t value;  // 12 bits duty, 0-4095, readonly


    /**
     *  @brief Constructor
     * 
     *  @param  pin  Output pin number
     */
    Triac(gpio_num_t pin);

    /**
     *  @brief   Set the output duty/power.
     *           Must wait at least 2 seconds after calling Triac::begin 
     * 
     *  @param  value   12 bits from 0 (OFF) to 4095 (Fully ON).
     */
    void set(int32_t value);

    /**
     * @brief   Increase or decrease the output duty/power.
     * 
     * @param  value   12 bits, +/-4095 (+/-100%)
     * @return  Added power within available range.
     */
    int32_t add(int32_t value);

    /**
     *  @brief   Outputs the zero crossing signal for calibration.
     */
    void test_zc();



    /**
     * @brief  Configure and start the phase controller.
     *
     * @param  sync_pin  Zero crossing input pins, GPIO number.
     * @param  delay_us  Zero crossing delay, microseconds.
     * @param  invert    Invert the zero crossing input, false: high pulse, true: low pulse.
     */
    static bool begin(gpio_num_t sync_pin, uint16_t delay_us = 170, bool invert = false);

    /**
     * @brief   Stop the phase controller and free resources.
     */
    static void end();

    /**
     * @brief   Grid period, moving average, microsecond.
     * 
     * @example
     * 
     * uint32_t period_us = Triac::get_period_us();
     * float frequency_hz = period_us ? (1000000.0 / period_us) : 0;
     * 
     */
    static uint32_t get_period_us();

};
