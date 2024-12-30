// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

#include <string>

extern const uint8_t ca_certs_bundle_start[] asm("_binary__pio_data_cacerts_bin_start");
extern const uint8_t ca_certs_bundle_end[] asm("_binary__pio_data_cacerts_bin_end");

Mycila::MQTT* mqtt;
Mycila::Task* mqttConnectTask;
Mycila::Task* mqttPublishConfigTask;
Mycila::Task* mqttPublishStaticTask;
Mycila::Task* mqttPublishTask;

Mycila::Task* haDiscoveryTask;

void connect() {
  mqtt->end();

  bool secured = config.getBool(KEY_MQTT_SECURED);

  Mycila::MQTT::Config mqttConfig;
  mqttConfig.server = config.getString(KEY_MQTT_SERVER);
  mqttConfig.port = static_cast<uint16_t>(config.getLong(KEY_MQTT_PORT));
  mqttConfig.secured = secured;
  mqttConfig.username = config.getString(KEY_MQTT_USERNAME);
  mqttConfig.password = config.getString(KEY_MQTT_PASSWORD);
  mqttConfig.clientId = Mycila::AppInfo.defaultMqttClientId;
  mqttConfig.willTopic = config.getString(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC;
  mqttConfig.keepAlive = YASOLR_MQTT_KEEPALIVE;

  if (secured) {
    // if a server certificate has been used, set it
    if (LittleFS.exists(YASOLR_MQTT_SERVER_CERT_FILE)) {
      logger.debug(TAG, "Loading MQTT PEM server certificate");
      File serverCertFile = LittleFS.open(YASOLR_MQTT_SERVER_CERT_FILE, "r");
      mqttConfig.serverCert = serverCertFile.readString().c_str();
      serverCertFile.close();
      logger.debug(TAG, "Loaded MQTT server certificate:\n%s", mqttConfig.serverCert.c_str());
    } else {
      logger.debug(TAG, "Using cacert bundle for MQTT");
      // if no server certificate has been used, set default CA certs bundle
      mqttConfig.certBundle = ca_certs_bundle_start;
      mqttConfig.certBundleSize = static_cast<size_t>(ca_certs_bundle_end - ca_certs_bundle_start);
    }
  }

  logger.info(TAG, "Connecting to MQTT broker %s://%s:%" PRIu16 "...", (secured ? "mqtts" : "mqtt"), mqttConfig.server.c_str(), mqttConfig.port);
  mqtt->setAsync(false);
  mqtt->begin(mqttConfig);
}

void subscribe() {
  logger.info(TAG, "Subscribing to MQTT topics...");

  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  // config

  mqtt->subscribe(baseTopic + "/config/+/set", [](const std::string& topic, const std::string_view& payload) {
    const std::size_t end = topic.rfind("/set");
    if (end == std::string::npos)
      return;
    const std::size_t start = topic.rfind("/", end - 1);
    const char* key = config.keyRef(topic.substr(start + 1, end - start - 1).c_str());
    if (key)
      config.set(key, std::string(payload));
  });

  // relays

  mqtt->subscribe(baseTopic + "/router/relay1/set", [](const std::string& topic, const std::string_view& payload) {
    if (relay1.isEnabled()) {
      const std::string_view state = payload.substr(0, payload.find("="));
      if (state.empty())
        return;
      if (state == YASOLR_ON)
        routerRelay1.tryRelayState(true);
      else if (state == YASOLR_OFF)
        routerRelay1.tryRelayState(false);
    }
  });

  mqtt->subscribe(baseTopic + "/router/relay2/set", [](const std::string& topic, const std::string_view& payload) {
    if (relay2.isEnabled()) {
      const std::string_view state = payload.substr(0, payload.find("="));
      if (state.empty())
        return;

      if (state == YASOLR_ON)
        routerRelay2.tryRelayState(true);
      else if (state == YASOLR_OFF)
        routerRelay2.tryRelayState(false);
    }
  });

  // router

  mqtt->subscribe(baseTopic + "/router/output1/duty_cycle/set", [](const std::string& topic, const std::string_view& payload) {
    output1.setDimmerDutyCycle(std::stof(std::string(payload)) / 100);
  });

  mqtt->subscribe(baseTopic + "/router/output2/duty_cycle/set", [](const std::string& topic, const std::string_view& payload) {
    output2.setDimmerDutyCycle(std::stof(std::string(payload)) / 100);
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
  const char* gridPowerMQTTTopic = config.get(KEY_GRID_POWER_MQTT_TOPIC);
  if (gridPowerMQTTTopic[0] != '\0') {
    logger.info(TAG, "Reading Grid Power from MQTT topic: %s", gridPowerMQTTTopic);
    mqtt->subscribe(gridPowerMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      float p = std::stof(std::string(payload));
      logger.debug(TAG, "Grid Power from MQTT: %f", p);
      grid.mqttPower().update(p);
      if (grid.updatePower()) {
        yasolr_divert();
      }
    });
  }

  // grid voltage
  const char* gridVoltageMQTTTopic = config.get(KEY_GRID_VOLTAGE_MQTT_TOPIC);
  if (gridVoltageMQTTTopic[0] != '\0') {
    logger.info(TAG, "Reading Grid Voltage from MQTT topic: %s", gridVoltageMQTTTopic);
    mqtt->subscribe(gridVoltageMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      float v = std::stof(std::string(payload));
      logger.debug(TAG, "Grid Voltage from MQTT: %f", v);
      grid.mqttVoltage().update(v);
    });
  }

  // output 1 temperature
  const char* output1TemperatureMQTTTopic = config.get(KEY_OUTPUT1_TEMPERATURE_MQTT_TOPIC);
  if (output1TemperatureMQTTTopic[0] != '\0') {
    logger.info(TAG, "Reading Output 1 Temperature from MQTT topic: %s", output1TemperatureMQTTTopic);
    mqtt->subscribe(output1TemperatureMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      float t = std::stof(std::string(payload));
      logger.debug(TAG, "Output 1 Temperature from MQTT: %f", t);
      output1.temperature().update(t);
    });
  }

  // output 2 temperature
  const char* output2TemperatureMQTTTopic = config.get(KEY_OUTPUT2_TEMPERATURE_MQTT_TOPIC);
  if (output2TemperatureMQTTTopic[0] != '\0') {
    logger.info(TAG, "Reading Output 2 Temperature from MQTT topic: %s", output2TemperatureMQTTTopic);
    mqtt->subscribe(output2TemperatureMQTTTopic, [](const std::string& topic, const std::string_view& payload) {
      float t = std::stof(std::string(payload));
      logger.debug(TAG, "Output 2 Temperature from MQTT: %f", t);
      output2.temperature().update(t);
    });
  }
}

void publishConfig() {
  logger.info(TAG, "Publishing config to MQTT...");
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  for (auto& key : config.keys()) {
    const char* value = config.get(key);
    if (value[0] != '\0' && config.isPasswordKey(key))
      value = "********";
    mqtt->publish(baseTopic + "/config/" + key, value, true);
  }
}

void publishStaticData() {
  logger.info(TAG, "Publishing static data to MQTT...");
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  mqtt->publish(baseTopic + "/system/app/manufacturer", Mycila::AppInfo.manufacturer, true);
  mqtt->publish(baseTopic + "/system/app/model", Mycila::AppInfo.model, true);
  mqtt->publish(baseTopic + "/system/app/name", Mycila::AppInfo.name, true);
  mqtt->publish(baseTopic + "/system/app/trial", YASOLR_BOOL(Mycila::AppInfo.trial), true);
  mqtt->publish(baseTopic + "/system/app/version", Mycila::AppInfo.version, true);
  yield();

  mqtt->publish(baseTopic + "/system/device/boots", std::to_string(Mycila::System::getBootCount()), true);
  mqtt->publish(baseTopic + "/system/device/cores", std::to_string(ESP.getChipCores()), true);
  mqtt->publish(baseTopic + "/system/device/cpu_freq", std::to_string(ESP.getCpuFreqMHz()), true);
  mqtt->publish(baseTopic + "/system/device/id", Mycila::AppInfo.id, true);
  mqtt->publish(baseTopic + "/system/device/model", ESP.getChipModel(), true);
  yield();

  mqtt->publish(baseTopic + "/system/firmware/build/branch", Mycila::AppInfo.buildBranch, true);
  mqtt->publish(baseTopic + "/system/firmware/build/hash", Mycila::AppInfo.buildHash, true);
  mqtt->publish(baseTopic + "/system/firmware/build/timestamp", Mycila::AppInfo.buildDate, true);
  mqtt->publish(baseTopic + "/system/firmware/debug", YASOLR_BOOL(Mycila::AppInfo.debug), true);
  mqtt->publish(baseTopic + "/system/firmware/filename", Mycila::AppInfo.firmware, true);
  yield();

  mqtt->publish(baseTopic + "/system/network/eth/mac_address", espConnect.getMACAddress(Mycila::ESPConnect::Mode::ETH), true);
  mqtt->publish(baseTopic + "/system/network/hostname", espConnect.getHostname(), true);
  mqtt->publish(baseTopic + "/system/network/wifi/mac_address", espConnect.getMACAddress(Mycila::ESPConnect::Mode::STA), true);
  yield();
}

void publishData() {
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);

  mqtt->publish(baseTopic + "/system/device/heap/total", std::to_string(memory.total));
  mqtt->publish(baseTopic + "/system/device/heap/usage", std::to_string(memory.usage));
  mqtt->publish(baseTopic + "/system/device/heap/used", std::to_string(memory.used));
  mqtt->publish(baseTopic + "/system/device/uptime", std::to_string(Mycila::System::getUptime()));
  yield();

  mqtt->publish(baseTopic + "/system/network/eth/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  mqtt->publish(baseTopic + "/system/network/ip_address", espConnect.getIPAddress().toString().c_str());
  mqtt->publish(baseTopic + "/system/network/mac_address", espConnect.getMACAddress());
  mqtt->publish(baseTopic + "/system/network/ntp", YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt->publish(baseTopic + "/system/network/wifi/bssid", espConnect.getWiFiBSSID());
  mqtt->publish(baseTopic + "/system/network/wifi/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  mqtt->publish(baseTopic + "/system/network/wifi/quality", std::to_string(espConnect.getWiFiSignalQuality()));
  mqtt->publish(baseTopic + "/system/network/wifi/rssi", std::to_string(espConnect.getWiFiRSSI()));
  mqtt->publish(baseTopic + "/system/network/wifi/ssid", espConnect.getWiFiSSID());
  yield();

  switch (espConnect.getMode()) {
    case Mycila::ESPConnect::Mode::ETH:
      mqtt->publish(baseTopic + "/system/network/mode", "eth");
      break;
    case Mycila::ESPConnect::Mode::STA:
      mqtt->publish(baseTopic + "/system/network/mode", "wifi");
      break;
    case Mycila::ESPConnect::Mode::AP:
      mqtt->publish(baseTopic + "/system/network/mode", "ap");
      break;
    default:
      mqtt->publish(baseTopic + "/system/network/mode", "");
      break;
  }

  Mycila::Grid::Metrics gridMetrics;
  grid.getGridMeasurements(gridMetrics);
  mqtt->publish(baseTopic + "/grid/apparent_power", std::to_string(gridMetrics.apparentPower));
  mqtt->publish(baseTopic + "/grid/current", std::to_string(gridMetrics.current));
  mqtt->publish(baseTopic + "/grid/energy", std::to_string(gridMetrics.energy));
  mqtt->publish(baseTopic + "/grid/energy_returned", std::to_string(gridMetrics.energyReturned));
  mqtt->publish(baseTopic + "/grid/frequency", std::to_string(gridMetrics.frequency));
  mqtt->publish(baseTopic + "/grid/online", YASOLR_BOOL(grid.isConnected()));
  mqtt->publish(baseTopic + "/grid/power", std::to_string(gridMetrics.power));
  mqtt->publish(baseTopic + "/grid/power_factor", std::to_string(gridMetrics.powerFactor));
  mqtt->publish(baseTopic + "/grid/voltage", std::to_string(gridMetrics.voltage));
  yield();

  Mycila::Router::Metrics routerMeasurements;
  router.getRouterMeasurements(routerMeasurements);
  mqtt->publish(baseTopic + "/router/apparent_power", std::to_string(routerMeasurements.apparentPower));
  mqtt->publish(baseTopic + "/router/current", std::to_string(routerMeasurements.current));
  mqtt->publish(baseTopic + "/router/energy", std::to_string(routerMeasurements.energy));
  mqtt->publish(baseTopic + "/router/lights", lights.toString());
  mqtt->publish(baseTopic + "/router/power_factor", isnan(routerMeasurements.powerFactor) ? "0" : std::to_string(routerMeasurements.powerFactor));
  mqtt->publish(baseTopic + "/router/power", std::to_string(routerMeasurements.power));
  mqtt->publish(baseTopic + "/router/relay1", YASOLR_STATE(relay1.isOn()));
  mqtt->publish(baseTopic + "/router/relay2", YASOLR_STATE(relay2.isOn()));
  if (ds18Sys)
    mqtt->publish(baseTopic + "/router/temperature", std::to_string(ds18Sys->getTemperature().value_or(0)));
  mqtt->publish(baseTopic + "/router/thdi", isnan(routerMeasurements.thdi) ? "0" : std::to_string(routerMeasurements.thdi));
  mqtt->publish(baseTopic + "/router/virtual_grid_power", std::to_string(gridMetrics.power - routerMeasurements.power));
  yield();

  for (const auto& output : router.getOutputs()) {
    const std::string outputTopic = baseTopic + "/router/" + output->getName();
    mqtt->publish(outputTopic + "/state", output->getStateName());
    mqtt->publish(outputTopic + "/bypass", YASOLR_STATE(output->isBypassOn()));
    mqtt->publish(outputTopic + "/dimmer", YASOLR_STATE(output->isDimmerOn()));
    mqtt->publish(outputTopic + "/duty_cycle", std::to_string(output->getDimmerDutyCycle() * 100));
    mqtt->publish(outputTopic + "/temperature", std::to_string(output->temperature().orElse(0)));
    yield();
  }
}

void haDiscovery() {
  logger.info(TAG, "Publishing Home Assistant Discovery configuration");

  Mycila::HA::Discovery haDiscovery;

  haDiscovery.setDiscoveryTopic(config.get(KEY_HA_DISCOVERY_TOPIC));
  haDiscovery.setWillTopic(config.getString(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC);
  haDiscovery.begin({
                      .id = Mycila::AppInfo.defaultMqttClientId,
                      .name = Mycila::AppInfo.defaultSSID,
                      .version = Mycila::AppInfo.version,
                      .model = Mycila::AppInfo.name + " " + Mycila::AppInfo.model,
                      .manufacturer = Mycila::AppInfo.manufacturer,
                      .url = std::string("http://") + espConnect.getIPAddress().toString().c_str(),
                    },
                    config.get(KEY_MQTT_TOPIC),
                    512,
                    [](const char* topic, const char* payload) { mqtt->publish(topic, payload, true); });

  // DIAGNOSTIC

  haDiscovery.publish(Mycila::HA::Button("device_restart", "Device: Restart", "/system/device/restart", "restart", nullptr, Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Counter("device_boots", "Device: Boot Count", "/system/device/boots", nullptr, nullptr, nullptr, Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Counter("device_uptime", "Device: Uptime", "/system/device/uptime", "duration", nullptr, "s", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Gauge("device_heap_usage", "Device: Heap Usage", "/system/device/heap/usage", nullptr, "mdi:memory", "%", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Gauge("device_heap_used", "Device: Heap Used", "/system/device/heap/used", "data_size", "mdi:memory", "B", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Gauge("network_wifi_quality", "Net: WiFi Signal", "/system/network/wifi/quality", nullptr, "mdi:signal", "%", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Gauge("network_wifi_rssi", "Net: WiFi RSSI", "/system/network/wifi/rssi", "signal_strength", "mdi:signal", "dBm", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("device_id", "Device: ID", "/system/device/id", nullptr, "mdi:identifier", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("firmware_filename", "Firmware", "/system/firmware/filename", nullptr, "mdi:file", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("network_eth_mac_address", "Net: Eth MAC Address", "/system/network/eth/mac_address", nullptr, "mdi:lan", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("network_hostname", "Net: Hostname", "/system/network/hostname", nullptr, "mdi:lan", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("network_ip_address", "Net: IP Address", "/system/network/ip_address", nullptr, "mdi:ip", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("network_wifi_mac_address", "Net: WiFi MAC Address", "/system/network/wifi/mac_address", nullptr, "mdi:lan", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::Value("network_wifi_ssid", "Net: WiFi SSID", "/system/network/wifi/ssid", nullptr, "mdi:wifi", Mycila::HA::Category::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HA::State("network_ntp", "Net: NTP", "/system/network/ntp", YASOLR_ON, YASOLR_OFF, "connectivity", nullptr, Mycila::HA::Category::DIAGNOSTIC));

  // CONFIG

  haDiscovery.publish(Mycila::HA::Number("output1_dimmer_limiter", "Output 1 Limiter", "/config/" KEY_OUTPUT1_DIMMER_LIMIT "/set", "/config/" KEY_OUTPUT1_DIMMER_LIMIT, Mycila::HA::NumberMode::SLIDER, 0, 100, 1, "mdi:flash", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Switch("output1_auto_bypass", "Output 1 Auto Bypass", "/config/" KEY_ENABLE_OUTPUT1_AUTO_BYPASS "/set", "/config/" KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Switch("output1_auto_dimmer", "Output 1 Auto Dimmer", "/config/" KEY_ENABLE_OUTPUT1_AUTO_DIMMER "/set", "/config/" KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output1_wdays", "Output 1 Week Days", "/config/" KEY_OUTPUT1_DAYS "/set", "/config/" KEY_OUTPUT1_DAYS, nullptr, "mdi:calendar", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output1_temperature_start", "Output 1 Temperature Start", "/config/" KEY_OUTPUT1_TEMPERATURE_START "/set", "/config/" KEY_OUTPUT1_TEMPERATURE_START, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output1_temperature_stop", "Output 1 Temperature Stop", "/config/" KEY_OUTPUT1_TEMPERATURE_STOP "/set", "/config/" KEY_OUTPUT1_TEMPERATURE_STOP, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output1_time_start", "Output 1 Time Start", "/config/" KEY_OUTPUT1_TIME_START "/set", "/config/" KEY_OUTPUT1_TIME_START, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output1_time_stop", "Output 1 Time Stop", "/config/" KEY_OUTPUT1_TIME_STOP "/set", "/config/" KEY_OUTPUT1_TIME_STOP, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HA::Category::CONFIG));

  haDiscovery.publish(Mycila::HA::Number("output2_dimmer_limiter", "Output 2 Limiter", "/config/" KEY_OUTPUT2_DIMMER_LIMIT "/set", "/config/" KEY_OUTPUT2_DIMMER_LIMIT, Mycila::HA::NumberMode::SLIDER, 0, 100, 1, "mdi:flash", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Switch("output2_auto_bypass", "Output 2 Auto Bypass", "/config/" KEY_ENABLE_OUTPUT2_AUTO_BYPASS "/set", "/config/" KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Switch("output2_auto_dimmer", "Output 2 Auto Dimmer", "/config/" KEY_ENABLE_OUTPUT2_AUTO_DIMMER "/set", "/config/" KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output2_wdays", "Output 2 Week Days", "/config/" KEY_OUTPUT2_DAYS "/set", "/config/" KEY_OUTPUT2_DAYS, nullptr, "mdi:calendar", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output2_temperature_start", "Output 2 Temperature Start", "/config/" KEY_OUTPUT2_TEMPERATURE_START "/set", "/config/" KEY_OUTPUT2_TEMPERATURE_START, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output2_temperature_stop", "Output 2 Temperature Stop", "/config/" KEY_OUTPUT2_TEMPERATURE_STOP "/set", "/config/" KEY_OUTPUT2_TEMPERATURE_STOP, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output2_time_start", "Output 2 Time Start", "/config/" KEY_OUTPUT2_TIME_START "/set", "/config/" KEY_OUTPUT2_TIME_START, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HA::Category::CONFIG));
  haDiscovery.publish(Mycila::HA::Text("output2_time_stop", "Output 2 Time Stop", "/config/" KEY_OUTPUT2_TIME_STOP "/set", "/config/" KEY_OUTPUT2_TIME_STOP, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HA::Category::CONFIG));

  // SENSORS

  haDiscovery.publish(Mycila::HA::State("grid", "Grid Electricity", "/grid/online", YASOLR_TRUE, YASOLR_FALSE, "power"));
  haDiscovery.publish(Mycila::HA::Counter("grid_energy", "Grid Energy", "/grid/energy", "energy", nullptr, "kWh"));
  haDiscovery.publish(Mycila::HA::Counter("grid_energy_returned", "Grid Energy Returned", "/grid/energy_returned", "energy", nullptr, "kWh"));
  haDiscovery.publish(Mycila::HA::Counter("grid_frequency", "Grid Frequency", "/grid/frequency", "frequency", nullptr, "Hz"));
  haDiscovery.publish(Mycila::HA::Gauge("grid_power", "Grid Power", "/grid/power", "power", nullptr, "W"));
  haDiscovery.publish(Mycila::HA::Gauge("grid_power_virtual", "Grid Power Without Routing", "/router/virtual_grid_power", "power", nullptr, "W"));
  haDiscovery.publish(Mycila::HA::Gauge("grid_power_factor", "Grid Power Factor", "/grid/power_factor", "power_factor"));
  haDiscovery.publish(Mycila::HA::Gauge("grid_voltage", "Grid Voltage", "/grid/voltage", "voltage", nullptr, "V"));

  haDiscovery.publish(Mycila::HA::Counter("routed_energy", "Routed Energy", "/router/energy", "energy", nullptr, "kWh"));
  haDiscovery.publish(Mycila::HA::Gauge("routed_power", "Routed Power", "/router/power", "power", nullptr, "W"));
  haDiscovery.publish(Mycila::HA::Gauge("router_power_factor", "Router Power Factor", "/router/power_factor", "power_factor"));
  haDiscovery.publish(Mycila::HA::Gauge("router_temperature", "Router Temperature", "/router/temperature", "temperature", "mdi:thermometer", "°C"));
  haDiscovery.publish(Mycila::HA::Value("router_lights", "Router Lights", "/router/lights", nullptr, "mdi:cards-heart"));

  haDiscovery.publish(Mycila::HA::Outlet("relay1", "Relay 1", "/router/relay1/set", "/router/relay1", YASOLR_ON, YASOLR_OFF));

  haDiscovery.publish(Mycila::HA::Outlet("relay2", "Relay 2", "/router/relay2/set", "/router/relay2", YASOLR_ON, YASOLR_OFF));

  haDiscovery.publish(Mycila::HA::Value("output1_state", "Output 1", "/router/output1/state"));
  haDiscovery.publish(Mycila::HA::State("output1_bypass", "Output 1 Bypass", "/router/output1/bypass", YASOLR_ON, YASOLR_OFF, "running"));
  haDiscovery.publish(Mycila::HA::Number("output1_dimmer_duty", "Output 1 Dimmer Duty Cycle", "/router/output1/duty_cycle/set", "/router/output1/duty_cycle", Mycila::HA::NumberMode::SLIDER, 0.0f, 100.0f, 0.01f, "mdi:water-boiler"));
  haDiscovery.publish(Mycila::HA::Outlet("output1_relay", "Output 1 Bypass", "/router/output1/bypass/set", "/router/output1/bypass", YASOLR_ON, YASOLR_OFF));
  haDiscovery.publish(Mycila::HA::Gauge("output1_temperature", "Output 1 Temperature", "/router/output1/temperature", "temperature", "mdi:thermometer", "°C"));

  haDiscovery.publish(Mycila::HA::Value("output2_state", "Output 2", "/router/output2/state"));
  haDiscovery.publish(Mycila::HA::State("output2_bypass", "Output 2 Bypass", "/router/output2/bypass", YASOLR_ON, YASOLR_OFF, "running"));
  haDiscovery.publish(Mycila::HA::Number("output2_dimmer_duty", "Output 2 Dimmer Duty Cycle", "/router/output2/duty_cycle/set", "/router/output2/duty_cycle", Mycila::HA::NumberMode::SLIDER, 0.0f, 100.0f, 0.01f, "mdi:water-boiler"));
  haDiscovery.publish(Mycila::HA::Outlet("output2_relay", "Output 2 Bypass", "/router/output2/bypass/set", "/router/output2/bypass", YASOLR_ON, YASOLR_OFF));
  haDiscovery.publish(Mycila::HA::Gauge("output2_temperature", "Output 2 Temperature", "/router/output2/temperature", "temperature", "mdi:thermometer", "°C"));

  haDiscovery.end();
}

void yasolr_init_mqtt() {
  if (config.getBool(KEY_ENABLE_MQTT)) {
    assert(!mqtt);
    assert(!mqttConnectTask);
    assert(!mqttPublishConfigTask);
    assert(!mqttPublishStaticTask);
    assert(!mqttPublishTask);

    logger.info(TAG, "Initialize MQTT...");

    mqtt = new Mycila::MQTT();
    mqttConnectTask = new Mycila::Task("MQTT Connect", Mycila::TaskType::ONCE, [](void* params) { connect(); });
    mqttPublishConfigTask = new Mycila::Task("MQTT Publish Config", Mycila::TaskType::ONCE, [](void* params) { publishConfig(); });
    mqttPublishStaticTask = new Mycila::Task("MQTT Publish Static Data", Mycila::TaskType::ONCE, [](void* params) { publishStaticData(); });
    mqttPublishTask = new Mycila::Task("MQTT Publish", [](void* params) { publishData(); });
    haDiscoveryTask = new Mycila::Task("HA Discovery", Mycila::TaskType::ONCE, [](void* params) { haDiscovery(); });

    mqtt->onConnect([](void) {
      logger.info(TAG, "MQTT connected!");
      haDiscoveryTask->resume();
      mqttPublishStaticTask->resume();
      mqttPublishConfigTask->resume();
    });

    mqttConnectTask->setManager(unsafeTaskManager);

    haDiscoveryTask->setManager(unsafeTaskManager);
    haDiscoveryTask->setEnabledWhen([]() { return mqtt->isConnected() && config.getBool(KEY_ENABLE_HA_DISCOVERY) && !config.isEmpty(KEY_HA_DISCOVERY_TOPIC); });

    mqttPublishConfigTask->setEnabledWhen([]() { return mqtt->isConnected(); });
    mqttPublishConfigTask->setManager(unsafeTaskManager);

    mqttPublishStaticTask->setEnabledWhen([]() { return mqtt->isConnected(); });
    mqttPublishStaticTask->setManager(unsafeTaskManager);

    mqttPublishTask->setEnabledWhen([]() { return mqtt->isConnected(); });
    mqttPublishTask->setIntervalSupplier([]() { return config.getLong(KEY_MQTT_PUBLISH_INTERVAL) * Mycila::TaskDuration::SECONDS; });
    mqttPublishTask->setManager(unsafeTaskManager);

    Mycila::TaskMonitor.addTask("mqtt_task"); // MQTT (set stack size with MYCILA_MQTT_STACK_SIZE)

    if (config.getBool(KEY_ENABLE_DEBUG)) {
      haDiscoveryTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
      mqttConnectTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
      mqttPublishConfigTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
      mqttPublishStaticTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
      mqttPublishTask->enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
    }
  }
}
