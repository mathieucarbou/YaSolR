// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <AsyncJson.h>
#include <StreamString.h>
#include <YaSolRWebsite.h>

#include <map>

Mycila::Task initRestApiTask("Init REST API", [](void* params) {
  logger.info(TAG, "Initializing REST API");

  // debug

  webServer
    .on("/api/debug", HTTP_GET, [](AsyncWebServerRequest* request) {
      if (!config.getBool(KEY_ENABLE_DEBUG)) {
        return request->send(404, "application/json", "{\"error\":\"Debug mode is disabled.\"}");
      }

      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();
      float voltage = grid.getVoltage().value_or(0);

      Mycila::AppInfo.toJson(root["app"].to<JsonObject>());
      config.toJson(root["config"].to<JsonObject>());
      grid.toJson(root["grid"].to<JsonObject>());
      jsy.toJson(root["jsy"].to<JsonObject>());
      espConnect.toJson(root["network"].to<JsonObject>());

      // output 1
      output1.toJson(root["output1"].to<JsonObject>(), voltage);
      dimmerO1.dimmerToJson(root["output1"]["dimmer"].to<JsonObject>());
      ds18O1.toJson(root["output1"]["ds18"].to<JsonObject>());
      pzemO1.toJson(root["output1"]["pzem"].to<JsonObject>());
      bypassRelayO1.toJson(root["output1"]["relay"].to<JsonObject>());

      // output 2
      output2.toJson(root["output2"].to<JsonObject>(), voltage);
      dimmerO2.dimmerToJson(root["output2"]["dimmer"].to<JsonObject>());
      ds18O2.toJson(root["output2"]["ds18"].to<JsonObject>());
      pzemO2.toJson(root["output2"]["pzem"].to<JsonObject>());
      bypassRelayO2.toJson(root["output2"]["relay"].to<JsonObject>());

      pidController.toJson(root["pid"].to<JsonObject>());
      pulseAnalyzer.toJson(root["pulse_analyzer"].to<JsonObject>());

      // relays
      relay1.toJson(root["relay1"].to<JsonObject>());
      relay2.toJson(root["relay2"].to<JsonObject>());

      // router
      router.toJson(root["router"].to<JsonObject>(), voltage);

      // system
      JsonObject system = root["system"].to<JsonObject>();
      Mycila::System::toJson(system);
      ds18Sys.toJson(system["ds18"].to<JsonObject>());
      lights.toJson(system["leds"].to<JsonObject>());

      // stack
      Mycila::TaskMonitor.toJson(system["stack"].to<JsonObject>());

      // tasks
      JsonObject tasks = system["task"].to<JsonObject>();
      pioTaskManager.toJson(tasks[pioTaskManager.getName()].to<JsonObject>());
      coreTaskManager.toJson(tasks[coreTaskManager.getName()].to<JsonObject>());
      routingTaskManager.toJson(tasks[routingTaskManager.getName()].to<JsonObject>());
      jsyTaskManager.toJson(tasks[jsyTaskManager.getName()].to<JsonObject>());
      pzemTaskManager.toJson(tasks[pzemTaskManager.getName()].to<JsonObject>());
      mqttTaskManager.toJson(tasks[mqttTaskManager.getName()].to<JsonObject>());

      // libs versions
      JsonObject library = system["lib"].to<JsonObject>();
      library["ArduinoJson"] = ARDUINOJSON_VERSION;
      library["AsyncTCP"] = ASYNCTCP_VERSION;
      library["CRC"] = CRC_LIB_VERSION;
      library["ESPAsyncWebServer"] = ASYNCWEBSERVER_VERSION;
      library["MycilaConfig"] = MYCILA_CONFIG_VERSION;
      library["MycilaDS18"] = MYCILA_DS18_VERSION;
      library["MycilaEasyDisplay"] = MYCILA_EASY_DISPLAY_VERSION;
      library["MycilaESPConnect"] = ESPCONNECT_VERSION;
      library["MycilaHADiscovery"] = MYCILA_HA_VERSION;
      library["MycilaJSY"] = MYCILA_JSY_VERSION;
      library["MycilaLogger"] = MYCILA_LOGGER_VERSION;
      library["MycilaMQTT"] = MYCILA_MQTT_VERSION;
      library["MycilaNTP"] = MYCILA_NTP_VERSION;
      library["MycilaPulseAnalyzer"] = MYCILA_PULSE_VERSION;
      library["MycilaPZEM004Tv3"] = MYCILA_PZEM_VERSION;
      library["MycilaRelay"] = MYCILA_RELAY_VERSION;
      library["MycilaSystem"] = MYCILA_SYSTEM_VERSION;
      library["MycilaTaskManager"] = MYCILA_TASK_MANAGER_VERSION;
      library["MycilaTaskMonitor"] = MYCILA_TASK_MONITOR_VERSION;
      library["MycilaTrafficLight"] = MYCILA_TRAFFIC_LIGHT_VERSION;
      library["MycilaUtilities"] = MYCILA_UTILITIES_VERSION;

      response->setLength();
      request->send(response);
    });

  // config

  webServer
    .on("/api/config/backup", HTTP_GET, [](AsyncWebServerRequest* request) {
      String body;
      body.reserve(4096);
      config.backup(body);
      AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", body);
      response->addHeader("Content-Disposition", "attachment; filename=\"config.txt\"");
      request->send(response);
    });

  webServer
    .on(
      "/api/config/restore",
      HTTP_POST,
      [](AsyncWebServerRequest* request) {
        if (!request->_tempObject) {
          return request->send(400, "text/plain", "No config file uploaded");
        }
        StreamString* buffer = reinterpret_cast<StreamString*>(request->_tempObject);
        config.restore(*buffer);
        delete buffer;
        request->_tempObject = nullptr;
        request->send(200, "text/plain", "OK");
      },
      [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (!index) {
          if (request->_tempObject) {
            delete reinterpret_cast<StreamString*>(request->_tempObject);
          }
          StreamString* buffer = new StreamString();
          buffer->reserve(4096);
          request->_tempObject = buffer;
        }
        if (len) {
          reinterpret_cast<StreamString*>(request->_tempObject)->write(data, len);
        }
      });

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
        YaSolR::Website.initCards();
        dashboardTask.requestEarlyRun();
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
    });

  webServer
    .on("/api/config", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse(false);
      config.toJson(response->getRoot());
      response->setLength();
      request->send(response);
    });

  // system

  webServer
    .on("/api/system/restart", HTTP_ANY, [](AsyncWebServerRequest* request) {
      restartTask.resume();
      request->send(200);
    });

  webServer
    .on("/api/system/safeboot", HTTP_ANY, [](AsyncWebServerRequest* request) {
      safeBootTask.resume();
      request->send(200);
    });

  webServer
    .on("/api/system/reset", HTTP_ANY, [](AsyncWebServerRequest* request) {
      resetTask.resume();
      request->send(200);
    });

  webServer
    .on("/api/system", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      Mycila::System::Memory memory;
      Mycila::System::getMemory(memory);

      root["app"]["manufacturer"] = Mycila::AppInfo.manufacturer;
      root["app"]["model"] = Mycila::AppInfo.model;
      root["app"]["name"] = Mycila::AppInfo.name;
      root["app"]["version"] = Mycila::AppInfo.version;

      root["device"]["boots"] = Mycila::System::getBootCount();
      root["device"]["cores"] = ESP.getChipCores();
      root["device"]["cpu_freq"] = ESP.getCpuFreqMHz();
      root["device"]["heap"]["total"] = memory.total;
      root["device"]["heap"]["usage"] = memory.usage;
      root["device"]["heap"]["used"] = memory.used;
      root["device"]["id"] = Mycila::AppInfo.id;
      root["device"]["model"] = ESP.getChipModel();
      root["device"]["revision"] = ESP.getChipRevision();
      root["device"]["uptime"] = Mycila::System::getUptime();

      root["firmware"]["build"]["branch"] = Mycila::AppInfo.buildBranch;
      root["firmware"]["build"]["hash"] = Mycila::AppInfo.buildHash;
      root["firmware"]["build"]["timestamp"] = Mycila::AppInfo.buildDate;
      root["firmware"]["debug"] = Mycila::AppInfo.debug;
      root["firmware"]["filename"] = Mycila::AppInfo.firmware;

      root["network"]["eth"]["ip_address"] = espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString();
      root["network"]["eth"]["mac_address"] = espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH);

      root["network"]["hostname"] = espConnect.getHostname();
      root["network"]["ip_address"] = espConnect.getIPAddress().toString();
      root["network"]["mac_address"] = espConnect.getMACAddress();
      switch (espConnect.getMode()) {
        case Mycila::ESPConnect::Mode::ETH:
          root["network"]["mode"] = "eth";
          break;
        case Mycila::ESPConnect::Mode::STA:
          root["network"]["mode"] = "wifi";
          break;
        case Mycila::ESPConnect::Mode::AP:
          root["network"]["mode"] = "ap";
          break;
        default:
          root["network"]["mode"] = "";
          break;
      }

      root["network"]["ntp"] = YASOLR_STATE(Mycila::NTP.isSynced());

      root["network"]["wifi"]["bssid"] = espConnect.getWiFiBSSID();
      root["network"]["wifi"]["ip_address"] = espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString();
      root["network"]["wifi"]["mac_address"] = espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA);
      root["network"]["wifi"]["quality"] = espConnect.getWiFiSignalQuality();
      root["network"]["wifi"]["rssi"] = espConnect.getWiFiRSSI();
      root["network"]["wifi"]["ssid"] = espConnect.getWiFiSSID();

      response->setLength();
      request->send(response);
    });

  // grid

  webServer
    .on("/api/grid", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();
      Mycila::Grid::Metrics metrics;
      grid.getMeasurements(metrics);
      Mycila::Grid::toJson(root, metrics);
      response->setLength();
      request->send(response);
    });

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
    });

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
    });

  // router dimmers

  webServer
    .on("/api/router/output1/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (request->hasParam("duty_cycle", true))
        output1.setDimmerDutyCycle(request->getParam("duty_cycle", true)->value().toFloat() / 100);
      request->send(200);
    });

  webServer
    .on("/api/router/output2/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (request->hasParam("duty_cycle", true))
        output2.setDimmerDutyCycle(request->getParam("duty_cycle", true)->value().toFloat() / 100);
      request->send(200);
    });

  // router bypass

  webServer
    .on("/api/router/output1/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (output1.isBypassEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        if (state == YASOLR_ON)
          output1.setBypassOn();
        else if (state == YASOLR_OFF)
          output1.setBypassOff();
      }
      request->send(200);
    });

  webServer
    .on("/api/router/output2/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
      if (output2.isBypassEnabled() && request->hasParam("state", true)) {
        String state = request->getParam("state", true)->value();
        if (state == YASOLR_ON)
          output2.setBypassOn();
        else if (state == YASOLR_OFF)
          output2.setBypassOff();
      }
      request->send(200);
    });

  webServer
    .on("/api/router", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      Mycila::Grid::Metrics gridMetrics;
      grid.getMeasurements(gridMetrics);

      Mycila::Router::Metrics routerMeasurements;
      router.getMeasurements(routerMeasurements);

      root["lights"] = lights.toString();
      root["relay1"] = YASOLR_STATE(relay1.isOn());
      root["relay2"] = YASOLR_STATE(relay2.isOn());
      root["temperature"] = ds18Sys.getTemperature().value_or(0);
      root["virtual_grid_power"] = gridMetrics.power - routerMeasurements.power;

      Mycila::Router::toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

      for (const auto& output : router.getOutputs()) {
        JsonObject json = root[output->getName()].to<JsonObject>();
        json["state"] = output->getStateName();
        json["bypass"] = YASOLR_STATE(output->isBypassOn());
        json["dimmer"] = YASOLR_STATE(output->isDimmerOn());
        json["duty_cycle"] = output->getDimmerDutyCycle() * 100;
        json["temperature"] = output->temperature().orElse(0);

        Mycila::RouterOutput::Metrics outputMeasurements;
        output->getMeasurements(outputMeasurements);
        Mycila::RouterOutput::toJson(json["measurements"].to<JsonObject>(), outputMeasurements);
      }

      response->setLength();
      request->send(response);
    });

  webServer
    .on("/api", HTTP_GET, [](AsyncWebServerRequest* request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject root = response->getRoot();

      String base = "http://";
      base.concat(espConnect.getIPAddress().toString());
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
    });
});
