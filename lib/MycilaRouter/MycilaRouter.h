// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <MycilaJSY.h>
#include <MycilaRouterOutput.h>

#include <vector>

namespace Mycila {
  class RouterClass {
    public:
      void setJSY(const JSY& jsy) { _jsy = &jsy; }
      void addOutput(const RouterOutput& output) {
        _outputs.reserve(_outputs.size() + 1);
        _outputs.push_back(&output);
      }

      float getVirtualGridPower() const;
      float getTotalRoutedPower() const;
      float getTotalPowerFactor() const;
      float getTotalTHDi() const;
      float getTotalRoutedEnergy() const;

      bool isRouting() const;

      void adjustRouting();

    private:
      const JSY* _jsy = nullptr;
      std::vector<const RouterOutput*> _outputs = {};
  };

  extern RouterClass Router;
} // namespace Mycila
