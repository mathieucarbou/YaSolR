// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#pragma once

#include <YaSolR.h>

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
      void _boolConfig(Card& card, const char* key);
      void _daysConfig(Card& card, const char* key);
      void _floatConfig(Card& card, const char* key);
      void _numConfig(Card& card, const char* key);
      void _pinConfig(Card& card, const char* key);
      void _passwordConfig(Card& card, const char* key);
      void _sliderConfig(Card& card, const char* key);
      void _percentageSlider(Card& card, const char* key);
      void _textConfig(Card& card, const char* key);

      void _outputDimmerSlider(Card& card, Mycila::RouterOutput& output);
      void _outputBypassSwitch(Card& card, Mycila::RouterOutput& output);
      void _relaySwitch(Card& card, Mycila::RouterRelay& relay);

      void _pinout(Card& card, int32_t pin, std::unordered_map<int32_t, Card*>& pinout);
      void _status(Card& card, const char* key, bool enabled, bool state = true, const char* err = "");
      void _temperature(Card& card, Mycila::DS18& sensor);
      void _temperature(Card& card, Mycila::RouterOutput& output);
  };
} // namespace YaSolR
