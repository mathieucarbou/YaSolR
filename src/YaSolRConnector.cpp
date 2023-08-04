// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolRConnector.h>

#include <MycilaAppInfo.h>
#include <MycilaBuzzer.h>
#include <MycilaConfig.h>
#include <MycilaGrid.h>
#include <MycilaHTTPd.h>
#include <MycilaJSY.h>
#include <MycilaLights.h>
#include <MycilaLogger.h>
#include <MycilaMQTT.h>
#include <MycilaNTP.h>
#include <MycilaRouter.h>
#include <MycilaSystem.h>
#include <MycilaTaskMonitor.h>
#include <MycilaTemperatureSensor.h>
#include <MycilaTime.h>
#include <MycilaZeroCrossDetection.h>

#include <YaSolR.h>
#include <YaSolRConfig.h>
#include <YaSolRHADiscoveryController.h>

#include <AsyncJson.h>
#include <LittleFS.h>
#include <MycilaESPConnect.h>
#include <map>

#define TAG "I/O"

void YaSolR::Connector::begin() {
  const String baseTopic = Mycila::Config.get(KEY_MQTT_TOPIC);

  // app

  Mycila::HTTPd.apiGET("/app", [](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    Mycila::AppInfo.toJson(response->getRoot());
    response->setLength();
    request->send(response);
  });

  // config

  Mycila::MQTT.subscribe(baseTopic + "/config/+/set", [](const String& topic, const String& payload) {
    const int end = topic.lastIndexOf("/");
    const int start = topic.lastIndexOf("/", end - 1);
    const String key = topic.substring(start + 1, end);
    const char* keyRef = Mycila::Config.keyRef(key.c_str());
    if (keyRef)
      Mycila::Config.set(keyRef, payload);
  });

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
#if ARDUINOJSON_VERSION_MAJOR == 6
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 4096);
#else
    AsyncJsonResponse* response = new AsyncJsonResponse(false);
#endif
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

  Mycila::MQTT.subscribe(baseTopic + "/relays/" NAME_RELAY1 "/state/set", [this](const String& topic, const String& payload) {
    if (Mycila::RelayManager.relays[0].isEnabled()) {
      int start = payload.indexOf("=");
      String state = start >= 0 ? payload.substring(0, start) : payload;
      uint32_t duration = start >= 0 ? payload.substring(start + 1).toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(0, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(0, false, duration);
    }
  });
  Mycila::MQTT.subscribe(baseTopic + "/relays/" NAME_RELAY2 "/state/set", [this](const String& topic, const String& payload) {
    if (Mycila::RelayManager.relays[1].isEnabled()) {
      int start = payload.indexOf("=");
      String state = start >= 0 ? payload.substring(0, start) : payload;
      uint32_t duration = start >= 0 ? payload.substring(start + 1).toInt() : 0;
      if (state == YASOLR_ON)
        Mycila::RelayManager.tryRelayState(1, true, duration);
      else if (state == YASOLR_OFF)
        Mycila::RelayManager.tryRelayState(1, false, duration);
    }
  });

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

  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT1 "/dimmer/level/set", [](const String& topic, const String& payload) {
    Mycila::Router.outputs[0].tryDimmerLevel(payload.toInt());
  });
  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT2 "/dimmer/level/set", [](const String& topic, const String& payload) {
    Mycila::Router.outputs[1].tryDimmerLevel(payload.toInt());
  });

  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT1 "/bypass_relay/state/set", [](const String& topic, const String& payload) {
    if (Mycila::Router.outputs[0].isBypassRelayEnabled()) {
      if (payload == YASOLR_ON)
        Mycila::Router.outputs[0].tryBypassRelayState(true);
      else if (payload == YASOLR_OFF)
        Mycila::Router.outputs[0].tryBypassRelayState(false);
    }
  });
  Mycila::MQTT.subscribe(baseTopic + "/router/" NAME_OUTPUT2 "/bypass_relay/state/set", [](const String& topic, const String& payload) {
    if (Mycila::Router.outputs[1].isBypassRelayEnabled()) {
      if (payload == YASOLR_ON)
        Mycila::Router.outputs[1].tryBypassRelayState(true);
      else if (payload == YASOLR_OFF)
        Mycila::Router.outputs[1].tryBypassRelayState(false);
    }
  });

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

  Mycila::MQTT.subscribe(baseTopic + "/system/restart", [](const String& topic, const String& payload) { Mycila::System.restart(); });

  Mycila::MQTT.subscribe(baseTopic + "/system/reset", [](const String& topic, const String& payload) { Mycila::System.reset(); });

  Mycila::HTTPd.apiANY("/system/restart", [](AsyncWebServerRequest* request) {
    request->send(200);
    Mycila::System.restart();
  });

  Mycila::HTTPd.apiANY("/system/reset", [](AsyncWebServerRequest* request) {
    request->send(200);
    Mycila::System.reset();
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
    _systemTempSensor->toJson(temp_sensor);
    response->setLength();
    request->send(response);
  });

  Mycila::MQTT.onConnect([this](void) {
    Mycila::Logger.info(TAG, "MQTT connected!");
    _publishHADiscovery = true;
  });
}

void YaSolR::Connector::publish() {
  _lastPublish = 0;
}

void YaSolR::Connector::loop() {
  const uint32_t start = millis();

  if (Mycila::MQTT.isConnected()) {
    if (millis() - _lastPublish >= Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt() * 1000) {
      _publish();
      yield();
    }

    if (_publishHADiscovery) {
      YaSolR::HADiscoveryController.publish();
      _publishHADiscovery = false;
      yield();
    }
  }

  const uint32_t elapsed = millis() - start;
  if (elapsed > 2000)
    Mycila::Logger.warn(TAG, "Abnormal loop duration: %u ms", elapsed);
}

void YaSolR::Connector::_publish() {
  if (Mycila::MQTT.isConnected()) {
    const uint32_t start = millis();
    const String baseTopic = Mycila::Config.get(KEY_MQTT_TOPIC);
    Mycila::Logger.debug(TAG, "Publishing...");

    // app
    {
      Mycila::MQTT.publish(baseTopic + "/app/firmware", Mycila::AppInfo.firmware);
      Mycila::MQTT.publish(baseTopic + "/app/id", Mycila::AppInfo.id);
      Mycila::MQTT.publish(baseTopic + "/app/manufacturer", Mycila::AppInfo.manufacturer);
      Mycila::MQTT.publish(baseTopic + "/app/model", Mycila::AppInfo.model);
      Mycila::MQTT.publish(baseTopic + "/app/name", Mycila::AppInfo.name);
      Mycila::MQTT.publish(baseTopic + "/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial));
      Mycila::MQTT.publish(baseTopic + "/app/version", Mycila::AppInfo.version);
    }
    yield();

    // config
    {
      for (auto& key : Mycila::Config.keys) {
        String value = Mycila::Config.get(key);
        if (!value.isEmpty() && Mycila::Config.isPasswordKey(key))
          value = "********";
        Mycila::MQTT.publish(baseTopic + "/config/" + key, value);
      }
    }
    yield();

    // grid
    {
      Mycila::MQTT.publish(baseTopic + "/grid/frequency", String(Mycila::Grid.getFrequency()));
      Mycila::MQTT.publish(baseTopic + "/grid/power", String(Mycila::Grid.getPower()));
      Mycila::MQTT.publish(baseTopic + "/grid/voltage", String(Mycila::Grid.getVoltage()));
      Mycila::MQTT.publish(baseTopic + "/grid/online", YASOLR_BOOL(Mycila::Grid.isOnline()));
    }
    yield();

    // network
    {
      Mycila::MQTT.publish(baseTopic + "/network/ip_address", ESPConnect.getIPAddress().toString());
      Mycila::MQTT.publish(baseTopic + "/network/mac_address", ESPConnect.getMacAddress());
      Mycila::MQTT.publish(baseTopic + "/network/wifi_bssid", ESPConnect.getWiFiBSSID());
      Mycila::MQTT.publish(baseTopic + "/network/wifi_quality", String(ESPConnect.getWiFiSignalQuality()));
      Mycila::MQTT.publish(baseTopic + "/network/wifi_rssi", String(ESPConnect.getWiFiRSSI()));
      Mycila::MQTT.publish(baseTopic + "/network/wifi_ssid", ESPConnect.getWiFiSSID());
    }

    yield();

    // ntp
    {
      Mycila::MQTT.publish(baseTopic + "/ntp/synced", YASOLR_BOOL(Mycila::NTP.isSynced()));
    }
    yield();

    // relays
    {
      for (size_t i = 0; i < Mycila::RelayManager.relays.size(); i++) {
        auto& relay = Mycila::RelayManager.relays[i];
        Mycila::MQTT.publish(baseTopic + "/relays/" + relay.name + "/enabled", YASOLR_BOOL(relay.isEnabled()));
        Mycila::MQTT.publish(baseTopic + "/relays/" + relay.name + "/state", YASOLR_STATE(relay.isOn()));
        Mycila::MQTT.publish(baseTopic + "/relays/" + relay.name + "/switch_count", String(relay.getSwitchCount()));
      }
    }
    yield();

    // router
    {
      Mycila::MQTT.publish(baseTopic + "/router/energy", String(Mycila::Router.getTotalRoutedEnergy()));
      Mycila::MQTT.publish(baseTopic + "/router/power_factor", String(Mycila::Router.getTotalPowerFactor()));
      Mycila::MQTT.publish(baseTopic + "/router/power", String(Mycila::Router.getTotalRoutedPower()));
      Mycila::MQTT.publish(baseTopic + "/router/thdi", String(Mycila::Router.getTotalTHDi()));
      Mycila::MQTT.publish(baseTopic + "/router/virtual_grid_power", String(Mycila::Router.getVirtualGridPower()));
      for (int i = 0; i < Mycila::Router.outputs.size(); i++) {
        auto& output = Mycila::Router.outputs[i];
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/enabled", YASOLR_BOOL(output.isEnabled()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/state", output.getStateString());

        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/dimmer/enabled", YASOLR_BOOL(output.dimmer.isEnabled()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/dimmer/level", String(output.dimmer.getLevel()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/dimmer/state", YASOLR_STATE(output.dimmer.isOn()));

        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/temp_sensor/enabled", YASOLR_BOOL(output.temperatureSensor.isEnabled()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/temp_sensor/temperature", String(output.temperatureSensor.getTemperature()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/temp_sensor/valid", YASOLR_BOOL(output.temperatureSensor.isValid()));

        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/bypass_relay/enabled", YASOLR_BOOL(output.relay.isEnabled()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/bypass_relay/state", YASOLR_STATE(output.relay.isOn()));
        Mycila::MQTT.publish(baseTopic + "/router/" + output.name + "/bypass_relay/switch_count", String(output.relay.getSwitchCount()));
      }
    }
    yield();

    // system
    {
      Mycila::SystemMemory memory = Mycila::System.getMemory();
      esp_chip_info_t chip_info;
      esp_chip_info(&chip_info);
      Mycila::MQTT.publish(baseTopic + "/system/boots", String(Mycila::System.getBootCount()));
      Mycila::MQTT.publish(baseTopic + "/system/chip_cores", String(chip_info.cores));
      Mycila::MQTT.publish(baseTopic + "/system/chip_model", ESP.getChipModel());
      Mycila::MQTT.publish(baseTopic + "/system/chip_revision", String(chip_info.revision));
      Mycila::MQTT.publish(baseTopic + "/system/cpu_freq", String(ESP.getCpuFreqMHz()));
      Mycila::MQTT.publish(baseTopic + "/system/heap_total", String(memory.total));
      Mycila::MQTT.publish(baseTopic + "/system/heap_usage", String(memory.usage));
      Mycila::MQTT.publish(baseTopic + "/system/heap_used", String(memory.used));
      Mycila::MQTT.publish(baseTopic + "/system/uptime", String(Mycila::System.getUptime()));
      Mycila::MQTT.publish(baseTopic + "/system/lights/code", Mycila::Lights.toString());
      Mycila::MQTT.publish(baseTopic + "/system/lights/green", YASOLR_STATE(Mycila::Lights.isGreenOn()));
      Mycila::MQTT.publish(baseTopic + "/system/lights/red", YASOLR_STATE(Mycila::Lights.isRedOn()));
      Mycila::MQTT.publish(baseTopic + "/system/lights/yellow", YASOLR_STATE(Mycila::Lights.isYellowOn()));
      Mycila::MQTT.publish(baseTopic + "/system/temp_sensor/enabled", YASOLR_BOOL(_systemTempSensor->isEnabled()));
      Mycila::MQTT.publish(baseTopic + "/system/temp_sensor/temperature", String(_systemTempSensor->getTemperature()));
      Mycila::MQTT.publish(baseTopic + "/system/temp_sensor/valid", YASOLR_BOOL(_systemTempSensor->isValid()));
    }
    yield();

    _lastPublish = millis();
    Mycila::Logger.debug(TAG, "Published in %u ms", (_lastPublish - start));
  }
}
