// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaDS18.h>
#include <MycilaDimmer.h>
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

  typedef std::function<void()> RouterOutputStateCallback;

  class RouterOutput {
    public:
      typedef struct {
          bool autoDimmer;
          uint8_t dimmerLimit;
          bool autoBypass;
          uint8_t autoStartTemperature;
          uint8_t autoStopTemperature;
          String autoStartTime;
          String autoStopTime;
          String weekDays;
      } Config;

      RouterOutput(const char* name, Dimmer& dimmer, DS18& temperatureSensor, Relay& relay, PZEM& pzem) : _name(name),
                                                                                                          _dimmer(&dimmer),
                                                                                                          _temperatureSensor(&temperatureSensor),
                                                                                                          _relay(&relay),
                                                                                                          _pzem(&pzem) {}
      // output

      RouterOutputState getState() const {
        if (!_dimmer->isEnabled() && !_relay->isEnabled())
          return RouterOutputState::OUTPUT_DISABLED;
        if (_dimmer->getLevel() > 0)
          return RouterOutputState::OUTPUT_ROUTING;
        // dimmer level is 0
        if (_autoBypassEnabled)
          return RouterOutputState::OUTPUT_BYPASS_AUTO;
        if (_relay->isOn())
          return RouterOutputState::OUTPUT_BYPASS_MANUAL;
        return RouterOutputState::OUTPUT_IDLE;
      }
      const char* getStateName() const;

      bool isEnabled() const { return _dimmer->isEnabled(); }
      void listen(RouterOutputStateCallback callback) { _callback = callback; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const {
        root["apparent_power"] = getApparentPower();
        root["current"] = getCurrent();
        root["enabled"] = _dimmer->isEnabled();
        root["energy"] = getEnergy();
        root["power_factor"] = getPowerFactor();
        root["power"] = getActivePower();
        root["resistance"] = getResistance();
        root["state"] = getStateName();
        root["thdi"] = getTHDi();
        root["voltage_dimmed"] = getDimmedVoltage();
        root["voltage"] = getVoltage();
      }
#endif

      // dimmer

      bool isDimmerEnabled() const { return _dimmer->isEnabled(); }
      bool isAutoDimmerEnabled() const { return _dimmer->isEnabled() && config.autoDimmer; }
      uint8_t getDimmerLevel() const { return _dimmer->getLevel(); }
      bool tryDimmerLevel(uint8_t level);
      void applyDimmerLimit();

      // bypass

      bool isBypassEnabled() const { return _relay->isEnabled() || _dimmer->isEnabled(); }
      bool isAutoBypassEnabled() const { return isBypassEnabled() && config.autoBypass; }
      bool isBypassOn() const { return _relay->isEnabled() ? _relay->isOn() : _bypassEnabled; }
      bool tryBypassState(bool state);
      void applyAutoBypass();

      // electricity settings

      bool hasMetrics() const { return _pzem->isConnected(); }
      float getDimmedVoltage() const { return _dimmer->getDimmedVoltage(getVoltage()); }
      float getResistance() const {
        // R = U / I
        float i = getCurrent();
        return i == 0 ? 0 : getDimmedVoltage() / i;
      }

      // metrics

      float getEnergy() const { return _pzem->getEnergy(); }
      float getVoltage() const { return _pzem->getVoltage(); }
      float getCurrent() const { return _pzem->getCurrent(); }
      float getActivePower() const { return _pzem->getPower(); }
      // float getActivePower() const { return getDimmedVoltage() * getCurrent(); }
      float getApparentPower() const { return _pzem->getApparentPower(); }
      // float getApparentPower() const { return getVoltage() * getCurrent(); }
      float getPowerFactor() const { return _pzem->getPowerFactor(); }
      // float getPowerFactor() const { return getActivePower() / getApparentPower(); }
      float getTHDi() const { return _pzem->getTHDi(0); }
      // float getTHDi() const {
      //   // https://fr.electrical-installation.org/frwiki/Indicateur_de_distorsion_harmonique_:_facteur_de_puissance
      //   // https://www.salicru.com/files/pagina/72/278/jn004a01_whitepaper-armonics_(1).pdf
      //   // For a resistive load, phi = 0 (no displacement between voltage and current)
      //   float pf = getPowerFactor();
      //   return pf == 0 ? 0 : sqrt(1 / pow(pf, 2) - 1);
      // }

    public:
      Config config;

    private:
      const char* _name;
      Dimmer* _dimmer;
      Mycila::DS18* _temperatureSensor;
      Relay* _relay;
      PZEM* _pzem;
      bool _autoBypassEnabled = false;
      bool _bypassEnabled = false;
      RouterOutputStateCallback _callback = nullptr;

    private:
      void _setBypass(bool state, uint8_t dimmerLevelWhenRelayOff = 0);
  };
} // namespace Mycila
