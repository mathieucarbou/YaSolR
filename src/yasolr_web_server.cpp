// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>
#include <yasolr_dashboard.h>

#include <esp_partition.h>

#include <map>
#include <string>

extern const uint8_t logo_png_gz_start[] asm("_binary__pio_embed_logo_png_gz_start");
extern const uint8_t logo_png_gz_end[] asm("_binary__pio_embed_logo_png_gz_end");
extern const uint8_t logo_icon_png_gz_start[] asm("_binary__pio_embed_logo_icon_png_gz_start");
extern const uint8_t logo_icon_png_gz_end[] asm("_binary__pio_embed_logo_icon_png_gz_end");
extern const uint8_t config_html_gz_start[] asm("_binary__pio_embed_config_html_gz_start");
extern const uint8_t config_html_gz_end[] asm("_binary__pio_embed_config_html_gz_end");

AsyncWebServer webServer(80);
ESPDash dashboard(webServer, "/dashboard", false);
Mycila::ESPConnect espConnect(webServer);

static AsyncAuthenticationMiddleware authMiddleware;
static AsyncLoggingMiddleware loggingMiddleware;
YaSolR::Website website;

Mycila::Task dashboardInitTask("Init Dashboard", Mycila::Task::Type::ONCE, [](void* params) {
  website.initCards();
  website.updateCards();
  dashboard.sendUpdates();
});

Mycila::Task dashboardUpdateTask("Dashboard", [](void* params) {
  if (website.realTimePIDEnabled())
    website.updatePIDCharts();
  website.updateCards();
  website.updateCharts();
  dashboard.sendUpdates();
});

void rewrites() {
  webServer.rewrite("/dash/assets/logo/mini", "/logo-icon");
  webServer.rewrite("/dash/assets/logo/large", "/logo");
  webServer.rewrite("/dash/assets/logo", "/logo");
  webServer.rewrite("/dash/logo/light", "/logo");
  webServer.rewrite("/dash/logo/dark", "/logo");
  webServer.rewrite("/ota/logo/dark", "/logo");
  webServer.rewrite("/ota/logo/light", "/logo");
  webServer.rewrite("/wsl/logo/dark", "/logo");
  webServer.rewrite("/wsl/logo/light", "/logo");
  webServer.rewrite("/", "/dashboard").setFilter([](AsyncWebServerRequest* request) { return espConnect.getState() != Mycila::ESPConnect::State::PORTAL_STARTED; });
}

void routes() {
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

  webServer.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", config_html_gz_start, config_html_gz_end - config_html_gz_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webServer.on("/timezones", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse(true);
    Mycila::NTP.timezonesToJsonArray(response->getRoot().as<JsonArray>());
    response->setLength();
    request->send(response);
  });
}

void rest_api() {
  LOGI(TAG, "Initialize REST API");

  // debug

  webServer.on("/api/debug", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    float voltage = grid.getVoltage().value_or(0);

    Mycila::AppInfo.toJson(root["app"].to<JsonObject>());
    config.toJson(root["config"].to<JsonObject>());
    grid.toJson(root["grid"].to<JsonObject>());
    if (jsy)
      jsy->toJson(root["jsy"].to<JsonObject>());
    espConnect.toJson(root["network"].to<JsonObject>());

    pidController.toJson(root["pid"].to<JsonObject>());
    if (pulseAnalyzer)
      pulseAnalyzer->toJson(root["pulse_analyzer"].to<JsonObject>());

    // relays
    if (relay1)
      relay1->toJson(root["relay1"].to<JsonObject>());
    if (relay2)
      relay2->toJson(root["relay2"].to<JsonObject>());

    // router
    router.toJson(root["router"].to<JsonObject>(), voltage);

    // output 1
    if (output1)
      output1->toJson(root["router"]["output1"].to<JsonObject>(), voltage);
    if (ds18O1)
      ds18O1->toJson(root["router"]["output1"]["ds18"].to<JsonObject>());
    if (pzemO1)
      pzemO1->toJson(root["router"]["output1"]["pzem"].to<JsonObject>());

    // output 2
    if (output2)
      output2->toJson(root["router"]["output2"].to<JsonObject>(), voltage);
    if (ds18O2)
      ds18O2->toJson(root["router"]["output2"]["ds18"].to<JsonObject>());
    if (pzemO2)
      pzemO2->toJson(root["router"]["output2"]["pzem"].to<JsonObject>());

    // system
    JsonObject system = root["system"].to<JsonObject>();
    Mycila::System::toJson(system);
    if (ds18Sys)
      ds18Sys->toJson(system["ds18"].to<JsonObject>());
    lights.toJson(system["leds"].to<JsonObject>());

    // stack
    Mycila::TaskMonitor.toJson(system["stack"].to<JsonObject>());

    // tasks
    JsonObject tasks = system["task"].to<JsonObject>();
    coreTaskManager.toJson(tasks[coreTaskManager.name()].to<JsonObject>());
    if (jsyTaskManager)
      jsyTaskManager->toJson(tasks[jsyTaskManager->name()].to<JsonObject>());
    if (pzemTaskManager)
      pzemTaskManager->toJson(tasks[pzemTaskManager->name()].to<JsonObject>());
    unsafeTaskManager.toJson(tasks[unsafeTaskManager.name()].to<JsonObject>());

    if (victron)
      victron->toJson(root["victron"].to<JsonObject>());

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

  webServer.on("/api/config/backup", HTTP_GET, [](AsyncWebServerRequest* request) {
    StreamString body;
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
          request->send(400, "text/plain", "No config file uploaded");
        } else {
          StreamString* buffer = reinterpret_cast<StreamString*>(request->_tempObject);
          config.restore((*buffer).c_str());
          delete buffer;
          request->_tempObject = nullptr;
          request->send(200, "text/plain", "OK");
        }
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
      "/api/safeboot/upload",
      HTTP_POST,
      [](AsyncWebServerRequest* request) {
        if (request->_tempObject) {
          request->_tempObject = nullptr;
          website.setSafeBootUpdateStatus("Update complete!", dash::Status::SUCCESS);
          request->send(200);
        } else {
          request->send(400);
        }
      },
      [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (!index) {
          const esp_partition_t* partition = esp_partition_find_first(esp_partition_type_t::ESP_PARTITION_TYPE_APP, esp_partition_subtype_t::ESP_PARTITION_SUBTYPE_APP_FACTORY, YASOLR_SAFEBOOT_PARTITION_NAME);
          if (partition) {
            LOGW(TAG, "Updating Safeboot recovery partition");
            website.setSafeBootUpdateStatus("Updating...", dash::Status::WARNING);
            if (esp_partition_erase_range(partition, 0, partition->size) == ESP_OK) {
              LOGI(TAG, "Safeboot partition formatted");
              request->_tempObject = const_cast<void*>(reinterpret_cast<const void*>(partition));
            } else {
              website.setSafeBootUpdateStatus("Failed to format partition. Update aborted.", dash::Status::DANGER);
            }
          } else {
            website.setSafeBootUpdateStatus("partition not found!", dash::Status::DANGER);
          }
        }
        if (len && request->_tempObject) {
          const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(request->_tempObject);
          if (esp_partition_write_raw(partition, index, data, len) != ESP_OK) {
            website.setSafeBootUpdateStatus("Failed writing recovery partition!", dash::Status::DANGER);
            request->_tempObject = nullptr;
          }
        }
      });

  webServer
    .on(
      "/api/config/mqttServerCertificate",
      HTTP_POST,
      [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "OK");
        if (!LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE)) {
          request->send(400, "text/plain", "No PEM server certificate file uploaded");
        } else {
          File serverCertFile = LittleFS.open(YASOLR_MQTT_SERVER_CERT_FILE, "r");
          LOGI(TAG, "Uploaded MQTT PEM server certificate:\n%s", serverCertFile.readString().c_str());
          serverCertFile.close();
          dashboardInitTask.resume();
          request->send(response);
        }
      },
      [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (!index)
          request->_tempFile = LittleFS.open(YASOLR_MQTT_SERVER_CERT_FILE, "w");
        if (len)
          request->_tempFile.write(data, len);
        if (final)
          request->_tempFile.close();
      });

  webServer.on("/api/config", HTTP_POST, [](AsyncWebServerRequest* request) {
    std::map<const char*, std::string> settings;
    for (size_t i = 0, max = request->params(); i < max; i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if (p->isPost() && !p->isFile()) {
        const char* keyRef = config.keyRef(p->name().c_str());
        settings[keyRef] = p->value().c_str();
      }
    }
    request->send(200);
    config.set(settings);
  });

  webServer.on("/api/config", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse(false);
    config.toJson(response->getRoot());
    response->setLength();
    request->send(response);
  });

  // system

  webServer.on("/api/system/restart", HTTP_ANY, [](AsyncWebServerRequest* request) {
    restartTask.resume();
    request->send(200);
  });

  webServer.on("/api/system/safeboot", HTTP_ANY, [](AsyncWebServerRequest* request) {
    safeBootTask.resume();
    request->send(200);
  });

  webServer.on("/api/system/reset", HTTP_ANY, [](AsyncWebServerRequest* request) {
    resetTask.resume();
    request->send(200);
  });

  webServer.on("/api/system", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    root["app"]["manufacturer"] = Mycila::AppInfo.manufacturer;
    root["app"]["model"] = Mycila::AppInfo.model;
    root["app"]["name"] = Mycila::AppInfo.name;
    root["app"]["version"] = Mycila::AppInfo.version;

    root["device"]["boots"] = Mycila::System::getBootCount();
    root["device"]["cores"] = ESP.getChipCores();
    root["device"]["cpu_freq"] = ESP.getCpuFreqMHz();

    Mycila::System::Memory* memory = new Mycila::System::Memory();
    Mycila::System::getMemory(*memory);
    root["device"]["heap"]["total"] = memory->total;
    root["device"]["heap"]["usage"] = memory->usage;
    root["device"]["heap"]["used"] = memory->used;
    delete memory;
    memory = nullptr;

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

    root["network"]["hostname"] = espConnect.getConfig().hostname;
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

  webServer.on("/api/grid", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    Mycila::Grid::Metrics metrics;
    grid.getGridMeasurements(metrics);
    Mycila::Grid::toJson(root, metrics);
    response->setLength();
    request->send(response);
  });

  // router relays

  webServer.on("/api/router/relay1", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (relay1 && relay1->isEnabled() && request->hasParam("state", true)) {
      std::string state = request->getParam("state", true)->value().c_str();
      if (state == YASOLR_ON)
        relay1->trySwitchRelay(true);
      else if (state == YASOLR_OFF)
        relay1->trySwitchRelay(false);
    }
    request->send(200);
  });

  webServer.on("/api/router/relay2", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (relay2 && relay2->isEnabled() && request->hasParam("state", true)) {
      std::string state = request->getParam("state", true)->value().c_str();
      if (state == YASOLR_ON)
        relay2->trySwitchRelay(true);
      else if (state == YASOLR_OFF)
        relay2->trySwitchRelay(false);
    }
    request->send(200);
  });

  // router dimmers

  webServer.on("/api/router/output1/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (output1 && request->hasParam("duty_cycle", true)) {
      output1->setDimmerDutyCycle(request->getParam("duty_cycle", true)->value().toFloat() / 100.0f);
      request->send(200);
    } else {
      request->send(400);
    }
  });

  webServer.on("/api/router/output2/dimmer", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (output2 && request->hasParam("duty_cycle", true)) {
      output2->setDimmerDutyCycle(request->getParam("duty_cycle", true)->value().toFloat() / 100.0f);
      request->send(200);
    } else {
      request->send(400);
    }
  });

  // router bypass

  webServer.on("/api/router/output1/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (output1 && request->hasParam("state", true)) {
      std::string state = request->getParam("state", true)->value().c_str();
      if (state == YASOLR_ON)
        output1->setBypassOn();
      else if (state == YASOLR_OFF)
        output1->setBypassOff();
      request->send(200);
    } else {
      request->send(400);
    }
  });

  webServer.on("/api/router/output2/bypass", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (output2 && request->hasParam("state", true)) {
      std::string state = request->getParam("state", true)->value().c_str();
      if (state == YASOLR_ON)
        output2->setBypassOn();
      else if (state == YASOLR_OFF)
        output2->setBypassOff();
      request->send(200);
    } else {
      request->send(400);
    }
  });

  webServer.on("/api/router", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    Mycila::Router::Metrics routerMeasurements;
    router.getRouterMeasurements(routerMeasurements);

    root["lights"] = lights.toString();
    if (relay1)
      root["relay1"] = YASOLR_STATE(relay1->isOn());
    if (relay2)
      root["relay2"] = YASOLR_STATE(relay2->isOn());
    if (ds18Sys) {
      float t = ds18Sys->getTemperature().value_or(NAN);
      if (!std::isnan(t))
        root["temperature"] = t;
    }
    float virtual_grid_power = grid.getPower().value_or(NAN) - routerMeasurements.power;
    if (!std::isnan(virtual_grid_power))
      root["virtual_grid_power"] = virtual_grid_power;

    Mycila::Router::toJson(root["measurements"].to<JsonObject>(), routerMeasurements);

    for (const auto& output : router.getOutputs()) {
      JsonObject json = root[output->getName()].to<JsonObject>();
      json["state"] = output->getStateName();
      json["bypass"] = YASOLR_STATE(output->isBypassOn());
      json["dimmer"] = YASOLR_STATE(output->isDimmerOn());
      json["duty_cycle"] = output->getDimmerDutyCycle() * 100.0f;
      float t = output->temperature().orElse(NAN);
      if (!std::isnan(t)) {
        json["temperature"] = t;
      }

      Mycila::RouterOutput::Metrics outputMeasurements;
      output->getOutputMeasurements(outputMeasurements);
      Mycila::RouterOutput::toJson(json["measurements"].to<JsonObject>(), outputMeasurements);
    }

    response->setLength();
    request->send(response);
  });

  webServer.on("/api" YASOLR_LOG_FILE, HTTP_GET, [](AsyncWebServerRequest* request) {
    if (LittleFS.exists(YASOLR_LOG_FILE)) {
      LittleFS.open(YASOLR_LOG_FILE, "r");
      request->send(LittleFS, YASOLR_LOG_FILE, "text/plain");
    } else {
      request->send(404);
    }
  });

  webServer.on("/api", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    std::string base = "http://";
    base += espConnect.getIPAddress().toString().c_str();
    base += "/api";

    root["config"] = base + "/config";
    root["config/backup"] = base + "/config/backup";
    root["debug"] = base + "/debug";
    root["grid"] = base + "/grid";
    root["logs"] = base + YASOLR_LOG_FILE;
    root["router"] = base + "/router";
    root["system"] = base + "/system";

    response->setLength();
    request->send(response);
  });
}

void yasolr_init_web_server() {
  LOGI(TAG, "Initialize web server");

  // Middleware

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    loggingMiddleware.setOutput(Serial);
    webServer.addMiddleware(&loggingMiddleware);
  }

  authMiddleware.setAuthType(AsyncAuthType::AUTH_DIGEST);
  authMiddleware.setRealm("YaSolR");
  authMiddleware.setUsername(YASOLR_ADMIN_USERNAME);
  authMiddleware.setPassword(config.get(KEY_ADMIN_PASSWORD));
  authMiddleware.generateHash();
  webServer.addMiddleware(&authMiddleware);

  rewrites();
  routes();
  rest_api();

  // ESP-DASH

#ifdef APP_MODEL_PRO
  dashboard.setTitle(Mycila::AppInfo.nameModel.c_str());
#endif
  dashboard.onBeforeUpdate([](bool changes_only) {
    if (!changes_only) {
      LOGI(TAG, "Dashboard refresh requested!");
      website.initCards();
    }
  });

  // YaSolR

  website.begin();

  // Tasks

  dashboardInitTask.setEnabledWhen([]() { return espConnect.isConnected() && !dashboard.isAsyncAccessInProgress(); });

  dashboardUpdateTask.setEnabledWhen([]() { return espConnect.isConnected() && !dashboard.isAsyncAccessInProgress(); });
  dashboardUpdateTask.setInterval(1000);

  coreTaskManager.addTask(dashboardInitTask);
  coreTaskManager.addTask(dashboardUpdateTask);

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    dashboardUpdateTask.enableProfiling();
    dashboardInitTask.enableProfiling();
  }

  // Task Monitor

  Mycila::TaskMonitor.addTask("async_tcp"); // AsyncTCP (set stack size with CONFIG_ASYNC_TCP_STACK_SIZE)
}
