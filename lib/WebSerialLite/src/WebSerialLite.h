#ifndef WebSerial_h
#define WebSerial_h

#include <functional>

#include <Arduino.h>
#include <stdlib_noniso.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#ifndef WEBSERIAL_MAX_WS_CLIENTS
#define WEBSERIAL_MAX_WS_CLIENTS DEFAULT_MAX_WS_CLIENTS
#endif

typedef std::function<void(AsyncWebSocketClient*, const String& msg)> RecvMsgHandler;

class WebSerialClass : public Print {
  public:
    void begin(AsyncWebServer* server, const char* url = "/webserial", const String& username = "", const String& password = "");

    void onMessage(RecvMsgHandler callbackFunc);

    // Print

    size_t write(uint8_t);
    size_t write(const uint8_t* buffer, size_t size);

  private:
    AsyncWebServer* _server;
    AsyncWebSocket* _ws;
    RecvMsgHandler _recvMsgCallback = NULL;
    String _username;
    String _password;
    bool _auth;
};

extern WebSerialClass WebSerial;
#endif
