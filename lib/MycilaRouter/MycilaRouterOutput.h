// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaExpiringValue.h>
#include <MycilaPZEM004Tv3.h>
#include <MycilaRelay.h>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

namespace Mycila {
  enum class RouterOutputState {
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
      float energy = 0;
      float power = 0;
      float powerFactor = 0;
      float resistance = 0;
      float thdi = 0;
      float voltage = 0;
  } RouterOutputMetrics;

  typedef std::function<void()> RouterOutputStateCallback;

  class RouterOutput {
    public:
      typedef struct {
          float calibratedResistance = 0;
          bool autoDimmer = false;
          uint8_t dimmerTempLimit = 0;
          bool autoBypass = false;
          uint8_t autoStartTemperature = 0;
          uint8_t autoStopTemperature = 0;
          String autoStartTime;
          String autoStopTime;
          String weekDays;
          float reservedExcessPowerRatio = 1;
      } Config;

      RouterOutput(const char* name,
                   Dimmer& dimmer,
                   Relay& relay,
                   PZEM& pzem) : _name(name),
                                 _dimmer(&dimmer),
                                 _relay(&relay),
                                 _pzem(&pzem) {}
      // output

      RouterOutputState getState() const {
        if (!_dimmer->isEnabled() && !_relay->isEnabled())
          return RouterOutputState::OUTPUT_DISABLED;
        if (_autoBypassEnabled)
          return RouterOutputState::OUTPUT_BYPASS_AUTO;
        if (_bypassEnabled)
          return RouterOutputState::OUTPUT_BYPASS_MANUAL;
        if (_dimmer->isOn())
          return RouterOutputState::OUTPUT_ROUTING;
        return RouterOutputState::OUTPUT_IDLE;
      }
      const char* getStateName() const;
      const char* getName() const { return _name; }

      void listen(RouterOutputStateCallback callback) { _callback = callback; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root, float gridVoltage) const {
        root["bypass"] = isBypassOn() ? "on" : "off";
        root["enabled"] = isDimmerEnabled();
        root["state"] = getStateName();
        root["temperature"] = _temperature.orElse(0);

        _dimmer->toJson(root["dimmer"].to<JsonObject>());

        RouterOutputMetrics outputMeasurements;
        getMeasurements(outputMeasurements);
        toJson(root["measurements"].to<JsonObject>(), outputMeasurements);

        RouterOutputMetrics dimmerMetrics;
        getDimmerMetrics(dimmerMetrics, gridVoltage);
        toJson(root["metrics"].to<JsonObject>(), dimmerMetrics);

        _pzem->toJson(root["pzem"].to<JsonObject>());
        _relay->toJson(root["relay"].to<JsonObject>());
      }

      static void toJson(const JsonObject& dest, const RouterOutputMetrics& metrics) {
        dest["apparent_power"] = metrics.apparentPower;
        dest["current"] = metrics.current;
        dest["energy"] = metrics.energy;
        dest["power"] = metrics.power;
        dest["power_factor"] = metrics.powerFactor;
        dest["resistance"] = metrics.resistance;
        dest["thdi"] = metrics.thdi;
        dest["voltage"] = metrics.voltage;
        dest["voltage_dimmed"] = metrics.dimmedVoltage;
      }
#endif

      // dimmer

      bool isDimmerEnabled() const { return _dimmer->isEnabled(); }
      bool isAutoDimmerEnabled() const { return _dimmer->isEnabled() && config.autoDimmer && config.calibratedResistance > 0; }
      bool isDimmerTemperatureLimitReached() const { return config.dimmerTempLimit > 0 && _temperature.orElse(0) >= config.dimmerTempLimit; }
      uint16_t getDimmerDuty() const { return _dimmer->getDuty(); }
      float getDimmerDutyCycle() const { return _dimmer->getDutyCycle(); }
      // Power Duty Cycle [0, MYCILA_DIMMER_MAX_DUTY]
      bool tryDimmerDuty(uint16_t duty);
      // Power Duty Cycle [0, 1]
      // At 0% power, duty == 0
      // At 100% power, duty == 1
      bool tryDimmerDutyCycle(float dutyCycle) { return tryDimmerDuty(dutyCycle * MYCILA_DIMMER_MAX_DUTY); }
      void applyTemperatureLimit();

      float autoDivert(float gridVoltage, float availablePowerToDivert) {
        if (!_dimmer->isEnabled() || _autoBypassEnabled || !config.autoDimmer || config.calibratedResistance <= 0 || isDimmerTemperatureLimitReached()) {
          _dimmer->setOff();
          return 0;
        }

        // maximum power that can be diverted to the load based on the calibrated resistance value
        // match duty == 4095
        const float maxPower = gridVoltage * gridVoltage / config.calibratedResistance;

        // power allowed to be diverted to the load after applying the reserved excess power ratio
        const float reservedPowerToDivert = constrain(availablePowerToDivert * config.reservedExcessPowerRatio, 0, maxPower);

        // convert to a duty
        const uint16_t duty = maxPower == 0 ? 0 : reservedPowerToDivert / maxPower;

        // try to apply duty
        _dimmer->setDuty(duty);

        // returns the used power as per the dimmer state
        return maxPower * _dimmer->getDutyCycle();
      }

      // bypass

      bool isBypassEnabled() const { return _relay->isEnabled() || _dimmer->isEnabled(); }
      bool isAutoBypassEnabled() const { return isBypassEnabled() && config.autoBypass; }
      bool isBypassOn() const { return _bypassEnabled; }
      bool tryBypassState(bool state);
      void applyAutoBypass();

      // metrics

      // get output theoretical metrics based on the dimmer state and the grid voltage
      void getDimmerMetrics(RouterOutputMetrics& metrics, float gridVoltage) const {
        metrics.resistance = config.calibratedResistance;
        metrics.voltage = gridVoltage;
        metrics.energy = _pzem->getEnergy();
        const float dutyCycle = _dimmer->getDutyCycle();
        const float maxPower = metrics.resistance == 0 ? 0 : metrics.voltage * metrics.voltage / metrics.resistance;
        metrics.power = dutyCycle * maxPower;
        metrics.powerFactor = sqrt(dutyCycle);
        metrics.dimmedVoltage = metrics.powerFactor * metrics.voltage;
        metrics.current = metrics.resistance == 0 ? 0 : metrics.dimmedVoltage / metrics.resistance;
        metrics.apparentPower = metrics.current * metrics.voltage;
        metrics.thdi = dutyCycle == 0 ? 0 : sqrt(1 / dutyCycle - 1);
      }

      // get PZEM measurements, and returns false if the PZEM is not connected, true if measurements are available
      bool getMeasurements(RouterOutputMetrics& metrics) const {
        if (!_pzem->isConnected())
          return false;
        metrics.voltage = _pzem->getVoltage();
        metrics.energy = _pzem->getEnergy();
        if (getState() == RouterOutputState::OUTPUT_ROUTING) {
          metrics.apparentPower = _pzem->getApparentPower();
          metrics.current = _pzem->getCurrent();
          metrics.dimmedVoltage = _pzem->getDimmedVoltage();
          metrics.power = _pzem->getPower();
          metrics.powerFactor = _pzem->getPowerFactor();
          metrics.resistance = _pzem->getResistance();
          metrics.thdi = _pzem->getTHDi(0);
        }
        return true;
      }

      // temperature

      ExpiringValue<float>& temperature() { return _temperature; }

    public:
      Config config;

    private:
      const char* _name;
      Dimmer* _dimmer;
      Relay* _relay;
      PZEM* _pzem;
      bool _autoBypassEnabled = false;
      bool _bypassEnabled = false;
      RouterOutputStateCallback _callback = nullptr;
      ExpiringValue<float> _temperature;

    private:
      void _setBypass(bool state, bool log = true);
  };
} // namespace Mycila
