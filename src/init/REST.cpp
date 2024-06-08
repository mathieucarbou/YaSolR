// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#include <AsyncJson.h>
#include <map>

#define TAG "YASOLR"

Mycila::Task initRestApiTask("Init REST API", [](void* params) {
  logger.info(TAG, "Initializing REST API...");

  // debug

  webServer
    .on("/api/debug", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      Mycila::Grid.toJson(root["grid"].to<JsonObject>());

      Mycila::ZCD.toJson(root["zcd"].to<JsonObject>());

      jsy.toJson(root["jsy"].to<JsonObject>());

      pzemO1.toJson(root["pzem"]["output1"].to<JsonObject>());
      pzemO2.toJson(root["pzem"]["output2"].to<JsonObject>());

      dimmerO1.toJson(root["dimmer"]["output1"].to<JsonObject>());
      dimmerO2.toJson(root["dimmer"]["output2"].to<JsonObject>());

      relay1.toJson(root["relay"]["relay1"].to<JsonObject>());
      relay2.toJson(root["relay"]["relay2"].to<JsonObject>());
      bypassRelayO1.toJson(root["relay"]["output1"].to<JsonObject>());
      bypassRelayO2.toJson(root["relay"]["output2"].to<JsonObject>());

      ds18Sys.toJson(root["ds18"]["router"].to<JsonObject>());
      ds18O1.toJson(root["ds18"]["output1"].to<JsonObject>());
      ds18O2.toJson(root["ds18"]["output2"].to<JsonObject>());

      Mycila::TaskMonitor.toJson(root["stack"].to<JsonObject>());

      ioTaskManager.toJson(root["ioTaskManager"].to<JsonObject>());
      jsyTaskManager.toJson(root["jsyTaskManager"].to<JsonObject>());
      coreTaskManager.toJson(root["coreTaskManager"].to<JsonObject>());
      pzemTaskManager.toJson(root["pzemTaskManager"].to<JsonObject>());
      routerTaskManager.toJson(root["routerTaskManager"].to<JsonObject>());

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
        AsyncWebParameter* p = request->getParam(i);
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

      Mycila::SystemMemory memory = Mycila::System.getMemory();

      root["app"]["manufacturer"] = Mycila::AppInfo.manufacturer;
      root["app"]["model"] = Mycila::AppInfo.model;
      root["app"]["name"] = Mycila::AppInfo.name;
      root["app"]["trial"] = Mycila::AppInfo.trial;
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

      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // grid

  webServer
    .on("/api/grid", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      Mycila::Grid.toJson(response->getRoot());
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
          Mycila::RelayManager.tryRelayState("relay1", true, duration);
        else if (state == YASOLR_OFF)
          Mycila::RelayManager.tryRelayState("relay1", false, duration);
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
          Mycila::RelayManager.tryRelayState("relay2", true, duration);
        else if (state == YASOLR_OFF)
          Mycila::RelayManager.tryRelayState("relay2", false, duration);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // router dimmers

  webServer
    .on("/api/router/output1/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (request->hasParam("level", true))
        output1.tryDimmerLevel(request->getParam("level", true)->value().toInt());
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router/output2/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (request->hasParam("level", true))
        output2.tryDimmerLevel(request->getParam("level", true)->value().toInt());
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  // router bypass

  webServer
    .on("/api/router/output1/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (output1.isBypassRelayEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        if (state == YASOLR_ON)
          output1.tryBypassRelayState(true);
        else if (state == YASOLR_OFF)
          output1.tryBypassRelayState(false);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router/output2/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (output2.isBypassRelayEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        if (state == YASOLR_ON)
          output2.tryBypassRelayState(true);
        else if (state == YASOLR_OFF)
          output2.tryBypassRelayState(false);
      }
      request->send(200);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api/router", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      root["lights"] = lights.toString();
      root["temperature"] = ds18Sys.getLastTemperature();
      root["energy"] = Mycila::Router.getTotalRoutedEnergy();
      root["power"] = Mycila::Router.getTotalRoutedPower();
      root["power_factor"] = Mycila::Router.getTotalPowerFactor();
      root["thdi"] = Mycila::Router.getTotalTHDi();
      root["virtual_grid_power"] = Mycila::Router.getVirtualGridPower();

      root["relay1"]["state"] = YASOLR_STATE(relay1.isOn());
      root["relay1"]["switch_count"] = relay1.getSwitchCount();

      root["relay2"]["state"] = YASOLR_STATE(relay2.isOn());
      root["relay2"]["switch_count"] = relay2.getSwitchCount();

      root["output1"]["bypass"] = YASOLR_STATE(output1.isBypassOn());
      root["output1"]["dimmer"]["level"] = dimmerO1.getLevel();
      root["output1"]["dimmer"]["state"] = YASOLR_STATE(dimmerO1.isOn());
      root["output1"]["relay"]["state"] = YASOLR_STATE(bypassRelayO1.isOn());
      root["output1"]["relay"]["switch_count"] = bypassRelayO1.getSwitchCount();
      root["output1"]["temperature"] = ds18O1.getLastTemperature();

      root["output2"]["bypass"] = YASOLR_STATE(output2.isBypassOn());
      root["output2"]["dimmer"]["level"] = dimmerO2.getLevel();
      root["output2"]["dimmer"]["state"] = YASOLR_STATE(dimmerO2.isOn());
      root["output2"]["relay"]["state"] = YASOLR_STATE(bypassRelayO2.isOn());
      root["output2"]["relay"]["switch_count"] = bypassRelayO2.getSwitchCount();
      root["output2"]["temperature"] = ds18O2.getLastTemperature();

      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));

  webServer
    .on("/api", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      String base = "http://";
      base += ESPConnect.getIPAddress().toString();
      base += "/api";

      root["config"] = base + "/config";
      root["config/backup"] = base + "/config/backup";
      root["debug"] = base + "/debug";
      root["grid"] = base + "/grid";
      root["router"] = base + "/router";
      root["system"] = base + "/system";
      root["system/restart"] = base + "/system/restart";

      response->setLength();
      request->send(response);
    })
    .setAuthentication(YASOLR_ADMIN_USERNAME, config.get(KEY_ADMIN_PASSWORD));
});
