// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaRouterOutput.h>

#include <ArduinoJson.h>
#include <vector>

namespace Mycila {
  class RouterClass {
    public:
      void begin();
      void loop();
      void end();

      void toJson(const JsonObject& root) const;

      float getVirtualGridPower() const;
      float getTotalRoutedPower() const;
      float getTotalPowerFactor() const;
      float getTotalTHDi() const;
      float getTotalRoutedEnergy() const;

      float getOutputPower(int idx) const;
      float getOutputResistance(int idx) const;

      inline bool isRouting() const { return outputs[0].getState() == RouterOutputState::OUTPUT_ROUTING || outputs[1].getState() == RouterOutputState::OUTPUT_ROUTING; }

    public:
      std::vector<RouterOutput> outputs;
      uint8_t _loopIdx = 0;
  };

  extern RouterClass Router;
} // namespace Mycila
