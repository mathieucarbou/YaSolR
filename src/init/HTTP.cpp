// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <AsyncJson.h>

extern const uint8_t logo_png_gz_start[] asm("_binary__pio_data_logo_png_gz_start");
extern const uint8_t logo_png_gz_end[] asm("_binary__pio_data_logo_png_gz_end");
extern const uint8_t logo_icon_png_gz_start[] asm("_binary__pio_data_logo_icon_png_gz_start");
extern const uint8_t logo_icon_png_gz_end[] asm("_binary__pio_data_logo_icon_png_gz_end");
extern const uint8_t config_html_gz_start[] asm("_binary__pio_data_config_html_gz_start");
extern const uint8_t config_html_gz_end[] asm("_binary__pio_data_config_html_gz_end");

Mycila::Task initWebTask("Init Web", [](void* params) {
  logger.info(TAG, "Initializing HTTP Endpoints");

  webServer.rewrite("/dash/assets/logo/mini", "/logo-icon");
  webServer.rewrite("/dash/assets/logo/large", "/logo");
  webServer.rewrite("/dash/assets/logo", "/logo");
  webServer.rewrite("/ota/logo/dark", "/logo");
  webServer.rewrite("/ota/logo/light", "/logo");
  webServer.rewrite("/wsl/logo/dark", "/logo");
  webServer.rewrite("/wsl/logo/light", "/logo");
  webServer.rewrite("/", "/dashboard").setFilter([](AsyncWebServerRequest* request) { return espConnect.getState() != Mycila::ESPConnect::State::PORTAL_STARTED; });

  webServer.on("/logo-icon", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "image/png", logo_icon_png_gz_start, logo_icon_png_gz_end - logo_icon_png_gz_start);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "public, max-age=900");
    request->send(response);
  });

  webServer.on("/logo", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "image/png", logo_png_gz_start, logo_png_gz_end - logo_png_gz_start);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "public, max-age=900");
    request->send(response);
  });

  webServer.on("/ping", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "pong");
    request->send(response);
  });

  webServer
    .on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(200, "text/html", config_html_gz_start, config_html_gz_end - config_html_gz_start);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/timezones", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse(true);
      Mycila::NTP.timezonesToJsonArray(response->getRoot().as<JsonArray>());
      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  wsDebugPID.setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));
  wsDebugPID.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      logger.info(TAG, "Websocket client connected to: /ws/pid/csv");
      wsDebugPID.cleanupClients();
      client->text("pMode,dMode,icMode,rev,setpoint,kp,ki,kd,outMin,outMax,input,output,error,sum,pTerm,iTerm,dTerm");
    }
  });
  webServer.addHandler(&wsDebugPID);
});
