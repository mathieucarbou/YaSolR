// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaTemperatureSensor.h>

#include <YaSolRConnector.h>
#include <YaSolRWebsite.h>
#include <MycilaThrottle.h>

#include <queue>

namespace YaSolR {
  enum class Event {
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
    POWER_MANAGER_RECONFIGURED,
    SYS_TEMP_RECONFIGURED,
    BUTTON_RECONFIGURED,
    NETWORK_RECONFIGURED,
    NETWORK_CONNECTED,
    NETWORK_DISCONNECTED,
    DISPLAY_RECONFIGURED
  };

  class Controller {
    public:
      explicit Controller(Mycila::TemperatureSensor* systemTemp,
                          YaSolR::Website* website,
                          YaSolR::Connector* connector) : _systemTemp(systemTemp),
                                                          _website(website),
                                                          _connector(connector) {}

      void begin();
      void loop();
      void updateDisplay();

    private:
      Mycila::TemperatureSensor* _systemTemp;
      YaSolR::Website* _website;
      YaSolR::Connector* _connector;
      std::queue<Event> _events;
      Mycila::Throttle _restartThrottle = Mycila::Throttle(false);
      Mycila::Throttle _resetThrottle = Mycila::Throttle(false);
      Mycila::Throttle _updateOnceThrottle = Mycila::Throttle(false);
      uint32_t _lastUpdate = 0;

    private:
      void _triggerRefresh();
  };
} // namespace YaSolR
