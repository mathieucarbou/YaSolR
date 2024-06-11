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
          bool autoDimmer;
          uint16_t dimmerLimit;
          bool autoBypass;
          uint8_t autoStartTemperature;
          uint8_t autoStopTemperature;
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
        if (_dimmer->getLevel() > 0)
          return RouterOutputState::OUTPUT_ROUTING;
        return RouterOutputState::OUTPUT_IDLE;
      }
      const char* getStateName() const;

      bool isEnabled() const { return _dimmer->isEnabled(); }
      void listen(RouterOutputStateCallback callback) { _callback = callback; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        RouterOutputMetrics metrics;
        getMetrics(metrics);
        root["enabled"] = isEnabled();
        root["online"] = metrics.connected;
        root["state"] = getStateName();
        root["metrics"]["apparent_power"] = metrics.apparentPower;
        root["metrics"]["current"] = metrics.current;
        root["metrics"]["energy"] = metrics.energy;
        root["metrics"]["power"] = metrics.power;
        root["metrics"]["power_factor"] = metrics.powerFactor;
        root["metrics"]["resistance"] = metrics.resistance;
        root["metrics"]["thdi"] = metrics.thdi;
        root["metrics"]["voltage"] = metrics.voltage;
        root["metrics"]["voltage_dimmed"] = metrics.dimmedVoltage;
      }
#endif

      // dimmer

      bool isDimmerEnabled() const { return _dimmer->isEnabled(); }
      bool isAutoDimmerEnabled() const { return _dimmer->isEnabled() && config.autoDimmer; }
      uint16_t getDimmerLevel() const { return _dimmer->getLevel(); }
      bool tryDimmerLevel(uint16_t level);
      void applyDimmerLimit();

      // bypass

      bool isBypassEnabled() const { return _relay->isEnabled() || _dimmer->isEnabled(); }
      bool isAutoBypassEnabled() const { return isBypassEnabled() && config.autoBypass; }
      bool isBypassOn() const { return _relay->isEnabled() ? _relay->isOn() : _bypassEnabled; }
      bool tryBypassState(bool state);
      void applyAutoBypass();

      // metrics

      float getPower() const {
        if (getState() != RouterOutputState::OUTPUT_ROUTING)
          return 0;

        if (_pzem->isConnected())
          return _pzem->getPower();

        GridMetrics gridMetrics;
        _grid->getMetrics(gridMetrics);
        float dimmedVoltage = gridMetrics.voltage * _dimmer->getVrms();
        float resistance = 0; // TODO: calibrate R
        return resistance == 0 ? 0 : dimmedVoltage * dimmedVoltage / resistance;
      }

      void getMetrics(RouterOutputMetrics& metrics) const {
        if (_pzem->isConnected()) {
          metrics.connected = _pzem->isConnected();
          metrics.voltage = _pzem->getVoltage();
          metrics.energy = _pzem->getEnergy();
          if (getState() == RouterOutputState::OUTPUT_ROUTING) {
            metrics.dimmedVoltage = metrics.voltage * _dimmer->getVrms();
            metrics.current = _pzem->getCurrent();
            metrics.resistance = metrics.current == 0 ? 0 : metrics.dimmedVoltage / metrics.current;
            metrics.apparentPower = _pzem->getApparentPower();
            metrics.power = _pzem->getPower();
            metrics.powerFactor = _pzem->getPowerFactor();
            metrics.thdi = _pzem->getTHDi(0);
          }

        } else {
          GridMetrics gridMetrics;
          _grid->getMetrics(gridMetrics);
          metrics.connected = gridMetrics.connected;
          metrics.voltage = gridMetrics.voltage;
          metrics.resistance = 0;              // TODO: calibrate R
          metrics.energy = _pzem->getEnergy(); // just in case we have it...
          if (getState() == RouterOutputState::OUTPUT_ROUTING) {
            metrics.dimmedVoltage = metrics.voltage * _dimmer->getVrms();
            metrics.current = metrics.resistance == 0 ? 0 : metrics.dimmedVoltage / metrics.resistance;
            metrics.power = metrics.resistance == 0 ? 0 : metrics.dimmedVoltage * metrics.dimmedVoltage / metrics.resistance;
            metrics.apparentPower = metrics.voltage * metrics.current;
            metrics.powerFactor = metrics.voltage == 0 ? 0 : metrics.dimmedVoltage / metrics.voltage;
            metrics.thdi = metrics.powerFactor == 0 ? 0 : sqrt(1 / pow(metrics.powerFactor, 2) - 1);
          }
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
      void _setBypass(bool state, uint16_t dimmerLevelWhenRelayOff = 0);
  };
} // namespace Mycila
