// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

#include <string>
#include <utility>

extern const uint8_t ca_certs_bundle_start[] asm("_binary__pio_embed_cacerts_bin_start");
extern const uint8_t ca_certs_bundle_end[] asm("_binary__pio_embed_cacerts_bin_end");

Mycila::MQTT* mqtt = nullptr;
Mycila::Task* mqttConnectTask = nullptr;
Mycila::Task* mqttPublishConfigTask = nullptr;
Mycila::Task* mqttPublishStaticTask = nullptr;
Mycila::Task* mqttPublishTask = nullptr;
Mycila::Task* haDiscoveryTask = nullptr;

static Mycila::ExpiringValue<float>* voltage = nullptr;
static Mycila::ExpiringValue<float>* power = nullptr;

static void connect() {
  mqtt->end();

  bool secured = config.get<bool>(KEY_MQTT_SECURED);

  Mycila::MQTT::Config mqttConfig;
  mqttConfig.server = config.getString(KEY_MQTT_SERVER);
  mqttConfig.port = config.get<uint16_t>(KEY_MQTT_PORT);
  mqttConfig.secured = secured;
  mqttConfig.username = config.getString(KEY_MQTT_USERNAME);
  mqttConfig.password = config.getString(KEY_MQTT_PASSWORD);
  mqttConfig.clientId = Mycila::AppInfo.defaultMqttClientId;
  mqttConfig.willTopic = std::string(config.getString(KEY_MQTT_TOPIC)) + YASOLR_MQTT_WILL_TOPIC;
  mqttConfig.keepAlive = YASOLR_MQTT_KEEPALIVE;

  if (secured) {
    // if a server certificate has been used, set it
    if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE)) {
      ESP_LOGI(TAG, "Loading MQTT PEM server certificate");
      File serverCertFile = LittleFS.open(YASOLR_MQTT_SERVER_CERT_FILE, "r");
      mqttConfig.serverCert = serverCertFile.readString().c_str();
      serverCertFile.close();
      ESP_LOGI(TAG, "Loaded MQTT server certificate:\n%s", mqttConfig.serverCert.c_str());
    } else {
      ESP_LOGI(TAG, "Using cacert bundle for MQTT");
      // if no server certificate has been used, set default CA certs bundle
      mqttConfig.certBundle = ca_certs_bundle_start;
      mqttConfig.certBundleSize = static_cast<size_t>(ca_certs_bundle_end - ca_certs_bundle_start);
    }
  }

  ESP_LOGI(TAG, "Connecting to MQTT broker %s://%s:%" PRIu16 "", (secured ? "mqtts" : "mqtt"), mqttConfig.server.c_str(), mqttConfig.port);
  mqtt->setAsync(false);
  mqtt->begin(mqttConfig);
}

static void subscribe() {
  ESP_LOGI(TAG, "Subscribing to MQTT topics");

  std::string baseTopic = config.getString(KEY_MQTT_TOPIC);

  // config

  mqtt->subscribe(baseTopic + "/config/+/set", [](const std::string& topic, const std::string_view& payload) {
    const std::size_t end = topic.rfind("/set");
    if (end == std::string::npos)
      return;
    const std::size_t start = topic.rfind("/", end - 1);
    const Mycila::config::Key* key = config.key(topic.substr(start + 1, end - start - 1).c_str());
    if (key != nullptr) {
      std::optional<Mycila::config::Value> converted = Mycila::config::Value::fromString(std::string(payload).c_str(), key->defaultValue);
      if (converted.has_value()) {
        config.set(key->name, std::move(converted.value()));
      }
    }
  });

  // relays

  mqtt->subscribe(baseTopic + "/router/relay1/set", [](const std::string& topic, const std::string_view& payload) {
    if (relay1 && relay1->isEnabled()) {
      const std::string_view state = payload.substr(0, payload.find("="));
      if (state.empty())
        return;
      if (state == YASOLR_ON)
        relay1->trySwitchRelay(true);
      else if (state == YASOLR_OFF)
        relay1->trySwitchRelay(false);
    }
  });

  mqtt->subscribe(baseTopic + "/router/relay2/set", [](const std::string& topic, const std::string_view& payload) {
    if (relay2 && relay2->isEnabled()) {
      const std::string_view state = payload.substr(0, payload.find("="));
      if (state.empty())
        return;

      if (state == YASOLR_ON)
        relay2->trySwitchRelay(true);
      else if (state == YASOLR_OFF)
        relay2->trySwitchRelay(false);
    }
  });

  // router

  mqtt->subscribe(baseTopic + "/router/output1/duty_cycle/set", [](const std::string& topic, const std::string_view& payload) {
    float duty;
    if (std::from_chars(payload.begin(), payload.end(), duty).ec == std::errc{}) {
      output1.setDimmerDutyCycle(duty / 100.0f);
    }
  });

  mqtt->subscribe(baseTopic + "/router/output2/duty_cycle/set", [](const std::string& topic, const std::string_view& payload) {
    float duty;
    if (std::from_chars(payload.begin(), payload.end(), duty).ec == std::errc{}) {
      output2.setDimmerDutyCycle(duty / 100.0f);
    }
  });

  mqtt->subscribe(baseTopic + "/router/output1/bypass/set", [](const std::string& topic, const std::string_view& payload) {
    if (payload == YASOLR_ON)
      output1.setBypassOn();
    else if (payload == YASOLR_OFF)
      output1.setBypassOff();
  });

  mqtt->subscribe(baseTopic + "/router/output2/bypass/set", [](const std::string& topic, const std::string_view& payload) {
    if (payload == YASOLR_ON)
      output2.setBypassOn();
    else if (payload == YASOLR_OFF)
      output2.setBypassOff();
  });

  // device

  mqtt->subscribe(baseTopic + "/system/device/restart", [](const std::string& topic, const std::string_view& payload) {
    restartTask.resume();
  });

  // grid power
  const char* gridPowerMQTTTopic = config.getString(KEY_GRID_POWER_MQTT_TOPIC);
  if (gridPowerMQTTTopic[0] != '\0') {
    ESP_LOGI(TAG, "Reading Grid Power from MQTT topic: %s", gridPowerMQTTTopic);
    mqtt->subscribe(gridPowerMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      if (payload.length()) {
        float p = NAN;

        // check if first character is '{' for json data
        if (payload[0] == '{') {
          JsonDocument doc;
          if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            // Shelly EM example: shellyproem50/status/em1:0
            // {"id":1,"current":2.681,"voltage":236.7,"act_power":-607.3,"aprt_power":636.0,"pf":0.95,"freq":50.0,"calibration":"factory"}
            // Shelly 3EM example: shellypowermeter/status/em:0
            // {"id":0,"a_current":0.132,"a_voltage":236.0,"a_act_power":3.9,"a_aprt_power":31.0,"a_pf":-0.53,"b_current":0.594,"b_voltage":236.4,"b_act_power":45.4,"b_aprt_power":140.3,"b_pf":-0.61,"c_current":0.368,"c_voltage":237.8,"c_act_power":54.3,"c_aprt_power":87.5,"c_pf":-0.72,"n_current":null,"total_current":1.094,"total_act_power":103.610,"total_aprt_power":258.799, "user_calibrated_phase":[]}
            p = doc["act_power"] | (doc["total_act_power"] | NAN);
          }

        } else {
          // direct value ?
          if (std::from_chars(payload.begin(), payload.end(), p).ec != std::errc{})
            p = NAN;
        }

        if (!isnan(p)) {
          ESP_LOGI(TAG, "Grid Power from MQTT: %f", p);
          power->update(p);
          Mycila::Grid::Metrics metrics;
          metrics.source = Mycila::Grid::Source::MQTT;
          metrics.power = power->orElse(NAN);
          metrics.voltage = voltage->orElse(NAN);
          grid.updateMetrics(std::move(metrics));
          if (grid.isUsing(Mycila::Grid::Source::MQTT)) {
            pidTask.requestEarlyRun();
          }
        }
      }
    });
  }

  // grid voltage
  const char* gridVoltageMQTTTopic = config.getString(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  if (gridVoltageMQTTTopic[0] != '\0') {
    ESP_LOGI(TAG, "Reading Grid Voltage from MQTT topic: %s", gridVoltageMQTTTopic);
    mqtt->subscribe(gridVoltageMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      if (payload.length()) {
        float v = NAN;

        // check if first character is '{' for json data
        if (payload[0] == '{') {
          JsonDocument doc;
          if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            // Shelly EM example: shellyproem50/status/em1:0
            // {"id":1,"current":2.681,"voltage":236.7,"act_power":-607.3,"aprt_power":636.0,"pf":0.95,"freq":50.0,"calibration":"factory"}
            // Shelly 3EM example: shellypowermeter/status/em:0
            // {"id":0,"a_current":0.132,"a_voltage":236.0,"a_act_power":3.9,"a_aprt_power":31.0,"a_pf":-0.53,"b_current":0.594,"b_voltage":236.4,"b_act_power":45.4,"b_aprt_power":140.3,"b_pf":-0.61,"c_current":0.368,"c_voltage":237.8,"c_act_power":54.3,"c_aprt_power":87.5,"c_pf":-0.72,"n_current":null,"total_current":1.094,"total_act_power":103.610,"total_aprt_power":258.799, "user_calibrated_phase":[]}
            v = doc["voltage"] | (doc["a_voltage"] | (doc["b_voltage"] | (doc["c_voltage"] | NAN)));
          }

        } else {
          // direct value
          if (std::from_chars(payload.begin(), payload.end(), v).ec != std::errc{})
            v = NAN;
        }

        if (!isnan(v)) {
          ESP_LOGI(TAG, "Grid Voltage from MQTT: %f", v);
          voltage->update(v);
          Mycila::Grid::Metrics metrics;
          metrics.source = Mycila::Grid::Source::MQTT;
          metrics.power = power->orElse(NAN);
          metrics.voltage = voltage->orElse(NAN);
          grid.updateMetrics(std::move(metrics));
        }
      }
    });
  }

  // output 1 temperature
  const char* output1TemperatureMQTTTopic = config.getString(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  if (output1TemperatureMQTTTopic[0] != '\0') {
    ESP_LOGI(TAG, "Reading Output 1 Temperature from MQTT topic: %s", output1TemperatureMQTTTopic);
    mqtt->subscribe(output1TemperatureMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      float t;
      if (std::from_chars(payload.begin(), payload.end(), t).ec == std::errc{}) {
        ESP_LOGI(TAG, "Output 1 Temperature from MQTT: %f", t);
        if (!output1.temperature().update(t).has_value()) {
          // if this is the first time we get the temperature, we can trigger the dashboard init task
          dashboardInitTask.resume();
        }
      }
    });
  }

  // output 2 temperature
  const char* output2TemperatureMQTTTopic = config.getString(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  if (output2TemperatureMQTTTopic[0] != '\0') {
    ESP_LOGI(TAG, "Reading Output 2 Temperature from MQTT topic: %s", output2TemperatureMQTTTopic);
    mqtt->subscribe(output2TemperatureMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      float t;
      if (std::from_chars(payload.begin(), payload.end(), t).ec == std::errc{}) {
        ESP_LOGI(TAG, "Output 2 Temperature from MQTT: %f", t);
        if (!output2.temperature().update(t).has_value()) {
          // if this is the first time we get the temperature, we can trigger the dashboard init task
          dashboardInitTask.resume();
        }
      }
    });
  }
}

static void publishConfig() {
  ESP_LOGI(TAG, "Publishing config to MQTT");
  std::string baseTopic = config.getString(KEY_MQTT_TOPIC);
  for (auto& key : config.keys()) {
    std::string value = config.get(key.name).toString();
    // ESP_LOGD(TAG, "%s: %s", key.name, value.c_str());
    if (value.length() && key.isPasswordKey())
      value = "********";
    mqtt->publish((baseTopic + "/config/" + key.name).c_str(), value, true);
  }
  ESP_LOGI(TAG, "Published config to MQTT");
}

static void publishStaticData() {
  ESP_LOGI(TAG, "Publishing static data to MQTT...");

  std::string baseTopic = config.getString(KEY_MQTT_TOPIC);

  mqtt->publish((baseTopic + "/system/app/manufacturer").c_str(), Mycila::AppInfo.manufacturer, true);
  mqtt->publish((baseTopic + "/system/app/model").c_str(), Mycila::AppInfo.model, true);
  mqtt->publish((baseTopic + "/system/app/name").c_str(), Mycila::AppInfo.name, true);
  mqtt->publish((baseTopic + "/system/app/trial").c_str(), YASOLR_BOOL(Mycila::AppInfo.trial), true);
  mqtt->publish((baseTopic + "/system/app/version").c_str(), Mycila::AppInfo.version, true);
  yield();

  mqtt->publish((baseTopic + "/system/device/boots").c_str(), std::to_string(Mycila::System::getBootCount()), true);
  mqtt->publish((baseTopic + "/system/device/cores").c_str(), std::to_string(ESP.getChipCores()), true);
  mqtt->publish((baseTopic + "/system/device/cpu_freq").c_str(), std::to_string(ESP.getCpuFreqMHz()), true);
  mqtt->publish((baseTopic + "/system/device/id").c_str(), Mycila::AppInfo.id, true);
  mqtt->publish((baseTopic + "/system/device/model").c_str(), ESP.getChipModel(), true);
  yield();

  mqtt->publish((baseTopic + "/system/firmware/build/branch").c_str(), Mycila::AppInfo.buildBranch, true);
  mqtt->publish((baseTopic + "/system/firmware/build/env").c_str(), Mycila::AppInfo.buildEnv, true);
  mqtt->publish((baseTopic + "/system/firmware/build/hash").c_str(), Mycila::AppInfo.buildHash, true);
  mqtt->publish((baseTopic + "/system/firmware/build/timestamp").c_str(), Mycila::AppInfo.buildDate, true);
  mqtt->publish((baseTopic + "/system/firmware/debug").c_str(), YASOLR_BOOL(Mycila::AppInfo.debug), true);
  yield();

  mqtt->publish((baseTopic + "/system/network/eth/mac_address").c_str(), espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH), true);
  mqtt->publish((baseTopic + "/system/network/hostname").c_str(), espConnect.getConfig().hostname.c_str(), true);
  mqtt->publish((baseTopic + "/system/network/wifi/mac_address").c_str(), espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA), true);
  yield();

  ESP_LOGI(TAG, "Published static data to MQTT");
}

static void publishData() {
  std::string baseTopic = config.getString(KEY_MQTT_TOPIC);

  mqtt->publish((baseTopic + "/system/app/latest_version").c_str(), Mycila::AppInfo.latestVersion);

  {
    Mycila::System::Memory* memory = new Mycila::System::Memory();
    Mycila::System::getMemory(*memory);
    mqtt->publish((baseTopic + "/system/device/heap/total").c_str(), std::to_string(memory->total));
    mqtt->publish((baseTopic + "/system/device/heap/usage").c_str(), std::to_string(memory->usage));
    mqtt->publish((baseTopic + "/system/device/heap/used").c_str(), std::to_string(memory->used));
    mqtt->publish((baseTopic + "/system/device/uptime").c_str(), std::to_string(Mycila::System::getUptime()));
    delete memory;
    yield();
  }

  {
    mqtt->publish((baseTopic + "/system/network/eth/ip_address").c_str(), espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
    mqtt->publish((baseTopic + "/system/network/ip_address").c_str(), espConnect.getIPAddress().toString().c_str());
    mqtt->publish((baseTopic + "/system/network/ip6_link_local_address").c_str(), espConnect.getLinkLocalIPv6Address().toString().c_str());
    mqtt->publish((baseTopic + "/system/network/ip6_global_address").c_str(), espConnect.getGlobalIPv6Address().toString().c_str());
    mqtt->publish((baseTopic + "/system/network/mac_address").c_str(), espConnect.getMACAddress());
    mqtt->publish((baseTopic + "/system/network/ntp").c_str(), YASOLR_STATE(Mycila::NTP.isSynced()));
    mqtt->publish((baseTopic + "/system/network/wifi/bssid").c_str(), espConnect.getWiFiBSSID());
    mqtt->publish((baseTopic + "/system/network/wifi/ip_address").c_str(), espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
    mqtt->publish((baseTopic + "/system/network/wifi/quality").c_str(), std::to_string(espConnect.getWiFiSignalQuality()));
    mqtt->publish((baseTopic + "/system/network/wifi/rssi").c_str(), std::to_string(espConnect.getWiFiRSSI()));
    mqtt->publish((baseTopic + "/system/network/wifi/ssid").c_str(), espConnect.getWiFiSSID());

    switch (espConnect.getMode()) {
      case Mycila::ESPConnect::Mode::ETH:
        mqtt->publish((baseTopic + "/system/network/mode").c_str(), "eth");
        break;
      case Mycila::ESPConnect::Mode::STA:
        mqtt->publish((baseTopic + "/system/network/mode").c_str(), "wifi");
        break;
      case Mycila::ESPConnect::Mode::AP:
        mqtt->publish((baseTopic + "/system/network/mode").c_str(), "ap");
        break;
      default:
        mqtt->publish((baseTopic + "/system/network/mode").c_str(), "");
        break;
    }

    yield();
  }

  {
    Mycila::Grid::Metrics* gridMetrics = new Mycila::Grid::Metrics();
    grid.readMeasurements(*gridMetrics);

    Mycila::Router::Metrics* routerMeasurements = new Mycila::Router::Metrics();
    if (!router.readMeasurements(*routerMeasurements)) {
      router.computeMetrics(*routerMeasurements, gridMetrics->voltage);
    }

    float virtual_grid_power = gridMetrics->power - routerMeasurements->power;

    mqtt->publish((baseTopic + "/grid/apparent_power").c_str(), std::to_string(gridMetrics->apparentPower));
    mqtt->publish((baseTopic + "/grid/current").c_str(), std::to_string(gridMetrics->current));
    mqtt->publish((baseTopic + "/grid/energy").c_str(), std::to_string(gridMetrics->energy));
    mqtt->publish((baseTopic + "/grid/energy_returned").c_str(), std::to_string(gridMetrics->energyReturned));
    mqtt->publish((baseTopic + "/grid/frequency").c_str(), std::to_string(gridMetrics->frequency));
    mqtt->publish((baseTopic + "/grid/online").c_str(), YASOLR_BOOL(grid.isConnected()));
    mqtt->publish((baseTopic + "/grid/power").c_str(), std::to_string(gridMetrics->power));
    mqtt->publish((baseTopic + "/grid/power_factor").c_str(), std::to_string(gridMetrics->powerFactor));
    mqtt->publish((baseTopic + "/grid/voltage").c_str(), std::to_string(gridMetrics->voltage));

    mqtt->publish((baseTopic + "/router/apparent_power").c_str(), std::to_string(routerMeasurements->apparentPower));
    mqtt->publish((baseTopic + "/router/current").c_str(), std::to_string(routerMeasurements->current));
    mqtt->publish((baseTopic + "/router/energy").c_str(), std::to_string(routerMeasurements->energy));
    mqtt->publish((baseTopic + "/router/power_factor").c_str(), std::isnan(routerMeasurements->powerFactor) ? "0" : std::to_string(routerMeasurements->powerFactor));
    mqtt->publish((baseTopic + "/router/power").c_str(), std::to_string(routerMeasurements->power));
    mqtt->publish((baseTopic + "/router/thdi").c_str(), std::isnan(routerMeasurements->thdi) ? "0" : std::to_string(routerMeasurements->thdi));

    mqtt->publish((baseTopic + "/router/virtual_grid_power").c_str(), std::isnan(virtual_grid_power) ? "0" : std::to_string(virtual_grid_power));

    delete gridMetrics;
    delete routerMeasurements;

    yield();
  }

  {
    mqtt->publish((baseTopic + "/router/status").c_str(), lights.toString());

    if (relay1)
      mqtt->publish((baseTopic + "/router/relay1").c_str(), YASOLR_STATE(relay1->isOn()));
    if (relay2)
      mqtt->publish((baseTopic + "/router/relay2").c_str(), YASOLR_STATE(relay2->isOn()));
    if (ds18Sys)
      mqtt->publish((baseTopic + "/router/temperature").c_str(), std::to_string(ds18Sys->getTemperature().value_or(0)));

    for (const auto& output : router.getOutputs()) {
      const std::string outputTopic = baseTopic + "/router/" + output->getName();
      mqtt->publish((outputTopic + "/state").c_str(), output->getStateName());
      mqtt->publish((outputTopic + "/bypass").c_str(), YASOLR_STATE(output->isBypassOn()));
      mqtt->publish((outputTopic + "/dimmer").c_str(), YASOLR_STATE(output->isDimmerOn()));
      mqtt->publish((outputTopic + "/duty_cycle").c_str(), std::to_string(output->getDimmerDutyCycle() * 100.0f));
      mqtt->publish((outputTopic + "/temperature").c_str(), std::to_string(output->temperature().orElse(0)));
    }

    yield();
  }
}

static void haDiscovery() {
  ESP_LOGI(TAG, "Publishing Home Assistant Discovery configuration");

  Mycila::HA::Discovery* haDiscovery = new Mycila::HA::Discovery();

  haDiscovery->setDiscoveryTopic(config.getString(KEY_HA_DISCOVERY_TOPIC));
  haDiscovery->setWillTopic(YASOLR_MQTT_WILL_TOPIC_HA);
  haDiscovery->begin({
                       .id = Mycila::AppInfo.defaultMqttClientId,
                       .name = Mycila::AppInfo.name + " " + Mycila::AppInfo.model + " " + Mycila::AppInfo.id,
                       .version = Mycila::AppInfo.version,
                       .model = Mycila::AppInfo.name + " " + Mycila::AppInfo.model,
                       .manufacturer = Mycila::AppInfo.manufacturer,
                       .url = std::string("http://") + espConnect.getIPAddress().toString().c_str(),
                     },
                     config.getString(KEY_MQTT_TOPIC),
                     512,
                     [](const char* topic, const std::string& payload) { mqtt->publish(topic, payload, true); });

  // CLEAR REMOVED ENTRIES

  haDiscovery->unpublish(std::make_unique<Mycila::HA::Value>("firmware_filename", nullptr, nullptr));

  // UPDATE

  haDiscovery->publish(std::make_unique<Mycila::HA::Update>("firmware_update", "Firmware Update", YASOLR_RELEASE_URL, "~/system/app/version", "~/system/app/latest_version"));

  // DIAGNOSTIC

  haDiscovery->publish(std::make_unique<Mycila::HA::Button>("device_restart", "Device: Restart", "~/system/device/restart", "restart", nullptr, Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Counter>("device_boots", "Device: Boot Count", "~/system/device/boots", nullptr, nullptr, nullptr, Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Counter>("device_uptime", "Device: Uptime", "~/system/device/uptime", "duration", nullptr, "s", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("device_heap_usage", "Device: Heap Usage", "~/system/device/heap/usage", nullptr, "mdi:memory", "%", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("device_heap_used", "Device: Heap Used", "~/system/device/heap/used", "data_size", "mdi:memory", "B", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("network_wifi_quality", "Net: WiFi Signal", "~/system/network/wifi/quality", nullptr, "mdi:signal", "%", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("network_wifi_rssi", "Net: WiFi RSSI", "~/system/network/wifi/rssi", "signal_strength", "mdi:signal", "dBm", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("device_id", "Device: ID", "~/system/device/id", nullptr, "mdi:identifier", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_eth_mac_address", "Net: Eth MAC Address", "~/system/network/eth/mac_address", nullptr, "mdi:lan", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_hostname", "Net: Hostname", "~/system/network/hostname", nullptr, "mdi:lan", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_ip_address", "Net: IPv4 Address", "~/system/network/ip_address", nullptr, "mdi:ip", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_ip6_global_address", "Net: IPv6 Global Address", "~/system/network/ip6_global_address", nullptr, "mdi:ip", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_ip6_link_local_address", "Net: IPv6 Link-local Address", "~/system/network/ip6_link_local_address", nullptr, "mdi:ip", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_wifi_mac_address", "Net: WiFi MAC Address", "~/system/network/wifi/mac_address", nullptr, "mdi:lan", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("network_wifi_ssid", "Net: WiFi SSID", "~/system/network/wifi/ssid", nullptr, "mdi:wifi", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery->publish(std::make_unique<Mycila::HA::State>("network_ntp", "Net: NTP", "~/system/network/ntp", YASOLR_ON, YASOLR_OFF, "connectivity", nullptr, Mycila::HA::Category::DIAGNOSTIC));
  yield();

  // CONFIG

  haDiscovery->publish(std::make_unique<Mycila::HA::Number>("output1_dimmer_limiter", "Output 1 Limiter", "~/config/" KEY_OUTPUT1_DIMMER_LIMIT "/set", "~/config/" KEY_OUTPUT1_DIMMER_LIMIT, Mycila::HA::NumberMode::SLIDER, 0, 100, 1, "mdi:flash", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Switch>("output1_auto_bypass", "Output 1 Auto Bypass", "~/config/" KEY_ENABLE_OUTPUT1_AUTO_BYPASS "/set", "~/config/" KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Switch>("output1_auto_dimmer", "Output 1 Auto Dimmer", "~/config/" KEY_ENABLE_OUTPUT1_AUTO_DIMMER "/set", "~/config/" KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output1_wdays", "Output 1 Week Days", "~/config/" KEY_OUTPUT1_DAYS "/set", "~/config/" KEY_OUTPUT1_DAYS, nullptr, "mdi:calendar", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output1_temperature_start", "Output 1 Temperature Start", "~/config/" KEY_OUTPUT1_TEMPERATURE_START "/set", "~/config/" KEY_OUTPUT1_TEMPERATURE_START, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output1_temperature_stop", "Output 1 Temperature Stop", "~/config/" KEY_OUTPUT1_TEMPERATURE_STOP "/set", "~/config/" KEY_OUTPUT1_TEMPERATURE_STOP, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output1_time_start", "Output 1 Time Start", "~/config/" KEY_OUTPUT1_TIME_START "/set", "~/config/" KEY_OUTPUT1_TIME_START, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output1_time_stop", "Output 1 Time Stop", "~/config/" KEY_OUTPUT1_TIME_STOP "/set", "~/config/" KEY_OUTPUT1_TIME_STOP, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HA::Category::CONFIG));
  yield();

  haDiscovery->publish(std::make_unique<Mycila::HA::Number>("output2_dimmer_limiter", "Output 2 Limiter", "~/config/" KEY_OUTPUT2_DIMMER_LIMIT "/set", "~/config/" KEY_OUTPUT2_DIMMER_LIMIT, Mycila::HA::NumberMode::SLIDER, 0, 100, 1, "mdi:flash", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Switch>("output2_auto_bypass", "Output 2 Auto Bypass", "~/config/" KEY_ENABLE_OUTPUT2_AUTO_BYPASS "/set", "~/config/" KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Switch>("output2_auto_dimmer", "Output 2 Auto Dimmer", "~/config/" KEY_ENABLE_OUTPUT2_AUTO_DIMMER "/set", "~/config/" KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output2_wdays", "Output 2 Week Days", "~/config/" KEY_OUTPUT2_DAYS "/set", "~/config/" KEY_OUTPUT2_DAYS, nullptr, "mdi:calendar", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output2_temperature_start", "Output 2 Temperature Start", "~/config/" KEY_OUTPUT2_TEMPERATURE_START "/set", "~/config/" KEY_OUTPUT2_TEMPERATURE_START, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output2_temperature_stop", "Output 2 Temperature Stop", "~/config/" KEY_OUTPUT2_TEMPERATURE_STOP "/set", "~/config/" KEY_OUTPUT2_TEMPERATURE_STOP, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output2_time_start", "Output 2 Time Start", "~/config/" KEY_OUTPUT2_TIME_START "/set", "~/config/" KEY_OUTPUT2_TIME_START, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HA::Category::CONFIG));
  haDiscovery->publish(std::make_unique<Mycila::HA::Text>("output2_time_stop", "Output 2 Time Stop", "~/config/" KEY_OUTPUT2_TIME_STOP "/set", "~/config/" KEY_OUTPUT2_TIME_STOP, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HA::Category::CONFIG));
  yield();

  // SENSORS

  haDiscovery->publish(std::make_unique<Mycila::HA::State>("grid", "Grid Electricity", "~/grid/online", YASOLR_TRUE, YASOLR_FALSE, "power"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Counter>("grid_energy", "Grid Energy", "~/grid/energy", "energy", nullptr, "Wh"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Counter>("grid_energy_returned", "Grid Energy Returned", "~/grid/energy_returned", "energy", nullptr, "Wh"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("grid_frequency", "Grid Frequency", "~/grid/frequency", "frequency", nullptr, "Hz"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("grid_power", "Grid Power", "~/grid/power", "power", nullptr, "W"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("grid_power_virtual", "Grid Power Without Routing", "~/router/virtual_grid_power", "power", nullptr, "W"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("grid_power_factor", "Grid Power Factor", "~/grid/power_factor", "power_factor"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("grid_voltage", "Grid Voltage", "~/grid/voltage", "voltage", nullptr, "V"));
  yield();

  haDiscovery->publish(std::make_unique<Mycila::HA::Counter>("routed_energy", "Routed Energy", "~/router/energy", "energy", nullptr, "Wh"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("routed_power", "Routed Power", "~/router/power", "power", nullptr, "W"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("router_power_factor", "Router Power Factor", "~/router/power_factor", "power_factor"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("router_temperature", "Router Temperature", "~/router/temperature", "temperature", "mdi:thermometer", "°C"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("router_status", "Router Status", "~/router/status", nullptr, "mdi:cards-heart"));
  yield();

  haDiscovery->publish(std::make_unique<Mycila::HA::Outlet>("relay1", "Relay 1", "~/router/relay1/set", "~/router/relay1", YASOLR_ON, YASOLR_OFF));
  haDiscovery->publish(std::make_unique<Mycila::HA::Outlet>("relay2", "Relay 2", "~/router/relay2/set", "~/router/relay2", YASOLR_ON, YASOLR_OFF));
  yield();

  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("output1_state", "Output 1", "~/router/output1/state"));
  haDiscovery->publish(std::make_unique<Mycila::HA::State>("output1_bypass", "Output 1 Bypass", "~/router/output1/bypass", YASOLR_ON, YASOLR_OFF, "running"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Number>("output1_dimmer_duty", "Output 1 Dimmer Duty Cycle", "~/router/output1/duty_cycle/set", "~/router/output1/duty_cycle", Mycila::HA::NumberMode::SLIDER, 0.0f, 100.0f, 0.01f, "mdi:water-boiler"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Outlet>("output1_relay", "Output 1 Bypass", "~/router/output1/bypass/set", "~/router/output1/bypass", YASOLR_ON, YASOLR_OFF));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("output1_temperature", "Output 1 Temperature", "~/router/output1/temperature", "temperature", "mdi:thermometer", "°C"));
  yield();

  haDiscovery->publish(std::make_unique<Mycila::HA::Value>("output2_state", "Output 2", "~/router/output2/state"));
  haDiscovery->publish(std::make_unique<Mycila::HA::State>("output2_bypass", "Output 2 Bypass", "~/router/output2/bypass", YASOLR_ON, YASOLR_OFF, "running"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Number>("output2_dimmer_duty", "Output 2 Dimmer Duty Cycle", "~/router/output2/duty_cycle/set", "~/router/output2/duty_cycle", Mycila::HA::NumberMode::SLIDER, 0.0f, 100.0f, 0.01f, "mdi:water-boiler"));
  haDiscovery->publish(std::make_unique<Mycila::HA::Outlet>("output2_relay", "Output 2 Bypass", "~/router/output2/bypass/set", "~/router/output2/bypass", YASOLR_ON, YASOLR_OFF));
  haDiscovery->publish(std::make_unique<Mycila::HA::Gauge>("output2_temperature", "Output 2 Temperature", "~/router/output2/temperature", "temperature", "mdi:thermometer", "°C"));
  yield();

  haDiscovery->end();
  delete haDiscovery;

  ESP_LOGI(TAG, "Published Home Assistant Discovery configuration");
}

void yasolr_configure_mqtt() {
  if (config.get<bool>(KEY_ENABLE_MQTT)) {
    if (mqtt == nullptr) {
      if (config.isEmpty(KEY_MQTT_SERVER)) {
        ESP_LOGE(TAG, "MQTT server is not set");
        return;
      }

      ESP_LOGI(TAG, "Enable MQTT");

      voltage = new Mycila::ExpiringValue<float>(YASOLR_MQTT_MEASUREMENT_EXPIRATION);
      power = new Mycila::ExpiringValue<float>(YASOLR_MQTT_MEASUREMENT_EXPIRATION);

      mqtt = new Mycila::MQTT();
      mqttConnectTask = new Mycila::Task("MQTT Connect", Mycila::Task::Type::ONCE, []() { connect(); });
      mqttPublishConfigTask = new Mycila::Task("MQTT Publish Config", Mycila::Task::Type::ONCE, []() { publishConfig(); });
      mqttPublishStaticTask = new Mycila::Task("MQTT Publish Static Data", Mycila::Task::Type::ONCE, []() { publishStaticData(); });
      mqttPublishTask = new Mycila::Task("MQTT Publish", []() { publishData(); });
      haDiscoveryTask = new Mycila::Task("HA Discovery", Mycila::Task::Type::ONCE, []() { haDiscovery(); });

      mqtt->onConnect([](void) {
        ESP_LOGI(TAG, "MQTT connected!");
        haDiscoveryTask->resume();
        mqttPublishStaticTask->resume();
        mqttPublishConfigTask->resume();
      });

      subscribe();

      haDiscoveryTask->setEnabledWhen([]() { return mqtt->isConnected() && config.get<bool>(KEY_ENABLE_HA_DISCOVERY) && !config.isEmpty(KEY_HA_DISCOVERY_TOPIC); });
      mqttPublishConfigTask->setEnabledWhen([]() { return mqtt->isConnected(); });
      mqttPublishStaticTask->setEnabledWhen([]() { return mqtt->isConnected(); });
      mqttPublishTask->setEnabledWhen([]() { return mqtt->isConnected(); });
      mqttPublishTask->setInterval(config.get<uint8_t>(KEY_MQTT_PUBLISH_INTERVAL) * 1000);

      unsafeTaskManager.addTask(*mqttConnectTask);
      unsafeTaskManager.addTask(*haDiscoveryTask);
      unsafeTaskManager.addTask(*mqttPublishConfigTask);
      unsafeTaskManager.addTask(*mqttPublishStaticTask);
      unsafeTaskManager.addTask(*mqttPublishTask);

      Mycila::TaskMonitor.addTask("mqtt_task"); // MQTT (set stack size with MYCILA_MQTT_STACK_SIZE)

      if (config.get<bool>(KEY_ENABLE_DEBUG)) {
        haDiscoveryTask->enableProfiling();
        mqttConnectTask->enableProfiling();
        mqttPublishConfigTask->enableProfiling();
        mqttPublishStaticTask->enableProfiling();
        mqttPublishTask->enableProfiling();
      }
    }
  } else {
    if (mqtt) {
      ESP_LOGI(TAG, "Disable MQTT");

      Mycila::TaskMonitor.removeTask("mqtt_task");

      unsafeTaskManager.removeTask(*mqttConnectTask);
      unsafeTaskManager.removeTask(*haDiscoveryTask);
      unsafeTaskManager.removeTask(*mqttPublishConfigTask);
      unsafeTaskManager.removeTask(*mqttPublishStaticTask);
      unsafeTaskManager.removeTask(*mqttPublishTask);

      mqtt->end();

      delete mqttConnectTask;
      delete haDiscoveryTask;
      delete mqttPublishConfigTask;
      delete mqttPublishStaticTask;
      delete mqttPublishTask;
      delete mqtt;
      delete voltage;
      delete power;

      mqttConnectTask = nullptr;
      haDiscoveryTask = nullptr;
      mqttPublishConfigTask = nullptr;
      mqttPublishStaticTask = nullptr;
      mqttPublishTask = nullptr;
      mqtt = nullptr;
      voltage = nullptr;
      power = nullptr;

      grid.deleteMetrics(Mycila::Grid::Source::MQTT);
    }
  }
}
