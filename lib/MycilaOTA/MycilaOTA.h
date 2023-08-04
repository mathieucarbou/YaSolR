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

      String getTitle() const;
      String getFirmwareName() const;
      String getFirmwareVersion() const;
  };

  class OTAClass {
    public:
      void listen(OTACompleteCallback callback) { _completeCallback = callback; }
      void listen(OTAPrepareCallback callback) { _prepareCallback = callback; }

      void begin(bool async = false);
      void end();

      // do not call if async is true
      void loop();

      bool isEnabled() const { return _enabled; }
      bool isUpdating() const { return _updating; }

    private:
      bool _enabled = false;
      bool _elegantOTAInitialized = false;
      bool _updating = false;
      OTAPrepareCallback _prepareCallback = nullptr;
      OTACompleteCallback _completeCallback = nullptr;

    private:
      static void _otaTask(void* params);
  };

  extern OTAConfigClass OTAConfig;
  extern OTAClass OTA;
} // namespace Mycila
