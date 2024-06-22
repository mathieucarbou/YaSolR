// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include "MycilaJSY.h"

#include <AsyncUDP.h>

namespace Mycila {
  class JSYClientUDP {
    public:
      typedef struct {
          float _current1 = 0;        // A
          float _current2 = 0;        // A
          float _energy1 = 0;         // kWh
          float _energy2 = 0;         // kWh
          float _energyReturned1 = 0; // kWh
          float _energyReturned2 = 0; // kWh
          float _frequency = 0;       // Hz
          float _power1 = 0;          // W
          float _power2 = 0;          // W
          float _powerFactor1 = 0;
          float _powerFactor2 = 0;
          float _voltage1 = 0; // V
          float _voltage2 = 0; // V
      } __attribute__((packed)) Data;

      ~JSYClientUDP() { end(); }

      bool begin(JSY& jsy, // NOLINT
                 const String& address = "239.3.2.1",
                 uint16_t port = 54321);

      void end();

      bool isEnabled();
      bool isCOnnected();

    private:
      bool _enabled = false;
      AsyncUDP _udp;
  };
} // namespace Mycila
