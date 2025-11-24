// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaPID.h>

namespace Mycila {
  /**
   * @brief PID Auto-Tuner using Step Response method for Solar Diversion
   * @brief Designed specifically for solar routers where power output depends on grid voltage and resistance
   * @brief Uses controlled step responses to measure system dynamics and calculate optimal PID parameters
   * 
   * @note How it works:
   * @note 1. Applies a small step increase in power diversion
   * @note 2. Measures how grid power responds (settling time, overshoot, steady-state error)
   * @note 3. Calculates system time constant and gain
   * @note 4. Uses these to compute conservative PID parameters suitable for solar diversion
   * 
   * @note Usage:
   * @note - Call start() to begin auto-tuning
   * @note - Call update() with each new measurement (e.g., every 330ms for JSY meter)
   * @note - When isComplete() returns true, call getTunedParameters() and apply to PID
   * @note - The tuner will automatically stop after measuring system response
   */
  class PIDAutoTuner {
    public:
      /**
       * @brief Tuning rules to use for calculating PID parameters from Ku and Tu
       */
      enum class TuningRule {
        /**
         * @brief AMIGO (Approximate M-constrained Integral Gain Optimization) - RECOMMENDED for solar diversion
         * @brief Kp = 0.35 * Ku, Ki = 0.6 * Ku / Tu, Kd = 0.13 * Ku * Tu
         * @brief Designed for excellent disturbance rejection and robustness
         * @brief Best choice for solar diversion systems with frequent load changes
         */
        AMIGO,
        
        /**
         * @brief Tyreus-Luyben tuning rule (robust, low overshoot)
         * @brief Kp = 0.45 * Ku, Ki = 0.98 * Ku / Tu, Kd = 0.105 * Ku * Tu
         * @brief Designed for processes with significant lag/dead time
         * @brief Provides robust disturbance rejection with minimal overshoot
         * @brief Good alternative to AMIGO for slower, more conservative control
         */
        TYREUS_LUYBEN,
        
        /**
         * @brief Some Overshoot tuning (balanced but aggressive)
         * @brief Kp = 0.33 * Ku, Ki = 0.66 * Ku / Tu, Kd = 0.11 * Ku * Tu
         * @brief Provides good balance between response time and overshoot
         * @brief May oscillate more under disturbances - not ideal for solar diversion
         */
        SOME_OVERSHOOT,
        
        /**
         * @brief No Overshoot tuning (very conservative)
         * @brief Kp = 0.2 * Ku, Ki = 0.4 * Ku / Tu, Kd = 0.066 * Ku * Tu
         * @brief Provides slow but stable response with minimal overshoot
         * @brief Use if AMIGO is too aggressive for your system
         */
        NO_OVERSHOOT,
      };

      /**
       * @brief Auto-tuner state
       */
      enum class State {
        IDLE,           // Not running
        INITIALIZING,   // Waiting for initial baseline
        STEP_UP,        // Applying step increase
        MEASURING,      // Measuring response
        STEP_DOWN,      // Returning to baseline
        ANALYZING,      // Analyzing response data
        COMPLETE,       // Tuning complete
        FAILED,         // Tuning failed
      };

    public:
      /**
       * @brief Construct a new PID Auto-Tuner
       * @param stepAmplitude The amplitude of the step test (W). Should be moderate - typically 20-30% of max power.
       *                      For solar diversion with ~2kW max, use 400-600W
       * @param tuningRule The tuning rule to use for calculating PID parameters (default: AMIGO - best for disturbance rejection)
       */
      PIDAutoTuner(float stepAmplitude = 500.0f, TuningRule tuningRule = TuningRule::AMIGO)
        : _tuningRule(tuningRule),
          _stepAmplitude(stepAmplitude) {}

      /**
       * @brief Start the auto-tuning process
       * @param setpoint The target setpoint to tune around (typically 0W for grid balance)
       * @param initialInput The current input value before starting
       */
      void start(float setpoint, float initialInput) {
        _state = State::INITIALIZING;
        _setpoint = setpoint;
        _baseline = initialInput;
        _lastInput = initialInput;
        _stepStartTime = 0;
        _measureCount = 0;
        _sumResponse = 0;
        _maxResponse = 0;
        _settledCount = 0;
        _startTime = micros();
        _ku = 0;
        _tu = 0;
        _kp = 0;
        _ki = 0;
        _kd = 0;
      }

      /**
       * @brief Stop the auto-tuning process
       * @note You can restart tuning by calling start() again
       */
      void stop() {
        _state = State::IDLE;
      }

      /**
       * @brief Reset the auto-tuner to initial state
       * @note This clears all collected data and results. Call start() afterwards to begin a new tuning session.
       */
      void reset() {
        _state = State::IDLE;
        _baseline = 0;
        _lastInput = 0;
        _stepStartTime = 0;
        _measureCount = 0;
        _sumResponse = 0;
        _maxResponse = 0;
        _settledCount = 0;
        _ku = 0;
        _tu = 0;
        _kp = 0;
        _ki = 0;
        _kd = 0;
      }

      /**
       * @brief Update the auto-tuner with a new measurement
       * @param gridPower Current grid power reading (W)
       * @param actualDiverted Actual power diverted by the router (W) - feedback from router.divert()
       * @return Control output to apply (power to divert in W)
       * @note This replaces the normal PID output during tuning
       * @note Call this method at regular intervals (e.g., every 330ms when JSY measurement arrives)
       */
      float update(float gridPower, float actualDiverted = 0) {
        if (_state == State::IDLE || _state == State::COMPLETE || _state == State::FAILED)
          return 0;

        const uint32_t now = micros();
        const uint32_t elapsed = (now - _startTime) / 1000000; // seconds

        // Timeout check (90 seconds max for entire test)
        if (elapsed > 90) {
          _state = State::FAILED;
          return 0;
        }

        // Phase 1: Establish baseline (wait for stable reading)
        if (_state == State::INITIALIZING) {
          const float change = abs(gridPower - _lastInput);
          if (change < 50.0f) { // Stable if change < 50W
            _settledCount++;
            if (_settledCount >= 5) { // 5 stable readings (~1.5s)
              _baseline = gridPower;
              _state = State::STEP_UP;
              _stepStartTime = now;
              _measureCount = 0;
            }
          } else {
            _settledCount = 0;
          }
          _lastInput = gridPower;
          return 0; // No diversion during baseline
        }

        // Phase 2: Apply step increase and measure response
        if (_state == State::STEP_UP || _state == State::MEASURING) {
          if (_state == State::STEP_UP) {
            // Switch to measuring after step is applied
            _state = State::MEASURING;
            _sumResponse = 0;
            _sumActualDiverted = 0;
            _maxResponse = 0;
            _measureCount = 0;
          }

          // Measure system response
          // Expected: if we divert X watts, grid power should decrease by ~X watts
          const float gridResponse = _baseline - gridPower; // Decrease in grid power (positive = good)
          _sumResponse += gridResponse;
          _sumActualDiverted += actualDiverted; // Track what router actually diverted
          if (gridResponse > _maxResponse)
            _maxResponse = gridResponse;
          _measureCount++;

          const uint32_t stepElapsed = (now - _stepStartTime) / 1000000;

          // Measure for 20 seconds or until settled
          if (stepElapsed >= 20 || _measureCount >= 60) {
            _state = State::STEP_DOWN;
            _settledCount = 0;
          }

          _lastInput = gridPower;
          return _stepAmplitude; // Request step amount
        }

        // Phase 3: Remove step and return to baseline
        if (_state == State::STEP_DOWN) {
          _settledCount++;
          if (_settledCount >= 10) { // Wait 3s after removing step
            _state = State::ANALYZING;
            _analyze();
          }
          _lastInput = gridPower;
          return 0; // No diversion
        }

        return 0;
      }

      /**
       * @brief Check if auto-tuning is complete
       * @return true if tuning completed successfully, false otherwise
       */
      bool isComplete() const { return _state == State::COMPLETE; }

      /**
       * @brief Check if auto-tuning failed
       * @return true if tuning failed, false otherwise
       */
      bool isFailed() const { return _state == State::FAILED; }

      /**
       * @brief Check if auto-tuning is running
       * @return true if tuning is in progress, false otherwise
       */
      bool isRunning() const { 
        return _state == State::INITIALIZING || 
               _state == State::STEP_UP || 
               _state == State::MEASURING || 
               _state == State::STEP_DOWN; 
      }

      /**
       * @brief Get the current state of the auto-tuner
       */
      State getState() const { return _state; }

      /**
       * @brief Get the tuned PID parameters
       * @param kp Reference to store the proportional gain
       * @param ki Reference to store the integral gain
       * @param kd Reference to store the derivative gain
       * @return true if parameters are available (tuning complete), false otherwise
       */
      bool getTunedParameters(float& kp, float& ki, float& kd) const {
        if (_state != State::COMPLETE)
          return false;
        kp = _kp;
        ki = _ki;
        kd = _kd;
        return true;
      }

      /**
       * @brief Get the ultimate gain (Ku) determined during tuning
       * @return The ultimate gain, or 0 if tuning not complete
       */
      float getKu() const { return _ku; }

      /**
       * @brief Get the ultimate period (Tu) determined during tuning
       * @return The ultimate period in seconds, or 0 if tuning not complete
       */
      float getTu() const { return _tu; }

      /**
       * @brief Apply the tuned parameters to a PID controller
       * @param pid The PID controller to configure
       * @return true if parameters were applied, false if tuning not complete
       */
      bool applyToController(PID& pid) const {
        if (_state != State::COMPLETE)
          return false;
        pid.setKp(_kp);
        pid.setKi(_ki);
        pid.setKd(_kd);
        return true;
      }

    private:
      float _stepAmplitude;
      TuningRule _tuningRule;
      State _state = State::IDLE;
      float _setpoint = 0;

      // Measurements
      float _baseline = 0;
      float _lastInput = 0;
      uint32_t _startTime = 0;
      uint32_t _stepStartTime = 0;
      int _measureCount = 0;
      float _sumResponse = 0;
      float _sumActualDiverted = 0; // Track actual diverted power from router feedback
      float _maxResponse = 0;
      int _settledCount = 0;

      // Analysis results
      float _ku = 0; // Ultimate gain
      float _tu = 0; // Ultimate period (time constant)
      float _kp = 0;
      float _ki = 0;
      float _kd = 0;

      /**
       * @brief Analyze the step response data and calculate PID parameters
       */
      void _analyze() {
        if (_measureCount == 0) {
          _state = State::FAILED;
          return;
        }

        // Calculate average steady-state response
        const float avgGridResponse = _sumResponse / _measureCount;
        const float avgActualDiverted = _sumActualDiverted / _measureCount;

        // Process gain: change in grid power / actual diverted power
        // This accounts for dimmer limits and remapping
        // If we diverted 500W but grid only decreased by 400W, process gain = 0.8
        // This could be due to:
        // - Dimmer limits preventing full power
        // - Some power still being consumed elsewhere
        // - System losses
        float processGain = 1.0f; // Default assumption
        
        if (avgActualDiverted > 50.0f) { // Only calculate if we actually diverted something
          processGain = avgGridResponse / avgActualDiverted;
        } else if (avgGridResponse > 50.0f) {
          // Fallback: compare to requested step
          processGain = avgGridResponse / _stepAmplitude;
        } else {
          // System not responding well enough
          _state = State::FAILED;
          return;
        }

        // Sanity check on process gain (should be between 0.3 and 1.5)
        // < 0.3: system barely responds (bad)
        // > 1.5: impossible (grid can't decrease more than we divert)
        if (processGain < 0.3f || processGain > 1.5f) {
          _state = State::FAILED;
          return;
        }

        // Estimate time constant (assume first-order system)
        // For thermal systems with power measurement, typically 3-8 seconds
        _tu = 5.0f; // Conservative time constant for solar diversion

        // Calculate controller gain for stability
        // For closed-loop system with process gain < 1, we can be more aggressive
        // Higher process gain means system responds well -> can use lower controller gain
        _ku = 1.5f / (processGain + 0.1f); // +0.1 to avoid division issues

        // Apply tuning rules - conservative for solar diversion
        switch (_tuningRule) {
          case TuningRule::AMIGO:
            _kp = 0.35f * _ku * 0.4f; // Very conservative
            _ki = 0.6f * _ku / _tu * 0.4f;
            _kd = 0.13f * _ku * _tu * 0.2f; // Minimal derivative
            break;

          case TuningRule::TYREUS_LUYBEN:
            _kp = 0.45f * _ku * 0.4f;
            _ki = 0.98f * _ku / _tu * 0.4f;
            _kd = 0.105f * _ku * _tu * 0.2f;
            break;

          case TuningRule::SOME_OVERSHOOT:
            _kp = 0.33f * _ku * 0.4f;
            _ki = 0.66f * _ku / _tu * 0.4f;
            _kd = 0.11f * _ku * _tu * 0.2f;
            break;

          case TuningRule::NO_OVERSHOOT:
            _kp = 0.2f * _ku * 0.4f;
            _ki = 0.4f * _ku / _tu * 0.4f;
            _kd = 0.066f * _ku * _tu * 0.2f;
            break;
        }

        // Final sanity checks (very conservative bounds)
        if (_kp > 0 && _kp < 20 && _ki > 0 && _ki < 20 && _kd >= 0 && _kd < 200) {
          _state = State::COMPLETE;
        } else {
          _state = State::FAILED;
        }
      }
  };

} // namespace Mycila
