// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#include <AsyncJson.h>
#include <map>

static void systemInfoToJson(JsonObject& root) {
  Mycila::SystemMemory memory = Mycila::System.getMemory();

  root["app"]["manufacturer"] = Mycila::AppInfo.manufacturer;
  root["app"]["model"] = Mycila::AppInfo.model;
  root["app"]["name"] = Mycila::AppInfo.name;
  root["app"]["version"] = Mycila::AppInfo.version;

  root["device"]["boots"] = Mycila::System.getBootCount();
  root["device"]["cores"] = ESP.getChipCores();
  root["device"]["cpu_freq"] = ESP.getCpuFreqMHz();
  root["device"]["heap"]["total"] = memory.total;
  root["device"]["heap"]["usage"] = memory.usage;
  root["device"]["heap"]["used"] = memory.used;
  root["device"]["id"] = Mycila::AppInfo.id;
  root["device"]["model"] = ESP.getChipModel();
  root["device"]["revision"] = ESP.getChipRevision();
  root["device"]["uptime"] = Mycila::System.getUptime();

  root["firmware"]["build"]["branch"] = Mycila::AppInfo.buildBranch;
  root["firmware"]["build"]["hash"] = Mycila::AppInfo.buildHash;
  root["firmware"]["build"]["timestamp"] = Mycila::AppInfo.buildDate;
  root["firmware"]["debug"] = Mycila::AppInfo.debug;
  root["firmware"]["filename"] = Mycila::AppInfo.firmware;

  root["network"]["eth"]["ip_address"] = ESPConnect.getIPAddress(ESPConnectMode::ETH).toString();
  root["network"]["eth"]["mac_address"] = ESPConnect.getMACAddress(ESPConnectMode::ETH);

  root["network"]["hostname"] = ESPConnect.getHostname();
  root["network"]["ip_address"] = ESPConnect.getIPAddress().toString();
  root["network"]["mac_address"] = ESPConnect.getMACAddress();
  switch (ESPConnect.getMode()) {
    case ESPConnectMode::ETH:
      root["network"]["mode"] = "eth";
      break;
    case ESPConnectMode::STA:
      root["network"]["mode"] = "wifi";
      break;
    case ESPConnectMode::AP:
      root["network"]["mode"] = "ap";
      break;
    default:
      root["network"]["mode"] = "";
      break;
  }

  root["network"]["ntp"] = YASOLR_STATE(Mycila::NTP.isSynced());

  root["network"]["wifi"]["bssid"] = ESPConnect.getWiFiBSSID();
  root["network"]["wifi"]["ip_address"] = ESPConnect.getIPAddress(ESPConnectMode::STA).toString();
  root["network"]["wifi"]["mac_address"] = ESPConnect.getMACAddress(ESPConnectMode::STA);
  root["network"]["wifi"]["quality"] = ESPConnect.getWiFiSignalQuality();
  root["network"]["wifi"]["rssi"] = ESPConnect.getWiFiRSSI();
  root["network"]["wifi"]["ssid"] = ESPConnect.getWiFiSSID();
}

Mycila::Task initRestApiTask("Init REST API", [](void* params) {
  logger.info(TAG, "Initializing REST API...");

  // debug

  webServer
    .on("/api/debug", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!config.getBool(KEY_ENABLE_DEBUG)) {
        return request->send(404, "application/json", "{\"error\":\"Debug mode is disabled.\"}");
      }

      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      JsonObject routerJson = root["router"].to<JsonObject>();
      grid.toJson(routerJson["grid"].to<JsonObject>());
      jsy.toJson(routerJson["jsy"].to<JsonObject>());
      relay1.toJson(routerJson["relay1"].to<JsonObject>());
      relay2.toJson(routerJson["relay2"].to<JsonObject>());
      router.toJson(routerJson["router"].to<JsonObject>(), grid.getVoltage().value_or(0));
      ds18O1.toJson(routerJson["router"]["output1"]["ds18"].to<JsonObject>());
      ds18O2.toJson(routerJson["router"]["output2"]["ds18"].to<JsonObject>());
      zcd.toJson(routerJson["zcd"].to<JsonObject>());

      JsonObject systemJson = root["system"].to<JsonObject>();
      systemInfoToJson(systemJson);
      ds18Sys.toJson(systemJson["ds18Sys"].to<JsonObject>());
      lights.toJson(systemJson["leds"].to<JsonObject>());
      Mycila::TaskMonitor.toJson(systemJson["stack"].to<JsonObject>());
      pioTaskManager.toJson(systemJson[pioTaskManager.getName()].to<JsonObject>());
      coreTaskManager.toJson(systemJson[coreTaskManager.getName()].to<JsonObject>());
      routerTaskManager.toJson(systemJson[routerTaskManager.getName()].to<JsonObject>());
      jsyTaskManager.toJson(systemJson[jsyTaskManager.getName()].to<JsonObject>());
      pzemTaskManager.toJson(systemJson[pzemTaskManager.getName()].to<JsonObject>());
      mqttTaskManager.toJson(systemJson[mqttTaskManager.getName()].to<JsonObject>());

      config.toJson(root["config"].to<JsonObject>());

      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // config

  webServer
    .on("/api/config/backup", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", config.backup());
      response->addHeader("Content-Disposition", "attachment; filename=\"config.txt\"");
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on(
      "/api/config/restore",
      HTTP_POST,
      [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "OK");
        if (!LittleFS.exists("/config.txt")) {
          return request->send(400, "text/plain", "No config.txt file uploaded");
        }
        File cfg = LittleFS.open("/config.txt", "r");
        const String data = cfg.readString();
        cfg.close();
        config.restore(data);
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
      })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on(
      "/api/config/mqttServerCertificate",
      HTTP_POST,
      [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "OK");
        if (!LittleFS.exists("/mqtt-server.crt")) {
          return request->send(400, "text/plain", "No server certificate file uploaded");
        }
        File serverCertFile = LittleFS.open("/mqtt-server.crt", "r");
        logger.info(TAG, "Uploaded MQTT server certificate:\n%s", serverCertFile.readString().c_str());
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
      })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/config", HTTP_POST, [](AsyncWebServerRequest* request) {
      std::map<const char*, String> settings;
      for (size_t i = 0, max = request->params(); i < max; i++) {
        const AsyncWebParameter* p = request->getParam(i);
        if (p->isPost() && !p->isFile()) {
          const char* keyRef = config.keyRef(p->name().c_str());
          settings[keyRef] = p->value();
        }
      }
      request->send(200);
      config.set(settings);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/config", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse(false);
      config.toJson(response->getRoot());
      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // system

  webServer
    .on("/api/system/restart", HTTP_ANY, [](AsyncWebServerRequest* request) {
      restartTask.resume();
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/system/reset", HTTP_ANY, [](AsyncWebServerRequest* request) {
      resetTask.resume();
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/system", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();
      systemInfoToJson(root);
      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // grid

  webServer
    .on("/api/grid", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();
      Mycila::GridMetrics metrics;
      grid.getMeasurements(metrics);
      Mycila::Grid::toJson(root, metrics);
      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // router relays

  webServer
    .on("/api/router/relay1", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (relay1.isEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        uint32_t duration = request->hasParam("duration", true) ? request->getParam("duration", true)->value().toInt() : 0;
        if (state == YASOLR_ON)
          routerRelay1.tryRelayState(true, duration);
        else if (state == YASOLR_OFF)
          routerRelay1.tryRelayState(false, duration);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router/relay2", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (relay2.isEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        uint32_t duration = request->hasParam("duration", true) ? request->getParam("duration", true)->value().toInt() : 0;
        if (state == YASOLR_ON)
          routerRelay2.tryRelayState(true, duration);
        else if (state == YASOLR_OFF)
          routerRelay2.tryRelayState(false, duration);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // router dimmers

  webServer
    .on("/api/router/output1/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (request->hasParam("duty", true))
        output1.tryDimmerDuty(request->getParam("level", true)->value().toInt());
      else if (request->hasParam("duty_cycle", true))
        output1.tryDimmerDutyCycle(request->getParam("duty_cycle", true)->value().toFloat());
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router/output2/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (request->hasParam("duty", true))
        output2.tryDimmerDuty(request->getParam("level", true)->value().toInt());
      else if (request->hasParam("duty_cycle", true))
        output2.tryDimmerDutyCycle(request->getParam("duty_cycle", true)->value().toFloat());
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // router bypass

  webServer
    .on("/api/router/output1/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (output1.isBypassEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        if (state == YASOLR_ON)
          output1.tryBypassState(true);
        else if (state == YASOLR_OFF)
          output1.tryBypassState(false);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router/output2/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (output2.isBypassEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        if (state == YASOLR_ON)
          output2.tryBypassState(true);
        else if (state == YASOLR_OFF)
          output2.tryBypassState(false);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      Mycila::GridMetrics gridMetrics;
      grid.getMeasurements(gridMetrics);

      Mycila::RouterMetrics routerMeasurements;
      router.getMeasurements(routerMeasurements);

      root["lights"] = lights.toString();
      root["temperature"] = ds18Sys.getTemperature().value_or(0);
      root["virtual_grid_power"] = gridMetrics.power - routerMeasurements.power;

      Mycila::Router::toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

      size_t idx = 0;
      for (const auto& output : router.getOutputs()) {
        JsonObject json = root[output->getName()].to<JsonObject>();
        json["bypass"] = YASOLR_STATE(output->isBypassOn());
        json["state"] = output->getStateName();
        json["temperature"] = output->temperature().orElse(0);

        json["dimmer"]["duty"] = dimmerO1.getDuty();
        json["dimmer"]["duty_cycle"] = dimmerO1.getDutyCycle();
        json["dimmer"]["state"] = YASOLR_STATE(dimmerO1.isOn());

        Mycila::RouterOutputMetrics outputMeasurements;
        output->getMeasurements(outputMeasurements);
        Mycila::RouterOutput::toJson(json["measurements"].to<JsonObject>(), outputMeasurements);

        json["relay"]["state"] = YASOLR_STATE(bypassRelayO1.isOn());
        json["relay"]["switch_count"] = bypassRelayO1.getSwitchCount();

        idx++;
      }

      root["relay1"]["state"] = YASOLR_STATE(relay1.isOn());
      root["relay1"]["switch_count"] = relay1.getSwitchCount();

      root["relay2"]["state"] = YASOLR_STATE(relay2.isOn());
      root["relay2"]["switch_count"] = relay2.getSwitchCount();

      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      String base = "http://";
      base.concat(ESPConnect.getIPAddress().toString());
      base.concat("/api");

      root["config"] = base + "/config";
      root["config/backup"] = base + "/config/backup";
      if (config.getBool(KEY_ENABLE_DEBUG)) {
        root["debug"] = base + "/debug";
      }
      root["grid"] = base + "/grid";
      root["router"] = base + "/router";
      root["system"] = base + "/system";

      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));
});
