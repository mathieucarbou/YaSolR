// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <YaSolR.h>

#include <string>
#include <unordered_map>

namespace YaSolR {
  class Website {
    public:
      void initLayout();
      void initCards();
      void updateCards();
      void updateCharts();
      void updatePID();
      void resetPID();

    private:
      void _boolConfig(dash::SwitchCard& card, const char* key) {
        card.onChange([key, &card, this](bool value) {
          config.setBool(key, value);
          card.setValue(config.getBool(key) ? 1 : 0);
          dashboard.refresh(card);
          dashboardInitTask.resume();
        });
      }

      template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
      void _sliderConfig(dash::SliderCard<T>& card, const char* key) {
        card.onChange([key, &card](const T& value) {
          config.set(key, std::to_string(value));
          card.setValue(static_cast<T>(config.getInt(key)));
          dashboard.refresh(card);
        });
      }

      void _outputDimmerSlider(dash::SliderCard<float, 2>& card, Mycila::RouterOutput& output) {
        card.onChange([&card, &output, this](float value) {
          if (output.isDimmerEnabled()) {
            output.setDimmerDutyCycle(value / 100);
          }
          card.setValue(output.getDimmerDutyCycle() * 100);
          dashboard.refresh(card);
          dashboardUpdateTask.requestEarlyRun();
        });
      }

      void _outputBypassSwitch(dash::SwitchCard& card, Mycila::RouterOutput& output) {
        card.onChange([&card, &output, this](bool value) {
          if (output.isBypassEnabled()) {
            output.setBypass(value);
          }
          card.setValue(output.isBypassOn());
          dashboard.refresh(card);
          dashboardUpdateTask.requestEarlyRun();
        });
      }

      void _relaySwitch(dash::SwitchCard& card, Mycila::RouterRelay& relay) {
        card.onChange([&card, &relay, this](bool value) {
          relay.tryRelayState(value);
          card.setValue(relay.isOn());
          dashboard.refresh(card);
        });
      }

#ifdef APP_MODEL_PRO
      void _daysConfig(dash::WeekCard<const char*>& card, const char* key) {
        card.onChange([key, &card, this](const char* value) {
          config.set(key, value[0] ? value : YASOLR_WEEK_DAYS_EMPTY);
          card.setValue(value);
          dashboard.refresh(card);
        });
      }

      template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
      void _numConfig(dash::TextInputCard<T>& card, const char* key) {
        card.onChange([key, &card](const std::optional<T>& value) {
          if (value.has_value()) {
            config.set(key, std::to_string(value.value()));
          } else {
            config.unset(key);
          }
          card.setValue(static_cast<T>(config.getInt(key)));
          dashboard.refresh(card);
        });
      }

      template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
      void _numConfig(dash::DropdownCard<T>& card, const char* key) {
        card.onChange([key, &card](const T& value) {
          config.set(key, std::to_string(value));
          card.setValue(config.getInt(key));
          dashboard.refresh(card);
        });
      }

      void _pinConfig(dash::FeedbackTextInputCard<int32_t>& card, const char* key) {
        card.onChange([key, &card, this](const std::optional<int32_t> value) {
          if (value.has_value()) {
            config.set(key, std::to_string(value.value()));
          } else {
            config.unset(key);
          }
          card.setValue(config.getInt(key));
          initCards();
          dashboard.refresh(card);
        });
      }

      void _passwordConfig(dash::PasswordCard& card, const char* key) {
        card.onChange([key, &card, this](const std::optional<const char*>& value) {
          if (value.has_value()) {
            config.set(key, value.value());
            card.setValue(value.value()); // will be replaced by stars
          } else {
            config.unset(key);
            card.removeValue();
          }
          dashboard.refresh(card);
        });
      }

      void _textConfig(dash::DropdownCard<const char*>& card, const char* key) {
        card.onChange([key, &card](const char* value) {
          config.set(key, value);
          card.setValue(config.get(key));
          dashboard.refresh(card);
        });
      }

      void _textConfig(dash::TextInputCard<const char*>& card, const char* key) {
        card.onChange([key, &card](const std::optional<std::string>& value) {
          if (value.has_value()) {
            config.set(key, value.value());
            card.setValue(config.get(key));
          } else {
            config.unset(key);
            card.removeValue();
          }
          dashboard.refresh(card);
        });
      }

      void _pinout(dash::FeedbackTextInputCard<int32_t>& card, const char* key, std::unordered_map<int32_t, dash::FeedbackTextInputCard<int32_t>*>& pinout) {
        int32_t pin = config.getInt(key);
        card.setValue(pin);
        if (pin == GPIO_NUM_NC) {
          card.setFeedback("(" YASOLR_LBL_115 ")", dash::Status::IDLE);
        } else if (pinout.find(pin) != pinout.end()) {
          pinout[pin]->setFeedback("(" YASOLR_LBL_153 ")", dash::Status::DANGER);
          card.setFeedback("(" YASOLR_LBL_153 ")", dash::Status::DANGER);
        } else if (!GPIO_IS_VALID_GPIO(pin)) {
          pinout[pin] = &card;
          card.setFeedback("(" YASOLR_LBL_154 ")", dash::Status::DANGER);
        } else if (!GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
          pinout[pin] = &card;
          card.setFeedback("(" YASOLR_LBL_155 ")", dash::Status::WARNING);
        } else {
          pinout[pin] = &card;
          card.setFeedback("(" YASOLR_LBL_156 ")", dash::Status::SUCCESS);
        }
      }

      void _status(dash::FeedbackSwitchCard& card, const char* key, bool enabled, bool active = true, const char* err = "") {
        const bool configEnabled = config.getBool(key);
        card.setValue(configEnabled);
        if (!configEnabled)
          card.setFeedback(YASOLR_LBL_115, dash::Status::IDLE);
        else if (!enabled)
          card.setFeedback(YASOLR_LBL_124, dash::Status::DANGER);
        else if (!active)
          card.setFeedback(err, dash::Status::WARNING);
        else
          card.setFeedback(YASOLR_LBL_130, dash::Status::SUCCESS);
      }
#endif
  };
} // namespace YaSolR
