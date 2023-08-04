// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <ESPAsyncWebServer.h>

namespace Mycila {
  class HTTPdClass {
    public:
      void init(AsyncWebServer* server, const String& username = emptyString, const String& password = emptyString);

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

      bool isAuthEnabled() const { return _auth; }
      const String& getUsername() const { return _username; }
      const String& getPassword() const { return _password; }

    private:
      AsyncWebServer* _server = nullptr;
      String _username = emptyString;
      String _password = emptyString;
      bool _auth = false;

    private:
      AsyncWebHandler& _authenticate(AsyncWebHandler* handler, bool anonymous);
  };

  extern HTTPdClass HTTPd;
} // namespace Mycila
