// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRelayManager.h>
#include <MycilaRouterOutput.h>

namespace YaSolR {
  class Connector {
    public:
      explicit Connector(Mycila::TemperatureSensor* systemTempSensor) : _systemTempSensor(systemTempSensor) {}

      void begin();
      void loop();

      void publish();

    private:
      Mycila::TemperatureSensor* _systemTempSensor;
      uint32_t _lastPublish = 0;
      bool _publishHADiscovery = false;

    private:
      void _publish();
  };
} // namespace YaSolR
