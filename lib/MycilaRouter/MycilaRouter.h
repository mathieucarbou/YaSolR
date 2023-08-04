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
      void setOutputs(const std::vector<RouterOutput*>& outputs) { _outputs = outputs; }

      void toJson(const JsonObject& root) const;

      float getVirtualGridPower() const;
      float getTotalRoutedPower() const;
      float getTotalPowerFactor() const;
      float getTotalTHDi() const;
      float getTotalRoutedEnergy() const;

      bool isRouting() const;

    private:
      std::vector<RouterOutput*> _outputs = {};
  };

  extern RouterClass Router;
} // namespace Mycila
