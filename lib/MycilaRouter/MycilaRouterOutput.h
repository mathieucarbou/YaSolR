// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaDS18.h>
#include <MycilaDimmer.h>
#include <MycilaGrid.h>
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
      bool connected = false;
      float apparentPower = 0;
      float current = 0;
      float dimmedVoltage = 0;
      float energy = 0;
      float nominalPower = 0;
      float power = 0;
      float powerFactor = 0;
      float thdi = 0;
      float voltage = 0;
  } RouterOutputMetrics;

  typedef std::function<void()> RouterOutputStateCallback;

  class RouterOutput {
    public:
      typedef struct {
          float resistance = 0;
          bool autoDimmer = false;
          uint16_t dimmerLimit = MYCILA_DIMMER_MAX_LEVEL;
          bool autoBypass = false;
          uint8_t autoStartTemperature = 0;
          uint8_t autoStopTemperature = 0;
          String autoStartTime;
          String autoStopTime;
          String weekDays;
      } Config;

      RouterOutput(const char* name,
                   Dimmer& dimmer,
                   Relay& relay,
                   DS18& temperatureSensor,
                   Grid& grid,
                   PZEM& pzem) : _name(name),
                                 _dimmer(&dimmer),
                                 _relay(&relay),
                                 _temperatureSensor(&temperatureSensor),
                                 _grid(&grid),
                                 _pzem(&pzem) {}
      // output

      RouterOutputState getState() const {
        if (!_dimmer->isEnabled() && !_relay->isEnabled())
          return RouterOutputState::OUTPUT_DISABLED;
        if (_autoBypassEnabled)
          return RouterOutputState::OUTPUT_BYPASS_AUTO;
        if (_bypassEnabled)
          return RouterOutputState::OUTPUT_BYPASS_MANUAL;
        if (_dimmer->getPowerDuty() > 0)
          return RouterOutputState::OUTPUT_ROUTING;
        return RouterOutputState::OUTPUT_IDLE;
      }
      const char* getStateName() const;
      const char* getName() const { return _name; }

      bool isEnabled() const { return _dimmer->isEnabled(); }
      void listen(RouterOutputStateCallback callback) { _callback = callback; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        RouterOutputMetrics metrics;
        getMetrics(metrics);

        root["enabled"] = isEnabled();
        root["state"] = getStateName();
        root["bypass"] = isBypassOn() ? "on" : "off";
        root["online"] = metrics.connected;
        root["resistance"] = config.resistance;

        _dimmer->toJson(root["dimmer"].to<JsonObject>());
        _temperatureSensor->toJson(root["ds18"].to<JsonObject>());

        JsonObject jsonMetrics = root["metrics"].to<JsonObject>();
        jsonMetrics["apparent_power"] = metrics.apparentPower;
        jsonMetrics["current"] = metrics.current;
        jsonMetrics["energy"] = metrics.energy;
        jsonMetrics["power"] = metrics.power;
        jsonMetrics["power_factor"] = metrics.powerFactor;
        jsonMetrics["thdi"] = metrics.thdi;
        jsonMetrics["voltage"] = metrics.voltage;
        jsonMetrics["voltage_dimmed"] = metrics.dimmedVoltage;

        jsonMetrics["pzem"]["apparent_power"] = _pzem->getApparentPower();
        jsonMetrics["pzem"]["current"] = _pzem->getCurrent();
        jsonMetrics["pzem"]["power"] = _pzem->getPower();
        jsonMetrics["pzem"]["power_factor"] = _pzem->getPowerFactor();
        jsonMetrics["pzem"]["thdi"] = _pzem->getTHDi(0);
        jsonMetrics["pzem"]["voltage"] = _pzem->getVoltage();
        jsonMetrics["pzem"]["voltage_dimmed"] = _pzem->getPower() / _pzem->getCurrent();

        _relay->toJson(root["relay"].to<JsonObject>());
      }
#endif

      // dimmer

      bool isDimmerEnabled() const { return _dimmer->isEnabled(); }
      bool isAutoDimmerEnabled() const { return _dimmer->isEnabled() && config.autoDimmer; }
      uint16_t getDimmerDuty() const { return _dimmer->getPowerDuty(); }
      // Power Duty Cycle [0, MYCILA_DIMMER_MAX_LEVEL]
      bool tryDimmerDuty(uint16_t duty);
      // Power Duty Cycle [0, 1]
      // At 0% power, duty == 0
      // At 100% power, duty == 1
      void tryDimmerDutyCycle(float dutyCycle) { tryDimmerDuty(dutyCycle * MYCILA_DIMMER_MAX_LEVEL); }
      void applyDimmerLimit();

      // bypass

      bool isBypassEnabled() const { return _relay->isEnabled() || _dimmer->isEnabled(); }
      bool isAutoBypassEnabled() const { return isBypassEnabled() && config.autoBypass; }
      bool isBypassOn() const { return _bypassEnabled; }
      bool tryBypassState(bool state);
      void applyAutoBypass();

      // metrics

      void getMetrics(RouterOutputMetrics& metrics) const {
        metrics.voltage = _pzem->isConnected() ? _pzem->getVoltage() : _grid->getVoltage();
        metrics.connected = metrics.voltage > 0;
        metrics.energy = _pzem->getEnergy();
        metrics.nominalPower = config.resistance == 0 ? 0 : metrics.voltage * metrics.voltage / config.resistance;
        if (getState() == RouterOutputState::OUTPUT_ROUTING) {
          float dutyCycle = _dimmer->getPowerDutyCycle();
          metrics.power = dutyCycle * metrics.nominalPower;
          metrics.powerFactor = sqrt(dutyCycle);
          metrics.dimmedVoltage = metrics.powerFactor * metrics.voltage;
          metrics.current = config.resistance == 0 ? 0 : metrics.dimmedVoltage / config.resistance;
          metrics.apparentPower = metrics.current * metrics.voltage;
          metrics.thdi = dutyCycle == 0 ? 0 : sqrt(1 / dutyCycle - 1);
        }
      }

    public:
      Config config;

    private:
      const char* _name;
      Dimmer* _dimmer;
      Relay* _relay;
      Mycila::DS18* _temperatureSensor;
      Grid* _grid;
      PZEM* _pzem;
      bool _autoBypassEnabled = false;
      bool _bypassEnabled = false;
      RouterOutputStateCallback _callback = nullptr;

    private:
      void _setBypass(bool state, uint16_t dimmerDutyWhenRelayOff = 0);
  };
} // namespace Mycila
