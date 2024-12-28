// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

extern const uint8_t logo_png_gz_start[] asm("_binary__pio_data_logo_png_gz_start");
extern const uint8_t logo_png_gz_end[] asm("_binary__pio_data_logo_png_gz_end");
extern const uint8_t logo_icon_png_gz_start[] asm("_binary__pio_data_logo_icon_png_gz_start");
extern const uint8_t logo_icon_png_gz_end[] asm("_binary__pio_data_logo_icon_png_gz_end");
extern const uint8_t config_html_gz_start[] asm("_binary__pio_data_config_html_gz_start");
extern const uint8_t config_html_gz_end[] asm("_binary__pio_data_config_html_gz_end");

void yasolr_http() {
  logger.info(TAG, "Initializing HTTP Endpoints");

  loggingMiddleware.setOutput(Serial);

  authMiddleware.setAuthType(AsyncAuthType::AUTH_DIGEST);
  authMiddleware.setRealm("YaSolR");
  authMiddleware.setUsername(YASOLR_ADMIN_USERNAME);
  authMiddleware.setPassword(config.get(KEY_ADMIN_PASSWORD));
  authMiddleware.generateHash();

  webServer.addMiddleware(&loggingMiddleware);
  webServer.addMiddleware(&authMiddleware);

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

  webServer
    .on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(200, "text/html", config_html_gz_start, config_html_gz_end - config_html_gz_start);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    });

  webServer
    .on("/timezones", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse(true);
      Mycila::NTP.timezonesToJsonArray(response->getRoot().as<JsonArray>());
      response->setLength();
      request->send(response);
    });
};
