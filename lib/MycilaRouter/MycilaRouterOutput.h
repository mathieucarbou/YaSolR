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
        _dimmer->toJson(root["dimmer"].to<JsonObject>());
        _temperatureSensor->toJson(root["ds18"].to<JsonObject>());
        JsonObject jsonMetrics = root["metrics"].to<JsonObject>();
        jsonMetrics["apparent_power"] = metrics.apparentPower;
        jsonMetrics["current"] = metrics.current;
        jsonMetrics["energy"] = metrics.energy;
        jsonMetrics["power"] = metrics.power;
        jsonMetrics["power_factor"] = metrics.powerFactor;
        jsonMetrics["resistance"] = metrics.resistance;
        jsonMetrics["thdi"] = metrics.thdi;
        jsonMetrics["voltage"] = metrics.voltage;
        jsonMetrics["voltage_dimmed"] = metrics.dimmedVoltage;
        RouterOutputMetrics metricsPZEM;
        _getMetricsFromPZEM(metricsPZEM);
        jsonMetrics["pzem"]["apparent_power"] = metricsPZEM.apparentPower;
        jsonMetrics["pzem"]["current"] = metricsPZEM.current;
        jsonMetrics["pzem"]["energy"] = metricsPZEM.energy;
        jsonMetrics["pzem"]["power"] = metricsPZEM.power;
        jsonMetrics["pzem"]["power_factor"] = metricsPZEM.powerFactor;
        jsonMetrics["pzem"]["resistance"] = metricsPZEM.resistance;
        jsonMetrics["pzem"]["thdi"] = metricsPZEM.thdi;
        jsonMetrics["pzem"]["voltage"] = metricsPZEM.voltage;
        jsonMetrics["pzem"]["voltage_dimmed"] = metricsPZEM.dimmedVoltage;
        RouterOutputMetrics metricsDuty;
        _getMetricsFromDuty(metricsDuty);
        jsonMetrics["duty"]["apparent_power"] = metricsDuty.apparentPower;
        jsonMetrics["duty"]["current"] = metricsDuty.current;
        jsonMetrics["duty"]["energy"] = metricsDuty.energy;
        jsonMetrics["duty"]["power"] = metricsDuty.power;
        jsonMetrics["duty"]["power_factor"] = metricsDuty.powerFactor;
        jsonMetrics["duty"]["resistance"] = metricsDuty.resistance;
        jsonMetrics["duty"]["thdi"] = metricsDuty.thdi;
        jsonMetrics["duty"]["voltage"] = metricsDuty.voltage;
        jsonMetrics["duty"]["voltage_dimmed"] = metricsDuty.dimmedVoltage;
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

      float getPower() const {
        if (getState() != RouterOutputState::OUTPUT_ROUTING)
          return 0;

        if (_pzem->isConnected())
          return _pzem->getPower();

        // TODO: calibrate R
        // float resistance = 0;
        float resistance = _pzem->getCurrent() == 0 ? 0 : sqrt(_dimmer->getPowerDutyCycle()) * _grid->getVoltage() / _pzem->getCurrent();
        float voltage = _grid->getVoltage();
        float nominalPower = voltage * voltage / resistance;
        float dutyCycle = _dimmer->getPowerDutyCycle();
        return dutyCycle * nominalPower;
      }

      void getMetrics(RouterOutputMetrics& metrics) const {
        if (_pzem->isConnected())
          _getMetricsFromPZEM(metrics);
        else
          _getMetricsFromDuty(metrics);
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

      void _getMetricsFromPZEM(RouterOutputMetrics& metrics) const {
        metrics.connected = _pzem->isConnected();
        metrics.voltage = _pzem->getVoltage();
        metrics.energy = _pzem->getEnergy();
        if (getState() == RouterOutputState::OUTPUT_ROUTING) {
          metrics.apparentPower = _pzem->getApparentPower();
          metrics.current = _pzem->getCurrent();
          metrics.power = _pzem->getPower();
          metrics.powerFactor = _pzem->getPowerFactor();
          metrics.thdi = _pzem->getTHDi(0);
          metrics.dimmedVoltage = metrics.power / metrics.current;
          metrics.resistance = metrics.current == 0 ? 0 : metrics.dimmedVoltage / metrics.current;
        }
      }

      void _getMetricsFromDuty(RouterOutputMetrics& metrics) const {
        metrics.connected = true;
        metrics.voltage = _grid->getVoltage();
        metrics.energy = _pzem->getEnergy();
        // TODO: calibrate R
        // metrics.resistance = 0;
        metrics.resistance = _pzem->getCurrent() == 0 ? 0 : sqrt(_dimmer->getPowerDutyCycle()) * _grid->getVoltage() / _pzem->getCurrent();
        if (getState() == RouterOutputState::OUTPUT_ROUTING) {
          float dutyCycle = _dimmer->getPowerDutyCycle();
          float nominalPower = metrics.voltage * metrics.voltage / metrics.resistance;
          metrics.powerFactor = sqrt(dutyCycle);
          metrics.power = dutyCycle * nominalPower;
          metrics.apparentPower = metrics.powerFactor * nominalPower;
          metrics.dimmedVoltage = metrics.powerFactor * metrics.voltage;
          metrics.current = metrics.dimmedVoltage / metrics.resistance;
          metrics.thdi = dutyCycle == 0 ? 0 : sqrt(1 / dutyCycle - 1);
        }
      }

    private:
      void _setBypass(bool state, uint16_t dimmerDutyWhenRelayOff = 0);
  };
} // namespace Mycila
