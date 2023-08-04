#include <WebSerialLite.h>
#include <WebSerialWebPage.h>

void WebSerialClass::begin(AsyncWebServer* server, const char* url, const String& username, const String& password) {
  _server = server;
  _username = username;
  _password = password;
  _auth = !_username.isEmpty() && !_password.isEmpty();

  _server->on(url, HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (_auth && !request->authenticate(_username.c_str(), _password.c_str())) {
      return request->requestAuthentication();
    }
    // Send Webpage
    AsyncWebServerResponse* response = request->beginResponse_P(
      200,
      "text/html",
      WEBSERIAL_HTML,
      WEBSERIAL_HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  String backendUrl = url;
  backendUrl.concat("ws");
  _ws = new AsyncWebSocket(backendUrl);
  if (_auth) {
    _ws->setAuthentication(_username.c_str(), _password.c_str());
  }
  _ws->onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) -> void {
    if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len) {
        if (info->opcode == WS_TEXT) {
          data[len] = 0;
        }
        const String msg = reinterpret_cast<const char*>(data);
        if (msg == "ping")
          client->text("pong");
        else if (_recvMsgCallback != NULL)
          _recvMsgCallback(client, msg);
      }
    }
  });
  _server->addHandler(_ws);
}

void WebSerialClass::onMessage(RecvMsgHandler callbackFunc) {
  _recvMsgCallback = callbackFunc;
}

// Print
size_t WebSerialClass::write(uint8_t m) {
  if (_ws != NULL) {
    _ws->textAll((const char*)&(m), 1);
  }
  return (1);
}

size_t WebSerialClass::write(const uint8_t* buffer, size_t size) {
  if (_ws != NULL) {
    _ws->cleanupClients(WEBSERIAL_MAX_WS_CLIENTS);
    if (_ws->count() > 0)
      _ws->textAll((const char*)buffer, size);
  }
  return (size);
}

WebSerialClass WebSerial;
