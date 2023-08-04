// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <stdlib.h>

namespace YaSolR {
  class ConfigClass {
    public:
      void begin();
  };

  extern YaSolR::ConfigClass Config;
} // namespace YaSolR
