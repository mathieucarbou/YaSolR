// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaHTTPd.h>
#include <MycilaLogger.h>

#ifndef MYCILA_API_PATH
#define MYCILA_API_PATH "/api"
#endif

#define TAG "HTTP"

void Mycila::HTTPdClass::init(AsyncWebServer* server, const String& username, const String& password) {
  if (_server)
    return;

  _server = server;
  _username = username;
  _password = password;
  _auth = !_username.isEmpty() && !_password.isEmpty();

  Logger.info(TAG, "Initializing Web Server...");
  Logger.debug(TAG, "- Authentication: %s", _auth ? "true" : "false");
  Logger.debug(TAG, "- Username: %s", _username.c_str());
  Logger.debug(TAG, "- Password: %s", _password.isEmpty() ? "" : "********");
}

AsyncWebHandler& Mycila::HTTPdClass::get(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  Logger.debug(TAG, "[GET   ] %s", path);
  return _authenticate(&_server->on(path, HTTP_GET, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::post(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  Logger.debug(TAG, "[POST  ] %s", path);
  return _authenticate(&_server->on(path, HTTP_POST, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::del(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  Logger.debug(TAG, "[DELETE] %s", path);
  return _authenticate(&_server->on(path, HTTP_DELETE, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::any(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  Logger.debug(TAG, "[ANY   ] %s", path);
  return _authenticate(&_server->on(path, HTTP_ANY, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::apiGET(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  String url;
  url.reserve(strlen(MYCILA_API_PATH) + strlen(path));
  url += MYCILA_API_PATH;
  url += path;
  Logger.debug(TAG, "[GET   ] %s", url.c_str());
  return _authenticate(&_server->on(url.c_str(), HTTP_GET, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::apiPOST(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  String url;
  url.reserve(strlen(MYCILA_API_PATH) + strlen(path));
  url += MYCILA_API_PATH;
  url += path;
  Logger.debug(TAG, "[POST  ] %s", url.c_str());
  return _authenticate(&_server->on(url.c_str(), HTTP_POST, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::apiUPLOAD(const char* path, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, bool anonymous) {
  String url;
  url.reserve(strlen(MYCILA_API_PATH) + strlen(path));
  url += MYCILA_API_PATH;
  url += path;
  Logger.debug(TAG, "[UPLOAD] %s", url.c_str());
  return _authenticate(&_server->on(url.c_str(), HTTP_POST, onRequest, onUpload), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::apiDELETE(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  String url;
  url.reserve(strlen(MYCILA_API_PATH) + strlen(path));
  url += MYCILA_API_PATH;
  url += path;
  Logger.debug(TAG, "[DELETE] %s", url.c_str());
  return _authenticate(&_server->on(url.c_str(), HTTP_DELETE, onRequest), anonymous);
}

AsyncWebHandler& Mycila::HTTPdClass::apiANY(const char* path, ArRequestHandlerFunction onRequest, bool anonymous) {
  String url;
  url.reserve(strlen(MYCILA_API_PATH) + strlen(path));
  url += MYCILA_API_PATH;
  url += path;
  Logger.debug(TAG, "[ANY   ] %s", url.c_str());
  return _authenticate(&_server->on(url.c_str(), HTTP_ANY, onRequest), anonymous);
}

AsyncWebSocket& Mycila::HTTPdClass::webSocket(const char* path) {
  AsyncWebSocket* ws = new AsyncWebSocket(path);
  _authenticate(ws, false);
  _server->addHandler(ws);
  return *ws;
}

AsyncWebHandler& Mycila::HTTPdClass::_authenticate(AsyncWebHandler* handler, bool anonymous) {
  return handler->setAuthentication(anonymous ? "" : (_auth ? _username.c_str() : ""), anonymous ? "" : (_auth ? _password.c_str() : ""));
}

namespace Mycila {
  HTTPdClass HTTPd;
} // namespace Mycila
