// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "MQTT"

void YaSolR::YaSolRClass::publishMQTT() {
  if (Mycila::MQTT.isConnected()) {
    const String baseTopic = Mycila::Config.get(KEY_MQTT_TOPIC);

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
      ESPConnectMode mode = ESPConnect.getMode();
      Mycila::MQTT.publish(baseTopic + "/network/ip_address", ESPConnect.getIPAddress().toString());
      Mycila::MQTT.publish(baseTopic + "/network/mac_address", ESPConnect.getMACAddress());
      Mycila::MQTT.publish(baseTopic + "/network/mode", mode == ESPConnectMode::AP ? "AP" : (mode == ESPConnectMode::STA ? "STA" : (mode == ESPConnectMode::ETH ? "ETH" : "NONE")));
      Mycila::MQTT.publish(baseTopic + "/network/state", ESPConnect.getStateName());
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
      const std::vector<Mycila::Relay> relays = {relay1, relay2};
      for (auto& relay : relays) {
        const String base = baseTopic + "/relays/" + relay.getName();
        Mycila::MQTT.publish(base + "/enabled", YASOLR_BOOL(relay.isEnabled()));
        Mycila::MQTT.publish(base + "/state", YASOLR_STATE(relay.isOn()));
        Mycila::MQTT.publish(base + "/switch_count", String(relay.getSwitchCount()));
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
      const std::vector<Mycila::RouterOutput> outputs = {output1, output2};
      for (auto& output : outputs) {
        const String base = baseTopic + "/router/" + output.getName();

        Mycila::MQTT.publish(base + "/enabled", YASOLR_BOOL(output.isEnabled()));
        Mycila::MQTT.publish(base + "/state", output.getStateString());

        Mycila::MQTT.publish(base + "/dimmer/enabled", YASOLR_BOOL(output.getDimmer()->isEnabled()));
        Mycila::MQTT.publish(base + "/dimmer/level", String(output.getDimmer()->getLevel()));
        Mycila::MQTT.publish(base + "/dimmer/state", YASOLR_STATE(output.getDimmer()->isOn()));

        Mycila::MQTT.publish(base + "/temp_sensor/enabled", YASOLR_BOOL(output.getTemperatureSensor()->isEnabled()));
        Mycila::MQTT.publish(base + "/temp_sensor/temperature", String(output.getTemperatureSensor()->getTemperature()));
        Mycila::MQTT.publish(base + "/temp_sensor/valid", YASOLR_BOOL(output.getTemperatureSensor()->isValid()));

        Mycila::MQTT.publish(base + "/bypass_relay/enabled", YASOLR_BOOL(output.getBypassRelay()->isEnabled()));
        Mycila::MQTT.publish(base + "/bypass_relay/state", YASOLR_STATE(output.getBypassRelay()->isOn()));
        Mycila::MQTT.publish(base + "/bypass_relay/switch_count", String(output.getBypassRelay()->getSwitchCount()));
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
      Mycila::MQTT.publish(baseTopic + "/system/temp_sensor/enabled", YASOLR_BOOL(systemTemperatureSensor.isEnabled()));
      Mycila::MQTT.publish(baseTopic + "/system/temp_sensor/temperature", String(systemTemperatureSensor.getTemperature()));
      Mycila::MQTT.publish(baseTopic + "/system/temp_sensor/valid", YASOLR_BOOL(systemTemperatureSensor.isValid()));
    }
    yield();
  }
}
