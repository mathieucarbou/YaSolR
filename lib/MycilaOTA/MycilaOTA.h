// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <WString.h>
#include <functional>

#ifndef MYCILA_OTA_PORT
#define MYCILA_OTA_PORT 3232
#endif

namespace Mycila {
  typedef std::function<void(bool success)> OTACompleteCallback;
  typedef std::function<void()> OTAPrepareCallback;

  class OTAConfigClass {
    public:
      String getHostname() const;
      String getUsername() const;
      String getPassword() const;

      String getID() const;
      String getTitle() const;
      String getFWVersion() const;
  };

  class OTAClass {
    public:
      void listen(OTACompleteCallback callback) { _completeCallback = callback; }
      void listen(OTAPrepareCallback callback) { _prepareCallback = callback; }

      void begin();
      void loop();
      void end();

      bool isEnabled() const { return _enabled; }

    private:
      bool _enabled = false;
      bool _elegantOTAInitialized = false;
      OTAPrepareCallback _prepareCallback = nullptr;
      OTACompleteCallback _completeCallback = nullptr;
  };

  extern OTAConfigClass OTAConfig;
  extern OTAClass OTA;
} // namespace Mycila
