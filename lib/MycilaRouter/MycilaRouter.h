// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2026 Mathieu Carbou
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaExpiringValue.h>
#include <MycilaMetrics.h>
#include <MycilaRelay.h>

#include <math.h>

#include <algorithm>
#include <cstdio>
#include <memory>
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

#ifndef MYCILA_OUTPUT_LOW_POWER_THRESHOLD
  // Set the threshold under which we consider that the load is not consuming
  #define MYCILA_OUTPUT_LOW_POWER_THRESHOLD 2
#endif

namespace Mycila {
  class Router {
    public:
      //////////////////
      // Router Relay //
      //////////////////

      class Relay {
        public:
          void setNominalLoad(uint16_t load) {
            _nominalLoad = load;
          }
          uint16_t getNominalLoad() const {
            return _nominalLoad;
          }

          /**
           * @brief Compute the load based on the grid voltage and the nominal load.
           */
          uint16_t computeLoad(float gridVoltage) const {
            if (gridVoltage > 0) {
              // detects the grid nominal voltage
              const uint16_t nominalVoltage = static_cast<uint8_t>(gridVoltage / 100) == 1 ? 110 : 230;
              // compute the amperage of the given nominal power of connected load
              const float resistance = static_cast<float>(nominalVoltage * nominalVoltage) / static_cast<float>(_nominalLoad);
              // compute with the current voltage what the exact power of the load would be
              return static_cast<uint16_t>(gridVoltage * gridVoltage / resistance);
            } else {
              return 0;
            }
          }

          void setTolerance(float tolerance) {
            _tolerance = tolerance;
          }
          float getTolerance() const {
            return _tolerance;
          }

          bool isEnabled() const {
            return _relay.isEnabled();
          }
          bool isAutoRelayEnabled() const {
            return _relay.isEnabled() && _nominalLoad > 0;
          }

          bool trySwitchRelay(bool state, uint32_t duration = 0);

          bool autoSwitch(float gridVoltage, float gridPower, float routedPower, float setpoint);

          bool isOn() const {
            return _relay.isOn();
          }
          bool isOff() const {
            return _relay.isOff();
          }

          uint64_t getSwitchCount() const {
            return _relay.getSwitchCount();
          }

#ifdef MYCILA_JSON_SUPPORT
          void toJson(const JsonObject& root) const {
            _relay.toJson(root);
          }
#endif

          Mycila::Relay& relay() {
            return _relay;
          }

        private:
          Mycila::Relay _relay;
          uint16_t _nominalLoad = 0;
          float _tolerance = MYCILA_RELAY_DEFAULT_TOLERANCE;
      };

      ////////////
      // Output //
      ////////////

      class Output : public metric::MetricSupport {
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
              float calibratedResistance = 0.0f;
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

          Output(const char* label, const char* mqttName) : _name(label), _mqttName(mqttName) {
            _lastTimeConsumptionWasDetectedWhileRouting.setExpiration(10000); // 10 seconds expiration
          }

          // dimmer is mandatory
          void setDimmer(Dimmer* dimmer) {
            _dimmer = dimmer;
          }
          // Bypass relay is optional
          void setBypassRelay(Mycila::Relay* relay) {
            _relay = relay;
          }

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
          const char* getName() const {
            return _name;
          }
          const char* getMqttName() const {
            return _mqttName;
          }
          bool isOn() const {
            return isDimmerOn() || isBypassRelayOn();
          }

#ifdef MYCILA_JSON_SUPPORT
          void toJson(const JsonObject& root, float gridVoltage) const {
            root["enabled"] = isDimmerOnline();
            root["state"] = getStateName();
            root["bypass"] = isBypassOn() ? "on" : "off";
            root["bypass_elapsed"] = getBypassUptime();
            _lastTimeConsumptionWasDetectedWhileRouting.toJson(root["last_routed_power"].to<JsonObject>());
            if (_temperature.isPresent())
              root["temperature"] = _temperature.get();
            JsonArray metrics = root["metrics"].to<JsonArray>();
            {
              std::unique_ptr<metric::Metrics> computed = std::make_unique<metric::Metrics>();
              if (computeRoutedMetrics(*computed, gridVoltage)) {
                JsonObject source = metrics.add<JsonObject>();
                source["source"] = metric::sourceToString(metric::Source::COMPUTED);
                metric::Metrics::toJson(source, *computed);
              }
            }
            if (_metrics.isPresent()) {
              JsonObject source = metrics.add<JsonObject>();
              source["source"] = getSourceString();
              source["time"] = _metrics.getLastUpdateTime();
              metric::Metrics::toJson(source, *_metrics.get());
            }
            _dimmer->toJson(root["dimmer"].to<JsonObject>());
            if (_relay)
              _relay->toJson(root["relay"].to<JsonObject>());
          }
#endif

          // dimmer

          bool isDimmerOnline() const {
            return _dimmer->isOnline();
          }
          bool isDimmerEnabled() const {
            return _dimmer->isEnabled();
          }
          bool isAutoDimmerEnabled() const {
            return config.autoDimmer && config.calibratedResistance > 0 && !isBypassOn();
          }
          bool isDimmerTemperatureLimitReached() const {
            return config.dimmerTempLimit > 0 && _temperature.orElse(0) >= config.dimmerTempLimit;
          }
          bool isDimmerOn() const {
            return _dimmer->isOn();
          }
          float getDimmerDutyCycle() const {
            return _dimmer->getDutyCycle();
          }
          float getDimmerDutyCycleOnline() const {
            return _dimmer->isOnline() ? _dimmer->getDutyCycle() : 0;
          }
          float getDimmerDutyCycleLimit() const {
            return _dimmer->getDutyCycleLimit();
          }
          // Power Duty Cycle [0, 1]
          // At 0% power, duty == 0
          // At 100% power, duty == 1
          bool setDimmerDutyCycle(float dutyCycle);
          bool setDimmerOff() {
            return setDimmerDutyCycle(0);
          }
          void setDimmerDutyCycleMin(float min) {
            _dimmer->setDutyCycleMin(min);
          }
          void setDimmerDutyCycleMax(float max) {
            _dimmer->setDutyCycleMax(max);
          }
          void setDimmerDutyCycleLimit(float limit) {
            _dimmer->setDutyCycleLimit(limit);
          }
          void applyTemperatureLimit();

          // preconditions:
          // - gridVoltage > 0
          // - availablePowerToDivert > 0
          // - config.calibratedResistance > 0
          float divert(float gridVoltage, float availablePowerToDivert) {
            // maximum power of the load based on the calibrated resistance value
            const float maxPower = gridVoltage * gridVoltage / config.calibratedResistance;

            // cap with maxPower
            if (availablePowerToDivert > maxPower)
              availablePowerToDivert = maxPower;

            // apply the excess power limiter
            if (config.excessPowerLimiter > 0 && availablePowerToDivert > config.excessPowerLimiter)
              availablePowerToDivert = config.excessPowerLimiter;

            // try to apply duty
            _dimmer->setDutyCycle(availablePowerToDivert / maxPower);

            // returns the real used power as per the dimmer state
            return availablePowerToDivert;
          }

          // returns true if the output was consuming but then stopped consuming
          bool wasConsuming() const {
            return !_lastTimeConsumptionWasDetectedWhileRouting.neverUpdated() && !_lastTimeConsumptionWasDetectedWhileRouting.isPresent();
          }

          // bypass

          bool isBypassRelayEnabled() const {
            return _relay && _relay->isEnabled();
          }
          bool isBypassRelayOn() const {
            return _relay && _relay->isOn();
          }
          bool isAutoBypassEnabled() const {
            return config.autoBypass;
          }
          bool isBypassOn() const {
            return _autoBypassEnabled || _manualBypassEnabled;
          }
          void setBypass(bool state);
          void setBypassOn() {
            setBypass(true);
          }
          void setBypassOff() {
            setBypass(false);
          }
          uint32_t getBypassUptime() const {
            return config.bypassTimeoutSec && _manualBypassTime ? (millis() - _manualBypassTime) / 1000UL : 0;
          }
          void applyAutoBypass();
          void applyBypassTimeout();
          uint64_t getBypassRelaySwitchCount() const {
            return _relay ? _relay->getSwitchCount() : 0;
          }

          // metrics

          void updateMetrics(std::unique_ptr<Mycila::metric::Metrics> metrics) override {
            Mycila::metric::MetricSupport::updateMetrics(std::move(metrics));
            // update the last time we detected some consumption on the output (used to detect a load that stopped consuming)
            if (getState() == State::ROUTING && isAutoDimmerEnabled() && _metrics.get()->power >= MYCILA_OUTPUT_LOW_POWER_THRESHOLD) {
              _lastTimeConsumptionWasDetectedWhileRouting.update(_metrics.get()->power);
            }
          }

          std::optional<float> getRoutedPower(float gridVoltage) const {
            if (getState() != State::ROUTING)
              return 0.0f;
            if (_metrics.isPresent())
              return _metrics.get()->power;
            if (gridVoltage > 0 && config.calibratedResistance > 0) {
              Mycila::Dimmer::Metrics dimmerMetrics;
              if (_dimmer->calculateMetrics(dimmerMetrics, gridVoltage, config.calibratedResistance)) {
                return dimmerMetrics.power;
              }
            }
            return std::nullopt;
          }

          std::optional<float> getRoutedCurrent(float gridVoltage) const {
            if (getState() != State::ROUTING)
              return 0.0f;
            if (_metrics.isPresent())
              return _metrics.get()->current;
            if (gridVoltage > 0 && config.calibratedResistance > 0) {
              Mycila::Dimmer::Metrics dimmerMetrics;
              if (_dimmer->calculateMetrics(dimmerMetrics, gridVoltage, config.calibratedResistance)) {
                return dimmerMetrics.current;
              }
            }
            return std::nullopt;
          }

          std::optional<float> measureResistance() const {
            if (_metrics.isPresent())
              return _metrics.get()->resistance;
            return std::nullopt;
          }

          bool getRoutedMetrics(metric::Metrics& metrics, float gridVoltage) const {
            return readRoutedMetrics(metrics) || computeRoutedMetrics(metrics, gridVoltage);
          }

          bool readRoutedMetrics(metric::Metrics& metrics) const {
            metrics.reset(0.0f);
            if (_metrics.isPresent()) {
              if (getState() == State::ROUTING) {
                memcpy(&metrics, _metrics.get().get(), sizeof(metric::Metrics));
              } else {
                metrics.energy = _metrics.get()->energy;
              }
              return true;
            }
            return false;
          }

          bool computeRoutedMetrics(metric::Metrics& metrics, float gridVoltage) const {
            metrics.reset(0.0f);
            if (gridVoltage > 0 && config.calibratedResistance > 0) {
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
                }
              }
              return true;
            }
            return false;
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

          ExpiringValue<float>& temperature() {
            return _temperature;
          }
          const ExpiringValue<float>& temperature() const {
            return _temperature;
          }

        public:
          Config config;

        private:
          const char* _name;
          const char* _mqttName;
          Dimmer* _dimmer;
          Mycila::Relay* _relay = nullptr; // optional
          bool _autoBypassEnabled = false;
          bool _manualBypassEnabled = false;
          uint32_t _manualBypassTime = 0;
          ExpiringValue<float> _temperature;
          // used to detect a load that stopped consuming while the dimmer is still on
          ExpiringValue<float> _lastTimeConsumptionWasDetectedWhileRouting;

        private:
          friend class Router;
          void _switchBypass(bool state, bool log = true);
          void _turnOffDimmer() {
            _dimmer->off();
            _lastTimeConsumptionWasDetectedWhileRouting.reset();
          }
      };

      ////////////
      // Router //
      ////////////

      void addOutput(Output& output) {
        _outputs.push_back(&output);
      }
      const std::vector<Output*>& getOutputs() const {
        return _outputs;
      }

      bool isAutoDimmerEnabled() const {
        for (const auto& output : _outputs) {
          if (output->isAutoDimmerEnabled()) {
            return true;
          }
        }
        return false;
      }

      void noDivert() { divert(0, 0); }

      // Note: can only be called if !isCalibrationRunning()
      float divert(const float gridVoltage, const float powerToDivert) {
        // if there is no voltage or no power to divert then we turn off all dimmers
        // - ensures gridVoltage > 0
        // - ensures powerToDivert > 0
        if (powerToDivert <= 0 || gridVoltage <= 0) {
          for (const auto& output : _outputs) {
            output->_turnOffDimmer();
          }
          return 0.0f;
        }

        // copy the output vector
        std::vector<Output*> remainingOutputs = _outputs;

        // filter dimmers
        for (const auto& output : _outputs) {
          // filter out dimmers that are not automatically dimmable
          // - ensures config.calibratedResistance > 0 with isAutoDimmerEnabled()
          if (!output->isDimmerEnabled() || !output->isDimmerOnline() || !output->isAutoDimmerEnabled()) {
            remainingOutputs.erase(std::remove(remainingOutputs.begin(), remainingOutputs.end(), output), remainingOutputs.end());
            continue;
          }
          // filter out dimmers that have reached their temperature limit or have a ratio of 0 set and make sure they are turned off
          if (output->config.excessPowerRatio == 0 || output->isDimmerTemperatureLimitReached()) {
            output->_turnOffDimmer();
            remainingOutputs.erase(std::remove(remainingOutputs.begin(), remainingOutputs.end(), output), remainingOutputs.end());
          }
        }

        // if there is no remaining output to divert then we return
        if (remainingOutputs.empty()) {
          return 0.0f;
        }

        // Filter out dimmers that were consuming before but stopped consuming while we were routing power to them.
        // We will set their dimmer to 1% (100W) in order to keep the dimmer opened in case they start consuming again.
        // But we won't decrease the 100W from the available power to divert because we will use it for the other outputs.
        // We only do that if we have more than 1 remaining output.
        if (remainingOutputs.size() > 1) {
          for (auto it = remainingOutputs.begin(); it != remainingOutputs.end();) {
            Output* output = *it;
            if (output->wasConsuming()) {
              // Notes:
              // - we have ensured gridVoltage > 0
              // - we have ensured config.calibratedResistance > 0 with isAutoDimmerEnabled()
              output->divert(gridVoltage, 100.0f);
              it = remainingOutputs.erase(it);
            } else {
              ++it;
            }
          }
        }

        // If there is no remaining output to divert because they have all stopped consuming then we return.
        if (remainingOutputs.empty()) {
          return 0.0f;
        }

        float routedPower = 0.0f;

        // At this point, the remainingOutputs list contains all outputs that are supposed to consume or
        // have consumed recently and that we can divert to.
        for (const auto& output : remainingOutputs) {
          // compute the remaining power to divert for this output after diverting to the previous ones
          const float remain = std::max(0.0f, powerToDivert - routedPower);

          // remain could be zero: this can happen with the second output if the first one takes everything
          if (remain == 0.0f) {
            output->_turnOffDimmer();
            continue;
          }

          // Apply the ratio of excess power sharing configured for this dimmer.
          // - we have ensured remaining powerToDivert > 0
          // - we have ensured config.excessPowerRatio > 0
          // So ratio will be > 0
          const float ratio = remain * output->config.excessPowerRatio;

          // - we have ensured gridVoltage > 0
          // - we have ensured config.calibratedResistance > 0 with isAutoDimmerEnabled()
          routedPower += output->divert(gridVoltage, ratio);
        }

        return routedPower;
      }

      typedef std::function<void()> CalibrationCallback;
      void beginCalibration(size_t outputIndex, CalibrationCallback cb = nullptr);
      void continueCalibration();
      bool isCalibrationRunning() const {
        return _calibrationRunning;
      }
      bool isCalibrationRunning(size_t outputIndex) const {
        return _calibrationRunning && _calibrationOutputIndex == outputIndex;
      }
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
        JsonArray json = root["metrics"].to<JsonArray>();
        std::unique_ptr<metric::Metrics> metrics = std::make_unique<metric::Metrics>();
        if (computeRoutedMetrics(*metrics, gridVoltage)) {
          JsonObject source = json.add<JsonObject>();
          source["source"] = metric::sourceToString(metric::Source::COMPUTED);
          metric::Metrics::toJson(source, *metrics);
        }
        if (readRoutedMetrics(*metrics)) {
          JsonObject source = json.add<JsonObject>();
          source["source"] = "Measured";
          metric::Metrics::toJson(source, *metrics);
        }
      }
#endif

      bool getRoutedMetrics(metric::Metrics& metrics, float gridVoltage) const {
        return readRoutedMetrics(metrics) || computeRoutedMetrics(metrics, gridVoltage);
      }

      // get router measurements based on the connected JSY (for an aggregated view of all outputs) or PZEM per output
      bool readRoutedMetrics(metric::Metrics& metrics) const {
        metrics.reset(0.0f);
        // check if outputs are sharing the same measurement clamp.
        const bool shared = std::any_of(_outputs.begin(), _outputs.end(), [](const auto& output) {
          return output->isUsing(metric::Source::SHARED);
        });
        // check if at least one output is routing
        const bool routing = std::any_of(_outputs.begin(), _outputs.end(), [](const auto& output) {
          return output->getState() == Output::State::ROUTING;
        });
        for (size_t i = 0; i < _outputs.size(); i++) {
          // if this output measurement source is shared (same clamp, 2 wires) then ignore
          if (_outputs[i]->isUsing(metric::Source::SHARED))
            continue;
          metric::Metrics outputMetrics;
          if (shared && routing ? _outputs[i]->readMetrics(outputMetrics) : _outputs[i]->readRoutedMetrics(outputMetrics)) {
            // Note: energy is not accurate in the case of a virtual bypass through dimmer
            metrics.energy += outputMetrics.energy;
            metrics.apparentPower += outputMetrics.apparentPower;
            metrics.current += outputMetrics.current;
            metrics.power += outputMetrics.power;
            metrics.frequency = outputMetrics.frequency; // should be the same for all outputs
          } else {
            // The other output does not have a measurement device setup.
            // We won't have a valid aggregated output since it will be ignored.
            // Note :we do not mix measurements with calculated metrics
          }
        }
        metrics.powerFactor = metrics.apparentPower > 0 ? metrics.power / metrics.apparentPower : 0;
        metrics.resistance = metrics.current > 0 ? metrics.power / (metrics.current * metrics.current) : 0;
        metrics.thdi = metrics.powerFactor > 0 ? 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f) : 0;
        return true;
      }

      bool computeRoutedMetrics(metric::Metrics& metrics, float gridVoltage) const {
        metrics.reset(0.0f);
        if (gridVoltage > 0) {
          for (size_t i = 0; i < _outputs.size(); i++) {
            metric::Metrics outputMetrics;
            if (_outputs[i]->computeRoutedMetrics(outputMetrics, gridVoltage)) {
              metrics.energy += outputMetrics.energy;
              metrics.apparentPower += outputMetrics.apparentPower;
              metrics.current += outputMetrics.current;
              metrics.power += outputMetrics.power;
            }
          }
          metrics.powerFactor = metrics.apparentPower > 0 ? metrics.power / metrics.apparentPower : 0;
          metrics.resistance = metrics.current > 0 ? metrics.power / (metrics.current * metrics.current) : 0;
          metrics.thdi = metrics.powerFactor > 0 ? 100.0f * std::sqrt(1.0f / (metrics.powerFactor * metrics.powerFactor) - 1.0f) : 0;
          return true;
        }
        return false;
      }

      std::optional<float> getTotalRoutedPower(float gridVoltage) const {
        float power = 0;
        for (size_t i = 0; i < _outputs.size(); i++) {
          // if this output measurement source is shared (same clamp, 2 wires) then ignore
          if (_outputs[i]->isUsing(metric::Source::SHARED))
            continue;
          std::optional<float> outputPower = _outputs[i]->getRoutedPower(gridVoltage);
          if (outputPower.has_value()) {
            power += outputPower.value();
          } else {
            // otherwise we do not have any valid total power
            return std::nullopt;
          }
        }
        return power;
      }

    private:
      std::vector<Output*> _outputs;

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
