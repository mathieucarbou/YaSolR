// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaExpiringValue.h>
#include <MycilaRelay.h>

#include <math.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#ifndef MYCILA_RELAY_DEFAULT_TOLERANCE
  // in percentage
  // => 50W for a tri-phase 3000W resistance (1000W per phase)
  // => 35W for a tri-phase 2100W resistance (700W per phase)
  #define MYCILA_RELAY_DEFAULT_TOLERANCE 0.05f
#endif

namespace Mycila {
  class Router {
    public:
      /////////////
      // Metrics //
      /////////////

      enum class Source {
        PZEM,
        JSY,
        JSY_REMOTE,
        COMPUTED,
        UNKNOWN
      };

      class Metrics {
        public:
          Source source = Source::UNKNOWN;
          float apparentPower = 0;
          float current = 0;
          uint32_t energy = 0;
          float power = 0;
          float powerFactor = NAN;
          float resistance = NAN;
          float thdi = NAN;
          float voltage = NAN;

          Metrics() = default;
          Metrics(Metrics&& other) noexcept {
            source = other.source;
            apparentPower = other.apparentPower;
            current = other.current;
            energy = other.energy;
            power = other.power;
            powerFactor = other.powerFactor;
            resistance = other.resistance;
            thdi = other.thdi;
            voltage = other.voltage;

            other.source = Source::UNKNOWN;
            other.apparentPower = 0;
            other.current = 0;
            other.energy = 0;
            other.power = 0;
            other.powerFactor = NAN;
            other.resistance = NAN;
            other.thdi = NAN;
            other.voltage = NAN;
          }
          Metrics& operator=(Metrics&& other) noexcept {
            if (this != &other) {
              source = other.source;
              apparentPower = other.apparentPower;
              current = other.current;
              energy = other.energy;
              power = other.power;
              powerFactor = other.powerFactor;
              resistance = other.resistance;
              thdi = other.thdi;
              voltage = other.voltage;

              other.source = Source::UNKNOWN;
              other.apparentPower = 0;
              other.current = 0;
              other.energy = 0;
              other.power = 0;
              other.powerFactor = NAN;
              other.resistance = NAN;
              other.thdi = NAN;
              other.voltage = NAN;
            }
            return *this;
          }
          Metrics(const Metrics& other) = delete;
          Metrics& operator=(const Metrics& other) = delete;
      };

#ifdef MYCILA_JSON_SUPPORT
      static void toJson(const JsonObject& dest, const Metrics& metrics) {
        switch (metrics.source) {
          case Source::JSY:
            dest["source"] = "jsy";
            break;
          case Source::JSY_REMOTE:
            dest["source"] = "jsy_remote";
            break;
          case Source::COMPUTED:
            dest["source"] = "computed";
            break;
          case Source::PZEM:
            dest["source"] = "pzem";
            break;
          default:
            dest["source"] = "unknown";
            break;
        }

        dest["energy"] = metrics.energy;
        if (!std::isnan(metrics.voltage))
          dest["voltage"] = metrics.voltage;
        if (!std::isnan(metrics.current))
          dest["current"] = metrics.current;
        if (!std::isnan(metrics.power))
          dest["power"] = metrics.power;
        if (!std::isnan(metrics.apparentPower))
          dest["apparent_power"] = metrics.apparentPower;
        if (!std::isnan(metrics.powerFactor))
          dest["power_factor"] = metrics.powerFactor;
        if (!std::isnan(metrics.resistance))
          dest["resistance"] = metrics.resistance;
        if (!std::isnan(metrics.thdi))
          dest["thdi"] = metrics.thdi;
      }
#endif

      //////////////////
      // Router Relay //
      //////////////////

      class Relay {
        public:
          void setNominalLoad(uint16_t load) { _nominalLoad = load; }
          uint16_t getNominalLoad() const { return _nominalLoad; }

          /**
           * @brief Compute the load based on the grid voltage and the nominal load.
           */
          uint16_t computeLoad(float gridVoltage) const {
            // detects the grid nominal voltage
            const uint16_t nominalVoltage = static_cast<uint8_t>(gridVoltage / 100) == 1 ? 110 : 230;
            // compute the amperage of the given nominal power of connected load
            const float resistance = static_cast<float>(nominalVoltage * nominalVoltage) / static_cast<float>(_nominalLoad);
            // compute with the current voltage what the exact power of the load would be
            return static_cast<uint16_t>(gridVoltage * gridVoltage / resistance);
          }

          void setTolerance(float tolerance) { _tolerance = tolerance; }
          float getTolerance() const { return _tolerance; }

          bool isEnabled() const { return _relay.isEnabled(); }
          bool isAutoRelayEnabled() const { return _relay.isEnabled() && _nominalLoad > 0; }

          bool trySwitchRelay(bool state, uint32_t duration = 0);

          bool autoSwitch(float gridVoltage, float gridPower, float routedPower, float setpoint);

          bool isOn() const { return _relay.isOn(); }
          bool isOff() const { return _relay.isOff(); }

          uint64_t getSwitchCount() const { return _relay.getSwitchCount(); }

#ifdef MYCILA_JSON_SUPPORT
          void toJson(const JsonObject& root) const { _relay.toJson(root); }
#endif

          Mycila::Relay& relay() { return _relay; }

        private:
          Mycila::Relay _relay;
          uint16_t _nominalLoad = 0;
          float _tolerance = MYCILA_RELAY_DEFAULT_TOLERANCE;
      };

      ////////////
      // Output //
      ////////////

      class Output {
        public:
          enum class State {
            // output disabled
            UNUSED,
            // idle
            IDLE,
            // excess power sent to load
            ROUTING,
            // full power sent to load through relay (manual trigger)
            BYPASS_MANUAL,
            // full power sent to load through relay (auto trigger)
            BYPASS_AUTO
          };

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

          explicit Output(const char* name) : _name(name) {
            _metrics.setExpiration(10000);
          }

          // dimmer is mandatory
          void setDimmer(Dimmer* dimmer) { _dimmer = dimmer; }
          // Bypass relay is optional
          void setBypassRelay(Mycila::Relay* relay) { _relay = relay; }

          // output

          State getState() const {
            if (!isDimmerOnline() && !isBypassRelayEnabled())
              return State::UNUSED;
            if (_autoBypassEnabled)
              return State::BYPASS_AUTO;
            if (_manualBypassEnabled)
              return State::BYPASS_MANUAL;
            if (_dimmer->isOn())
              return State::ROUTING;
            return State::IDLE;
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

            {
              Metrics* metrics = new Metrics();
              readMeasurements(*metrics);
              Router::toJson(root["measurements"].to<JsonObject>(), *metrics);
              delete metrics;
            }

            JsonArray sources = root["sources"].to<JsonArray>();

            {
              Metrics* metrics = new Metrics();
              computeMetrics(*metrics, gridVoltage);
              Router::toJson(sources.add<JsonObject>(), *metrics);
              delete metrics;
            }

            if (_metrics.isPresent()) {
              JsonObject source = sources.add<JsonObject>();
              Router::toJson(source, _metrics.get());
              source["time"] = _metrics.getLastUpdateTime();
            }

            _dimmer->toJson(root["dimmer"].to<JsonObject>());
            if (_relay)
              _relay->toJson(root["relay"].to<JsonObject>());
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
            if (availablePowerToDivert <= 0 || gridVoltage <= 0) {
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
            return powerToDivert;
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

          void updateMetrics(Metrics metrics) {
            _metrics.update(std::move(metrics));
          }

          bool readMeasurements(Metrics& metrics) const {
            if (_metrics.isPresent()) {
              if (getState() == State::ROUTING) {
                memcpy(&metrics, &_metrics.get(), sizeof(Metrics));
              } else {
                metrics.source = _metrics.get().source;
                metrics.energy = _metrics.get().energy;
              }
              return true;
            }
            return false;
          }

          bool computeMetrics(Router::Metrics& metrics, float gridVoltage) const {
            if (gridVoltage > 0 && config.calibratedResistance > 0) {
              metrics.source = Source::COMPUTED;
              metrics.resistance = config.calibratedResistance;
              if (getState() == State::ROUTING) {
                Mycila::Dimmer::Metrics dimmerMetrics;
                if (_dimmer->calculateMetrics(dimmerMetrics, gridVoltage, config.calibratedResistance)) {
                  metrics.voltage = dimmerMetrics.voltage;
                  metrics.current = dimmerMetrics.current;
                  metrics.apparentPower = dimmerMetrics.apparentPower;
                  metrics.power = dimmerMetrics.power;
                  metrics.powerFactor = dimmerMetrics.powerFactor;
                  metrics.thdi = dimmerMetrics.thdi;
                  return true;
                }
              }
            }
            return false;
          }

          std::optional<float> readRoutedPower() const {
            if (getState() != State::ROUTING)
              return 0.0f;
            if (_metrics.isPresent())
              return _metrics.get().power;
            return std::nullopt;
          }

          std::optional<float> computeRoutedPower(float gridVoltage) const {
            if (getState() != State::ROUTING)
              return 0.0f;
            if (gridVoltage > 0) {
              Mycila::Dimmer::Metrics dimmerMetrics;
              if (_dimmer->calculateMetrics(dimmerMetrics, gridVoltage, config.calibratedResistance)) {
                return dimmerMetrics.power;
              }
            }
            return std::nullopt;
          }

          std::optional<float> readRoutedCurrent() const {
            if (getState() != State::ROUTING)
              return 0.0f;
            if (_metrics.isPresent())
              return _metrics.get().current;
            return std::nullopt;
          }

          std::optional<float> computeRoutedCurrent(float gridVoltage) const {
            if (getState() != State::ROUTING)
              return 0.0f;
            if (gridVoltage > 0) {
              Mycila::Dimmer::Metrics dimmerMetrics;
              if (_dimmer->calculateMetrics(dimmerMetrics, gridVoltage, config.calibratedResistance)) {
                return dimmerMetrics.current;
              }
            }
            return std::nullopt;
          }

          std::optional<float> readResistance() const {
            if (_metrics.isPresent())
              return _metrics.get().resistance;
            return std::nullopt;
          }

          bool computeHarmonics(float* array, size_t n) const {
            if (array == nullptr || n == 0)
              return false;

            if (getState() != State::ROUTING) {
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
          Mycila::Relay* _relay = nullptr; // optional
          bool _autoBypassEnabled = false;
          bool _manualBypassEnabled = false;
          uint32_t _manualBypassTime = 0;
          ExpiringValue<float> _temperature;
          ExpiringValue<Metrics> _metrics;

        private:
          void _switchBypass(bool state, bool log = true);
      };

      ////////////
      // Router //
      ////////////

      void addOutput(Output& output) { _outputs.push_back(&output); }
      const std::vector<Output*>& getOutputs() const { return _outputs; }

      bool isAutoDimmerEnabled() const {
        for (const auto& output : _outputs) {
          if (output->isAutoDimmerEnabled()) {
            return true;
          }
        }
        return false;
      }

      // returns true if some power were diverted
      float divert(const float gridVoltage, const float powerToDivert) {
        if (isCalibrationRunning())
          return false;
        float routedPower = 0;
        for (const auto& output : _outputs) {
          routedPower += output->autoDivert(gridVoltage, std::max(0.0f, powerToDivert - routedPower));
        }
        return routedPower;
      }

      void noDivert() {
        if (isCalibrationRunning())
          return;
        for (const auto& output : _outputs) {
          output->autoDivert(0, 0);
        }
      }

      typedef std::function<void()> CalibrationCallback;
      void beginCalibration(size_t outputIndex, CalibrationCallback cb = nullptr);
      void continueCalibration();
      bool isCalibrationRunning() const { return _calibrationRunning; }
      bool isCalibrationRunning(size_t outputIndex) const { return isCalibrationRunning() && _calibrationOutputIndex == outputIndex; }
      uint8_t getCalibrationCompletion(size_t outputIndex) const {
        if (!isCalibrationRunning(outputIndex))
          return 0;
        if (_calibrationStartTime == 0)
          return 0;
        // calibration lasts 10 seconds
        return std::min(100, static_cast<int>((millis() - _calibrationStartTime) / 100));
      }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root, float gridVoltage) const {
        {
          Metrics* routerMeasurements = new Metrics();
          readMeasurements(*routerMeasurements);
          Router::toJson(root["measurements"].to<JsonObject>(), *routerMeasurements);
          delete routerMeasurements;
        }

        JsonArray sources = root["sources"].to<JsonArray>();

        {
          Metrics* metrics = new Metrics();
          computeMetrics(*metrics, gridVoltage);
          Router::toJson(sources.add<JsonObject>(), *metrics);
          delete metrics;
        }

        if (_jsyMetrics != nullptr && _jsyMetrics->isPresent()) {
          JsonObject jsy = sources.add<JsonObject>();
          toJson(jsy, _jsyMetrics->get());
          jsy["time"] = _jsyMetrics->getLastUpdateTime();
        }

        if (_jsyRemoteMetrics != nullptr && _jsyRemoteMetrics->isPresent()) {
          JsonObject jsyRemote = sources.add<JsonObject>();
          toJson(jsyRemote, _jsyRemoteMetrics->get());
          jsyRemote["time"] = _jsyRemoteMetrics->getLastUpdateTime();
        }
      }
#endif

      void deleteMetrics(Source source) {
        switch (source) {
          case Source::JSY:
            delete _jsyMetrics;
            _jsyMetrics = nullptr;
            break;
          case Source::JSY_REMOTE:
            delete _jsyRemoteMetrics;
            _jsyRemoteMetrics = nullptr;
            break;
          default:
            throw std::runtime_error("Unknown Router Source");
        }
      }

      void updateMetrics(Metrics metrics) {
        switch (metrics.source) {
          case Source::JSY: {
            if (_jsyMetrics == nullptr) {
              _jsyMetrics = new ExpiringValue<Metrics>();
              _jsyMetrics->setExpiration(10000);
            }
            _jsyMetrics->update(std::move(metrics));
            break;
          }
          case Source::JSY_REMOTE: {
            if (_jsyRemoteMetrics == nullptr) {
              _jsyRemoteMetrics = new ExpiringValue<Metrics>();
              _jsyRemoteMetrics->setExpiration(10000);
            }
            _jsyRemoteMetrics->update(std::move(metrics));
            break;
          }
          default:
            break;
        }
      }

      // get router measurements based on the connected JSY (for an aggregated view of all outputs) or PZEM per output
      bool readMeasurements(Metrics& metrics) const {
        metrics.source = Source::UNKNOWN;

        for (size_t i = 0; i < _outputs.size(); i++) {
          Router::Metrics outputMetrics;
          if (_outputs[i]->readMeasurements(outputMetrics)) {
            metrics.source = Source::PZEM;
            // Note: energy is not accurate in the case of a virtual bypass through dimmer
            metrics.energy += outputMetrics.energy;
            metrics.apparentPower += outputMetrics.apparentPower;
            metrics.current += outputMetrics.current;
            metrics.power += outputMetrics.power;
          }
        }

        // we found some pzem ? we are done
        if (metrics.source == Source::PZEM) {
          metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
          metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
          metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);
          return true;
        }

        // no pzem found, let's check if we have a local JSY or remote JSY
        if (_jsyMetrics && _jsyMetrics->isPresent()) {
          // Note: if one output is routing and the other one is doing a virtual bypass through dimmer, sadly we cannot have accurate measurements
          memcpy(&metrics, &_jsyMetrics->get(), sizeof(Metrics));
          return true;
        }
        if (_jsyRemoteMetrics && _jsyRemoteMetrics->isPresent()) {
          // Note: if one output is routing and the other one is doing a virtual bypass through dimmer, sadly we cannot have accurate measurements
          memcpy(&metrics, &_jsyRemoteMetrics->get(), sizeof(Metrics));
          return true;
        }

        return false;
      }

      bool computeMetrics(Metrics& metrics, float gridVoltage) const {
        if (gridVoltage > 0) {
          metrics.source = Source::COMPUTED;
          for (size_t i = 0; i < _outputs.size(); i++) {
            Router::Metrics outputMetrics;
            if (_outputs[i]->computeMetrics(outputMetrics, gridVoltage)) {
              metrics.energy += outputMetrics.energy;
              metrics.apparentPower += outputMetrics.apparentPower;
              metrics.current += outputMetrics.current;
              metrics.power += outputMetrics.power;
            }
          }
          metrics.powerFactor = metrics.apparentPower == 0 ? NAN : metrics.power / metrics.apparentPower;
          metrics.resistance = metrics.current == 0 ? NAN : metrics.power / (metrics.current * metrics.current);
          metrics.thdi = metrics.powerFactor == 0 ? NAN : 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f);
          return true;
        }
        return false;
      }

      std::optional<float> readTotalRoutedPower() const {
        float power = 0;
        for (size_t i = 0; i < _outputs.size(); i++) {
          power += _outputs[i]->readRoutedPower().value_or(NAN);
        }
        if (!std::isnan(power)) {
          return power;
        }
        if (_jsyMetrics && _jsyMetrics->isPresent())
          return _jsyMetrics->get().power;
        if (_jsyRemoteMetrics && _jsyRemoteMetrics->isPresent())
          return _jsyRemoteMetrics->get().power;
        return std::nullopt;
      }

      std::optional<float> computeTotalRoutedPower(float gridVoltage) const {
        float power = 0;
        for (size_t i = 0; i < _outputs.size(); i++) {
          power += _outputs[i]->computeRoutedPower(gridVoltage).value_or(NAN);
        }
        if (!std::isnan(power)) {
          return power;
        }
        return std::nullopt;
      }

      std::optional<float> readResistance() const {
        if (_jsyMetrics && _jsyMetrics->isPresent())
          return _jsyMetrics->get().resistance;
        if (_jsyRemoteMetrics && _jsyRemoteMetrics->isPresent())
          return _jsyRemoteMetrics->get().resistance;
        return std::nullopt;
      }

    private:
      std::vector<Output*> _outputs;
      ExpiringValue<Metrics>* _jsyMetrics;
      ExpiringValue<Metrics>* _jsyRemoteMetrics;

      // calibration
      // 0: idle
      // 1: prepare
      // 2: output 1 50%
      // 3: output 1 100%
      // 4: cleanup
      uint8_t _calibrationStep = 0;
      uint32_t _calibrationStartTime = 0;
      uint32_t _calibrationStepStartTime = 0;
      size_t _calibrationOutputIndex = 0;
      bool _calibrationRunning = false;
      CalibrationCallback _calibrationCallback = nullptr;
  };
} // namespace Mycila
