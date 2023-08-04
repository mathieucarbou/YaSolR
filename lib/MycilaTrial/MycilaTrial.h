// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <stdint.h>

namespace Mycila {
  class TrialClass {
    public:
      void begin();
      void loop();

    private:
      uint32_t _lastSave = 0;
  };

  extern TrialClass Trial;
} // namespace Mycila
