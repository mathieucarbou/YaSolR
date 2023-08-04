// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

namespace YaSolR {
  class HADiscoveryControllerClass {
    public:
      void publish();
  };

  extern YaSolR::HADiscoveryControllerClass HADiscoveryController;
} // namespace YaSolR
