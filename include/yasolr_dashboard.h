// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <yasolr.h>

#include <string>
#include <unordered_map>

namespace YaSolR {
  class Website {
    public:
      void begin();
      void initCards();
      void updateCards();
      void updateWarnings();
      void updateCharts();
      void updatePIDCharts();
      void resetPIDCharts();
      bool realTimePIDEnabled() const;
      void setSafeBootUpdateStatus(const char* msg, dash::Status status);

    private:
      void _boolConfig(dash::ToggleButtonCard& card, const char* key) {
        card.onChange([key, &card, this](bool value) {
          config.set<bool>(key, value);
          card.setValue(config.get<bool>(key));
          dashboardInitTask.resume();
        });
      }

      template <typename T = Mycila::config::Value, std::enable_if_t<std::is_integral_v<T>, bool> = true>
      void _sliderConfig(dash::SliderCard<T>& card, const char* key) {
        card.onChange([key, &card](const T& value) {
          config.set<T>(key, value);
          card.setValue(config.get<T>(key));
          dashboardInitTask.resume();
        });
      }

      void _outputDimmerSlider(dash::SliderCard<float, 2>& card, Mycila::Router::Output& output) {
        card.onChange([&card, &output, this](float value) {
          output.setDimmerDutyCycle(value / 100.0f);
          card.setValue(output.getDimmerDutyCycle() * 100.0f);
          dashboardInitTask.resume();
        });
      }

      void _outputBypassSwitch(dash::ToggleButtonCard& card, Mycila::Router::Output& output) {
        card.onChange([&card, &output, this](bool value) {
          output.setBypass(value);
          card.setValue(output.isBypassOn());
          dashboardInitTask.resume();
        });
      }

#ifdef APP_MODEL_PRO
      void _daysConfig(dash::WeekCard<const char*>& card, const char* key) {
        card.onChange([key, &card, this](const char* value) {
          config.setString(key, value[0] ? value : YASOLR_WEEK_DAYS_EMPTY);
          card.setValue(value);
          dashboardInitTask.resume();
        });
      }

      template <typename T = Mycila::config::Value, std::enable_if_t<std::is_integral_v<T>, bool> = true>
      void _numConfig(dash::InputCard<T>& card, const char* key) {
        card.onChange([key, &card](const std::optional<T>& value) {
          if (value.has_value()) {
            config.set<T>(key, value.value());
          } else {
            config.unset(key);
          }
          card.setValue(config.get<T>(key));
          dashboardInitTask.resume();
        });
      }

      template <uint8_t Precision>
      void _numConfig(dash::InputCard<float, Precision>& card, const char* key) {
        card.onChange([key, &card](const std::optional<float>& value) {
          if (value.has_value()) {
            config.set<float>(key, value.value());
          } else {
            config.unset(key);
          }
          card.setValue(config.get<float>(key));
          dashboardInitTask.resume();
        });
      }

      template <typename T = Mycila::config::Value, std::enable_if_t<std::is_integral_v<T>, bool> = true>
      void _numConfig(dash::DropdownCard<T>& card, const char* key) {
        card.onChange([key, &card](const T& value) {
          config.set<T>(key, value);
          card.setValue(config.get<T>(key));
          // GPIO page may need to be revalidated
          dashboardInitTask.resume();
        });
      }

      void _passwordConfig(dash::PasswordCard& card, const char* key) {
        card.onChange([key, &card, this](const std::optional<const char*>& value) {
          config.setString(key, value.value_or(""));
          card.setValue(config.getString(key));
          dashboard.refresh(card);
        });
      }

      void _textConfig(dash::InputCard<const char*>& card, const char* key) {
        card.onChange([key, &card](const std::optional<const char*>& value) {
          if (value.has_value() && value.value()[0] != '\0') {
            config.setString(key, value.value());
          } else {
            config.unset(key);
          }
          card.setValue(config.getString(key));
          dashboard.refresh(card);
        });
      }

      void _textConfig(dash::DropdownCard<const char*>& card, const char* key) {
        card.onChange([key, &card](const char* value) {
          config.setString(key, value);
          card.setValue(config.getString(key));
          dashboard.refresh(card);
        });
      }

      void _rangeConfig(dash::RangeSliderCard<uint8_t>& card, const char* keyMin, const char* keyMax) {
        card.onChange([keyMin, keyMax, &card](const dash::Range<uint8_t>& range) {
          config.set<uint8_t>(keyMin, range.low());
          config.set<uint8_t>(keyMax, range.high());
          card.setValue({config.get<uint8_t>(keyMin), config.get<uint8_t>(keyMax)});
          dashboard.refresh(card);
        });
      }

      void _pinout(dash::FeedbackInputCard<int8_t>& card, const char* key, std::unordered_map<int8_t, dash::FeedbackInputCard<int8_t>*>& pinout) {
        int8_t pin = config.get<int8_t>(key);
        card.setValue(pin);
        if (pin == GPIO_NUM_NC) {
          card.setFeedback("(" YASOLR_LBL_115 ")", dash::Status::INFO);
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
#endif
  };
} // namespace YaSolR

extern YaSolR::Website website;
