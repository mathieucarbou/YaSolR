// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#include <AsyncJson.h>
#include <LittleFS.h>
#include <map>

#define TAG "YASOLR"

void YaSolR::YaSolRClass::_initREST() {
  Mycila::Logger.info(TAG, "Initializing REST API...");

  // app

  Mycila::HTTPd.apiGET("/app", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    Mycila::AppInfo.toJson(response->getRoot());
    response->setLength();
    request->send(response);
  });

  // config

  Mycila::HTTPd.apiGET("/config/backup", [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", Mycila::Config.backup());
    response->addHeader("Content-Disposition", "attachment; filename=\"config.txt\"");
    request->send(response);
  });

  Mycila::HTTPd.apiUPLOAD(
    "/config/restore",
    [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "OK");
      if (!LittleFS.exists("/config.txt")) {
        return request->send(400, "text/plain", "No config.txt file uploaded");
      }
      File cfg = LittleFS.open("/config.txt", "r");
      const String data = cfg.readString();
      cfg.close();
      Mycila::Config.restore(data);
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    },
    [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
      if (!index)
        request->_tempFile = LittleFS.open("/config.txt", "w");
      if (len)
        request->_tempFile.write(data, len);
      if (final)
        request->_tempFile.close();
    });

  Mycila::HTTPd.apiPOST("/config", [](AsyncWebServerRequest* request) {
    std::map<const char*, String> settings;
    for (size_t i = 0, max = request->params(); i < max; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isPost() && !p->isFile()) {
        const char* keyRef = Mycila::Config.keyRef(p->name().c_str());
        settings[keyRef] = p->value();
      }
    }
    request->send(200);
    Mycila::Config.set(settings);
  });

  Mycila::HTTPd.apiGET("/config", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse(false);
    Mycila::Config.toJson(response->getRoot());
    response->setLength();
    request->send(response);
  });

  // grid

  Mycila::HTTPd.apiGET("/grid", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    Mycila::Grid.toJson(root);
    response->setLength();
    request->send(response);
  });

  // network

  Mycila::HTTPd.apiGET("/network", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    ESPConnect.toJson(response->getRoot());
    response->setLength();
    request->send(response);
  });

  // ntp

  Mycila::HTTPd.apiGET("/ntp/timezones", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse(true);
    Mycila::NTP.timezonesToJsonArray(response->getRoot().as<JsonArray>());
    response->setLength();
    request->send(response);
  });

  Mycila::HTTPd.apiGET("/ntp", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    response->getRoot()["synced"] = Mycila::NTP.isSynced();
    response->setLength();
    request->send(response);
  });

  // relays

  Mycila::HTTPd.apiPOST("/relays/" NAME_RELAY1, [this](AsyncWebServerRequest* request) {
    if (Mycila::RelayManager.relays[0].isEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      uint32_t duration = request->hasParam("duration", true) ? request->getParam("duration", true)->value().toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(0, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(0, false, duration);
    }
    request->send(200);
  });
  Mycila::HTTPd.apiPOST("/relays/" NAME_RELAY2, [this](AsyncWebServerRequest* request) {
    if (Mycila::RelayManager.relays[1].isEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      uint32_t duration = request->hasParam("duration", true) ? request->getParam("duration", true)->value().toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(1, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(1, false, duration);
    }
    request->send(200);
  });

  Mycila::HTTPd.apiGET("/relays", [this](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    Mycila::RelayManager.toJson(root);
    response->setLength();
    request->send(response);
  });

  // router

  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT1 "/dimmer", [](AsyncWebServerRequest* request) {
    if (request->hasParam("level", true))
      Mycila::Router.outputs[0].tryDimmerLevel(request->getParam("level", true)->value().toInt());
    request->send(200);
  });
  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT2 "/dimmer", [](AsyncWebServerRequest* request) {
    if (request->hasParam("level", true))
      Mycila::Router.outputs[1].tryDimmerLevel(request->getParam("level", true)->value().toInt());
    request->send(200);
  });

  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT1 "/bypass_relay", [](AsyncWebServerRequest* request) {
    if (Mycila::Router.outputs[0].isBypassRelayEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      if (state == YASOLR_ON)
        Mycila::Router.outputs[0].tryBypassRelayState(true);
      else if (state == YASOLR_OFF)
        Mycila::Router.outputs[0].tryBypassRelayState(false);
    }
    request->send(200);
  });
  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT2 "/bypass_relay", [](AsyncWebServerRequest* request) {
    if (Mycila::Router.outputs[1].isBypassRelayEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      if (state == YASOLR_ON)
        Mycila::Router.outputs[1].tryBypassRelayState(true);
      else if (state == YASOLR_OFF)
        Mycila::Router.outputs[1].tryBypassRelayState(false);
    }
    request->send(200);
  });

  Mycila::HTTPd.apiGET("/router", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    Mycila::Router.toJson(root);
    response->setLength();
    request->send(response);
  });

  // system

  Mycila::HTTPd.apiANY("/system/restart", [=](AsyncWebServerRequest* request) {
    restartTask.resume();
    request->send(200);
  });

  Mycila::HTTPd.apiANY("/system/reset", [=](AsyncWebServerRequest* request) {
    resetTask.resume();
    request->send(200);
  });

  Mycila::HTTPd.apiGET("/system", [this](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    Mycila::System.toJson(root);
    JsonObject buzzer = root["buzzer"].to<JsonObject>();
    Mycila::Buzzer.toJson(buzzer);
    JsonObject lights = root["lights"].to<JsonObject>();
    Mycila::Lights.toJson(lights);
    JsonObject tasks = root["tasks"].to<JsonObject>();
    Mycila::TaskMonitor.toJson(tasks);
    JsonObject temp_sensor = root["temp_sensor"].to<JsonObject>();
    systemTemperatureSensor.toJson(temp_sensor);
    response->setLength();
    request->send(response);
  });
}
