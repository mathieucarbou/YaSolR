// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaExpiringValue.h>
#include <MycilaRelay.h>
#include <math.h>

#include <cstdio>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <string>

namespace Mycila {
  class RouterOutput {
    public:
      enum class State {
        // output disabled
        OUTPUT_DISABLED = 0,
        // idle
        OUTPUT_IDLE,
        // excess power sent to load
        OUTPUT_ROUTING,
        // full power sent to load through relay (manual trigger)
        OUTPUT_BYPASS_MANUAL,
        // full power sent to load through relay (auto trigger)
        OUTPUT_BYPASS_AUTO
      };

      typedef struct {
          float apparentPower = 0;
          float current = 0;
          float dimmedVoltage = 0;
          uint32_t energy = 0;
          float power = 0;
          float powerFactor = NAN;
          float resistance = NAN;
          float thdi = NAN;
          float voltage = NAN;
      } Metrics;

      typedef struct {
          float calibratedResistance = NAN;
          bool autoDimmer = false;
          uint8_t dimmerTempLimit = 0;
          bool autoBypass = false;
          uint16_t bypassTimeoutSec = 0;
          uint8_t autoStartTemperature = 0;
          uint8_t autoStopTemperature = 0;
          std::string autoStartTime;
          std::string autoStopTime;
          std::string weekDays;
          uint16_t excessPowerLimiter = 0;
          float excessPowerRatio = 0.0f;
      } Config;

      explicit RouterOutput(const char* name) : _name(name) {}

      // dimmer is mandatory
      void setDimmer(Dimmer* dimmer) { _dimmer = dimmer; }
      // Bypass relay is optional
      void setBypassRelay(Relay* relay) { _relay = relay; }

      // output

      State getState() const {
        if (!isDimmerOnline() && !isBypassRelayEnabled())
          return State::OUTPUT_DISABLED;
        if (_autoBypassEnabled)
          return State::OUTPUT_BYPASS_AUTO;
        if (_manualBypassEnabled)
          return State::OUTPUT_BYPASS_MANUAL;
        if (_dimmer->isOn())
          return State::OUTPUT_ROUTING;
        return State::OUTPUT_IDLE;
      }
      const char* getStateName() const;
      const char* getName() const { return _name; }
      bool isOn() const { return isDimmerOn() || isBypassRelayOn(); }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root, float gridVoltage) const {
        root["bypass"] = isBypassOn() ? "on" : "off";
        root["enabled"] = isDimmerOnline();
        root["state"] = getStateName();
        root["elapsed"] = getBypassUptime();
        float t = _temperature.orElse(NAN);
        if (!std::isnan(t)) {
          root["temperature"] = t;
        }

        Metrics* outputMeasurements = new Metrics();
        readMeasurements(*outputMeasurements);
        toJson(root["measurements"].to<JsonObject>(), *outputMeasurements);
        delete outputMeasurements;
        outputMeasurements = nullptr;

        JsonObject metrics = root["metrics"].to<JsonObject>();
        if (_metrics.isPresent()) {
          metrics["enabled"] = true;
          metrics["time"] = _metrics.getLastUpdateTime();
          toJson(metrics, _metrics.get());
        } else {
          metrics["enabled"] = false;
        }

        _dimmer->toJson(root["dimmer"].to<JsonObject>());
        if (_relay)
          _relay->toJson(root["relay"].to<JsonObject>());
      }

      static void toJson(const JsonObject& dest, const Metrics& metrics) {
        if (!std::isnan(metrics.apparentPower))
          dest["apparent_power"] = metrics.apparentPower;
        if (!std::isnan(metrics.current))
          dest["current"] = metrics.current;
        dest["energy"] = metrics.energy;
        if (!std::isnan(metrics.power))
          dest["power"] = metrics.power;
        if (!std::isnan(metrics.powerFactor))
          dest["power_factor"] = metrics.powerFactor;
        if (!std::isnan(metrics.resistance))
          dest["resistance"] = metrics.resistance;
        if (!std::isnan(metrics.thdi))
          dest["thdi"] = metrics.thdi;
        if (!std::isnan(metrics.voltage))
          dest["voltage"] = metrics.voltage;
        if (!std::isnan(metrics.dimmedVoltage))
          dest["voltage_dimmed"] = metrics.dimmedVoltage;
      }
#endif

      // dimmer

      bool isDimmerOnline() const { return _dimmer->isOnline(); }
      bool isDimmerEnabled() const { return _dimmer->isEnabled(); }
      bool isAutoDimmerEnabled() const { return config.autoDimmer && config.calibratedResistance > 0 && !isBypassOn(); }
      bool isDimmerTemperatureLimitReached() const { return config.dimmerTempLimit > 0 && _temperature.orElse(0) >= config.dimmerTempLimit; }
      bool isDimmerOn() const { return _dimmer->isOn(); }
      float getDimmerDutyCycle() const { return _dimmer->getDutyCycle(); }
      float getDimmerDutyCycleOnline() const { return _dimmer->isOnline() ? _dimmer->getDutyCycle() : 0; }
      float getDimmerDutyCycleLimit() const { return _dimmer->getDutyCycleLimit(); }
      // Power Duty Cycle [0, 1]
      // At 0% power, duty == 0
      // At 100% power, duty == 1
      bool setDimmerDutyCycle(float dutyCycle);
      bool setDimmerOff() { return setDimmerDutyCycle(0); }
      void setDimmerDutyCycleMin(float min) { _dimmer->setDutyCycleMin(min); }
      void setDimmerDutyCycleMax(float max) { _dimmer->setDutyCycleMax(max); }
      void setDimmerDutyCycleLimit(float limit) { _dimmer->setDutyCycleLimit(limit); }
      void applyTemperatureLimit();

      float autoDivert(float gridVoltage, float availablePowerToDivert) {
        if (!_dimmer->isEnabled() || !isAutoDimmerEnabled()) {
          return 0;
        }

        if (availablePowerToDivert <= 0) {
          _dimmer->off();
          return 0;
        }

        if (isDimmerTemperatureLimitReached()) {
          _dimmer->off();
          return 0;
        }

        if (!_dimmer->isOnline()) {
          return 0;
        }

        // maximum power of the load based on the calibrated resistance value
        const float maxPower = gridVoltage * gridVoltage / config.calibratedResistance;

        if (maxPower == 0) {
          return 0;
        }

        // 1. apply excess power ratio for sharing
        // 2. cap the power to divert to the load
        float powerToDivert = constrain(availablePowerToDivert * config.excessPowerRatio, 0, maxPower);

        // apply the excess power limiter
        if (config.excessPowerLimiter)
          powerToDivert = constrain(powerToDivert, 0, config.excessPowerLimiter);

        // try to apply duty
        _dimmer->setDutyCycle(powerToDivert / maxPower);

        // returns the real used power as per the dimmer state
        return maxPower * powerToDivert;
      }

      // bypass

      bool isBypassRelayEnabled() const { return _relay && _relay->isEnabled(); }
      bool isBypassRelayOn() const { return _relay && _relay->isOn(); }
      bool isAutoBypassEnabled() const { return config.autoBypass; }
      bool isBypassOn() const { return _autoBypassEnabled || _manualBypassEnabled; }
      void setBypass(bool state);
      void setBypassOn() { setBypass(true); }
      void setBypassOff() { setBypass(false); }
      uint32_t getBypassUptime() const { return config.bypassTimeoutSec && _manualBypassTime ? (millis() - _manualBypassTime) / 1000UL : 0; }
      void applyAutoBypass();
      void applyBypassTimeout();
      uint64_t getBypassRelaySwitchCount() const { return _relay ? _relay->getSwitchCount() : 0; }

      // metrics

      ExpiringValue<Metrics>& metrics() { return _metrics; }

      // get PZEM measurements, and returns false if the PZEM is not connected, true if measurements are available
      bool readMeasurements(Metrics& metrics) const {
        if (_metrics.isAbsent())
          return false;
        metrics.voltage = _metrics.get().voltage;
        metrics.energy = _metrics.get().energy;
        if (getState() == State::OUTPUT_ROUTING) {
          metrics.apparentPower = _metrics.get().apparentPower;
          metrics.current = _metrics.get().current;
          metrics.dimmedVoltage = _metrics.get().dimmedVoltage;
          metrics.power = _metrics.get().power;
          metrics.powerFactor = _metrics.get().powerFactor;
          metrics.resistance = _metrics.get().resistance;
          metrics.thdi = _metrics.get().thdi;
        }
        return true;
      }

      bool calculateHarmonics(float* array, size_t n) const {
        if (array == nullptr || n == 0)
          return false;

        if (getState() != State::OUTPUT_ROUTING) {
          // Initialize all values to 0
          for (size_t i = 0; i < n; i++) {
            array[i] = 0.0f;
          }
          return true;
        }

        return _dimmer->calculateHarmonics(array, n);
      }

      // temperature

      ExpiringValue<float>& temperature() { return _temperature; }
      const ExpiringValue<float>& temperature() const { return _temperature; }

    public:
      Config config;

    private:
      const char* _name;
      Dimmer* _dimmer;
      Relay* _relay = nullptr; // optional
      bool _autoBypassEnabled = false;
      bool _manualBypassEnabled = false;
      uint32_t _manualBypassTime = 0;
      ExpiringValue<float> _temperature;
      ExpiringValue<Metrics> _metrics;

    private:
      void _switchBypass(bool state, bool log = true);
  };
} // namespace Mycila
