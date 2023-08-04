// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ESPAsyncWebServer.h>

namespace Mycila {
  class HTTPdConfigClass {
    public:
      bool isEnabled() const;
      String getUsername() const;
      String getPassword() const;
  };

  class HTTPdClass {
    public:
      void begin();
      void end();

      AsyncWebHandler& get(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);
      AsyncWebHandler& post(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);
      AsyncWebHandler& del(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);
      AsyncWebHandler& any(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);

      AsyncWebHandler& apiGET(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);
      AsyncWebHandler& apiPOST(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);
      AsyncWebHandler& apiUPLOAD(const char* path, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, bool anonymous = false);
      AsyncWebHandler& apiDELETE(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);
      AsyncWebHandler& apiANY(const char* path, ArRequestHandlerFunction onRequest, bool anonymous = false);

      AsyncWebSocket& webSocket(const char* path);

    public:
      AsyncWebServer server = AsyncWebServer(80);

    private:
      bool _enabled;

    private:
      AsyncWebHandler& _authenticate(AsyncWebHandler* handler, bool anonymous);
  };

  extern HTTPdConfigClass HTTPdConfig;
  extern HTTPdClass HTTPd;
} // namespace Mycila
