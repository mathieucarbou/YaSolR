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

  Mycila::HTTPd.apiUPLOAD(
    "/config/mqttServerCertificate",
    [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "OK");
      if (!LittleFS.exists("/mqtt-server.crt")) {
        return request->send(400, "text/plain", "No server certificate file uploaded");
      }
      File serverCertFile = LittleFS.open("/mqtt-server.crt", "r");
      Mycila::Logger.info(TAG, "Uploaded MQTT server certificate:\n%s", serverCertFile.readString().c_str());
      serverCertFile.close();
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    },
    [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
      if (!index)
        request->_tempFile = LittleFS.open("/mqtt-server.crt", "w");
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
    if (relay1.isEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      uint32_t duration = request->hasParam("duration", true) ? request->getParam("duration", true)->value().toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(&relay1, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(&relay1, false, duration);
    }
    request->send(200);
  });
  Mycila::HTTPd.apiPOST("/relays/" NAME_RELAY2, [this](AsyncWebServerRequest* request) {
    if (relay2.isEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      uint32_t duration = request->hasParam("duration", true) ? request->getParam("duration", true)->value().toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(&relay2, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(&relay2, false, duration);
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
      output1.tryDimmerLevel(request->getParam("level", true)->value().toInt());
    request->send(200);
  });
  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT2 "/dimmer", [](AsyncWebServerRequest* request) {
    if (request->hasParam("level", true))
      output2.tryDimmerLevel(request->getParam("level", true)->value().toInt());
    request->send(200);
  });

  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT1 "/bypass_relay", [](AsyncWebServerRequest* request) {
    if (output1.isBypassRelayEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      if (state == YASOLR_ON)
        output1.tryBypassRelayState(true);
      else if (state == YASOLR_OFF)
        output1.tryBypassRelayState(false);
    }
    request->send(200);
  });
  Mycila::HTTPd.apiPOST("/router/" NAME_OUTPUT2 "/bypass_relay", [](AsyncWebServerRequest* request) {
    if (output2.isBypassRelayEnabled() && request->hasParam("state", true)) {
      String state = request->getParam("state", true)->value();
      if (state == YASOLR_ON)
        output2.tryBypassRelayState(true);
      else if (state == YASOLR_OFF)
        output2.tryBypassRelayState(false);
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
    Mycila::Buzzer.toJson(root["buzzer"].to<JsonObject>());
    Mycila::Lights.toJson(root["lights"].to<JsonObject>());
    systemTemperatureSensor.toJson(root["temp_sensor"].to<JsonObject>());
    response->setLength();
    request->send(response);
  });

  Mycila::HTTPd.apiGET("/debug", [this](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    Mycila::TaskMonitor.toJson(root["stack"].to<JsonObject>());
    // ioTaskManager.toJson(root["taskManagers"][0].to<JsonObject>());
    loopTaskManager.toJson(root["taskManagers"][1].to<JsonObject>());
    response->setLength();
    request->send(response);
  });
}
