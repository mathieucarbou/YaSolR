// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaEasyDisplay.h>

#include <YaSolR.h>

namespace YaSolR {
  enum class Event {
    NONE,
    MQTT_RECONFIGURED,
    NTP_RECONFIGURED,
    RELAY1_RECONFIGURED,
    RELAY2_RECONFIGURED,
    BUZZER_RECONFIGURED,
    LIGHTS_RECONFIGURED,
    OUT1_TEMP_RECONFIGURED,
    OUT1_RELAY_RECONFIGURED,
    OUT1_DIMMER_RECONFIGURED,
    OUT2_TEMP_RECONFIGURED,
    OUT2_RELAY_RECONFIGURED,
    OUT2_DIMMER_RECONFIGURED,
    JSY_RECONFIGURED,
    ZCD_RECONFIGURED,
    GRID_RECONFIGURED,
    SYS_TEMP_RECONFIGURED,
    BUTTON_RECONFIGURED,
    NETWORK_RECONFIGURED,
    DISPLAY_RECONFIGURED,
  };

  class YaSolRClass {
    public:
      void begin() {
        _initConfig();
        _initEventHandlers();
        _initREST();
        _initMQTT();
      }

      Mycila::EasyDisplayType getDisplayType() const;
      uint32_t getDisplayRotation() const;
      uint32_t getRelayPauseDuration() const;

      void updateDisplay();
      void publishHADiscovery();
      void publishMQTT();

    private:
      void _initConfig();
      void _initEventHandlers();
      void _initREST();
      void _initMQTT();
  };

  extern YaSolRClass YaSolR;
} // namespace YaSolR
