// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaDimmer.h>
#include <MycilaEasyDisplay.h>
#include <MycilaMQTT.h>

#include <YaSolR.h>

namespace YaSolR {
  class YaSolRClass {
    public:
      void begin() {
        _initConfig();
        _initHTTPd();
        _initTasks();
        _initEventHandlers();
        _initWebsite();
        _initREST();
        _initMQTT();
      }

      Mycila::EasyDisplayType getDisplayType() const;
      uint32_t getDisplayRotation() const;
      Mycila::DimmerType getDimmerType(const char* name) const;
      Mycila::MQTTConfig getMQTTConfig() const;

      void updateDisplay();
      void updateWebsite();
      void publishHADiscovery();
      void publishMQTT();

    private:
      void _initConfig();
      void _initHTTPd();
      void _initTasks();
      void _initEventHandlers();
      void _initWebsite();
      void _initREST();
      void _initMQTT();
  };

  extern YaSolRClass YaSolR;
} // namespace YaSolR
