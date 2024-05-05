// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaPZEM004Tv3.h>
#include <MycilaRelay.h>
#include <MycilaTemperatureSensor.h>

#include <ArduinoJson.h>

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

  class RouterOutputConfigClass {
    public:
      bool isAutoDimmerEnabled(const char* name) const;
      uint8_t getDimmerLevelLimit(const char* name) const;
      bool isAutoBypassEnabled(const char* name) const;
      uint8_t getAutoStartTemperature(const char* name) const;
      uint8_t getAutoStopTemperature(const char* name) const;
      String getAutoStartTime(const char* name) const;
      String getAutoStopTime(const char* name) const;
      String getWeekDays(const char* name) const;
  };

  class RouterOutput {
    public:
      RouterOutput(const char* name, Dimmer* dimmer, TemperatureSensor* temperatureSensor, Relay* relay, PZEM* pzem) : _name(name),
                                                                                                                       _dimmer(dimmer),
                                                                                                                       _temperatureSensor(temperatureSensor),
                                                                                                                       _relay(relay),
                                                                                                                       _pzem(pzem) {}
      // output

      const char* getName() const { return _name; }
      RouterOutputState getState() const;
      const char* getStateString() const;
      bool isEnabled() const { return _dimmer->isEnabled(); }

      void toJson(const JsonObject& root) const;

      void listen(RouterOutputStateCallback callback) { _callback = callback; }

      // components

      const Dimmer* getDimmer() const { return _dimmer; }
      const TemperatureSensor* getTemperatureSensor() const { return _temperatureSensor; }
      const Relay* getBypassRelay() const { return _relay; }
      const PZEM* getPZEM() const { return _pzem; }

      // dimmer

      // level: 0-100 (%)
      bool tryDimmerLevel(uint8_t level);
      void applyDimmerLimit();

      // bypass

      void autoBypass();
      bool isBypassRelayOn() const { return _relay->isEnabled() ? _relay->isOn() : _bypassEnabled; }
      bool isBypassRelayEnabled() const { return _relay->isEnabled() || _dimmer->isEnabled(); }
      inline bool tryBypassRelayToggle() { return tryBypassRelayState(!isBypassRelayOn()); }
      bool tryBypassRelayState(bool state);

      // electricity (if available through PZEM)

      void updateElectricityStatistics();

      // kWh : Energy measured by PZEM
      float getEnergy() const { return _energy; }
      // A : Current measured by PZEM
      float getCurrent() const { return _current; }
      // V : Input Voltage at Dimmer entrance (measured with PZEM, JSY, MQTT or configured default value)
      float getInputVoltage() const { return _inputVoltage; }
      // V : Dimmed Voltage at Dimmer output, calculated based on phase angle and trigger delay
      float getOutputVoltage() const { return _outputVoltage; }
      // VA : Apparent Power == Input Voltage * Current
      float getApparentPower() const { return _inputVoltage * _current; }
      // Ohm : Resistance == Dimmed Voltage / Current (U = R * I)
      float getResistance() const { return _current == 0 ? 0 : _outputVoltage / _current; }
      // W : Active Power == Dimmed Voltage * Current (power consumed by the resistive load)
      float getActivePower() const { return _outputVoltage * _current; }
      // PF : Power Factor == Active Power / Apparent Power
      float getPowerFactor() const { return _outputVoltage / _inputVoltage; }
      // THDi : Total Harmonic Distortion of current
      // https://fr.electrical-installation.org/frwiki/Indicateur_de_distorsion_harmonique_:_facteur_de_puissance
      // For a resistive load, phi = 0 (no displacement between voltage and current)
      float getTHDi() const { return _outputVoltage == 0 ? 0 : sqrt(1 / pow(_outputVoltage / _inputVoltage, 2) - 1); }

    private:
      const char* _name;
      Dimmer* _dimmer;
      const TemperatureSensor* _temperatureSensor;
      Relay* _relay;
      PZEM* _pzem;
      bool _autoBypassEnabled = false;
      bool _bypassEnabled = false;
      RouterOutputStateCallback _callback = nullptr;

    private:
      float _energy = 0;
      float _current = 0;
      float _outputVoltage = 0;
      float _inputVoltage = 0;

    private:
      void _setBypassRelay(bool state, uint8_t dimmerLevelWhenRelayOff = 0);
  };

  extern RouterOutputConfigClass RouterOutputConfig;
} // namespace Mycila
