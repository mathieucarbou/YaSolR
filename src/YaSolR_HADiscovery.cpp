// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "HA-DISCO"

void YaSolR::YaSolRClass::publishHADiscovery() {
  if (!Mycila::Config.getBool(KEY_HA_DISCOVERY_ENABLE))
    return;

  if (Mycila::Config.getBool(KEY_AP_MODE_ENABLE))
    return;

  if (!Mycila::MQTT.isConnected())
    return;

  if (Mycila::Config.get(KEY_HA_DISCOVERY_TOPIC).isEmpty())
    return;

  // GLOBAL SETTINGS

  Mycila::HADiscovery.setDiscoveryTopic(Mycila::Config.get(KEY_HA_DISCOVERY_TOPIC));
  Mycila::HADiscovery.setSensorExpirationTime(4 * Mycila::Config.get(KEY_MQTT_PUBLISH_INTERVAL).toInt());

  Mycila::HADiscovery.setBaseTopic(Mycila::Config.get(KEY_MQTT_TOPIC));
  Mycila::HADiscovery.setWillTopic(Mycila::Config.get(KEY_MQTT_TOPIC) + YASOLR_MQTT_WILL_TOPIC);

  Mycila::HADiscovery.setPublisher([](const String& topic, const String& payload) {
    Mycila::MQTT.publish(topic, payload, true);
  });

  String appNameLow = Mycila::AppInfo.name;
  appNameLow.toLowerCase();

  Mycila::HADiscovery.setDevice({
    .id = appNameLow + "_" + Mycila::AppInfo.id,
    .name = Mycila::AppInfo.name + "-" + Mycila::AppInfo.id,
    .version = Mycila::AppInfo.version,
    .model = Mycila::AppInfo.name + " " + Mycila::AppInfo.model,
    .manufacturer = Mycila::AppInfo.manufacturer,
    .url = "http://" + ESPConnect.getIPAddress().toString(),
  });

  // DIAGNOSTIC

  Mycila::HADiscovery.publish(Mycila::HAButton("restart", "Restart", "/system/restart", "restart", nullptr, Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HACounter("boots", "Boot Count", "/system/boots", nullptr, nullptr, nullptr, Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HACounter("uptime", "Uptime", "/system/uptime", "duration", nullptr, "s", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAGauge("cpu_cores", "CPU Cores", "/system/chip_cores", nullptr, "mdi:chip", nullptr, Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAGauge("memory_total", "Memory Total", "/system/heap_total", "data_size", "mdi:memory", "B", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAGauge("memory_usage", "Memory Usage", "/system/heap_usage", nullptr, "mdi:memory", "%", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAGauge("memory_used", "Memory Used", "/system/heap_used", "data_size", "mdi:memory", "B", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAGauge("wifi_quality", "WiFi Signal", "/network/wifi_quality", nullptr, "mdi:signal", "%", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAGauge("wifi_rssi", "WiFi RSSI", "/network/wifi_rssi", "signal_strength", "mdi:signal", "dBm", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAState("ntp", "NTP", "/ntp/synced", YASOLR_TRUE, YASOLR_FALSE, "connectivity", nullptr, Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("cpu_model", "CPU Model", "/system/chip_model", nullptr, "mdi:car-esp", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("firmware", "Firmware", "/app/firmware", nullptr, "mdi:file", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("hostname", "Hostname", "/config/" KEY_HOSTNAME, nullptr, "mdi:lan", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("id", "ID", "/app/id", nullptr, "mdi:identifier", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("mac_address", "MAC Address", "/network/mac_address", nullptr, "mdi:lan", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("system_status", "System Status", "/system/lights/code", nullptr, "mdi:cards-heart", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("wifi_ip_address", "WiFi IP Address", "/network/ip_address", nullptr, "mdi:ip", Mycila::HACategory::DIAGNOSTIC));
  Mycila::HADiscovery.publish(Mycila::HAText("wifi_ssid", "WiFi SSID", "/network/wifi_ssid", nullptr, "mdi:wifi", Mycila::HACategory::DIAGNOSTIC));

  Mycila::HAGauge haTempSys("system_temperature", "System Temperature", "/system/temp_sensor/temperature", "temperature", "mdi:thermometer", "°C", Mycila::HACategory::DIAGNOSTIC);
  haTempSys.availabilityTopic = "/system/temp_sensor/valid";
  haTempSys.payloadAvailable = YASOLR_TRUE;
  haTempSys.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(haTempSys);

  // CONFIG

  Mycila::HADiscovery.publish(Mycila::HATextField("mqtt_publish_interval", "MQTT Publish Interval", "/config/" KEY_MQTT_PUBLISH_INTERVAL "/set", "/config/" KEY_MQTT_PUBLISH_INTERVAL, "^\\d+$", "mdi:timer-sand", Mycila::HACategory::CONFIG));

  Mycila::HADiscovery.publish(Mycila::HATextField("display_power_save", "Display Power Save", "/config/" KEY_DISPLAY_POWER_SAVE_DELAY "/set", "/config/" KEY_DISPLAY_POWER_SAVE_DELAY, "^\\d+$", "mdi:monitor-off", Mycila::HACategory::CONFIG));

  Mycila::HADiscovery.publish(Mycila::HANumber("relay1_power_threshold", "Relay 1 Power Threshold", "/config/" KEY_RELAY1_POWER "/set", "/config/" KEY_RELAY1_POWER, Mycila::HANumberMode::SLIDER, 0, 3000, 50, "mdi:flash", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HANumber("relay2_power_threshold", "Relay 2 Power Threshold", "/config/" KEY_RELAY2_POWER "/set", "/config/" KEY_RELAY2_POWER, Mycila::HANumberMode::SLIDER, 0, 3000, 50, "mdi:flash", Mycila::HACategory::CONFIG));

  Mycila::HADiscovery.publish(Mycila::HANumber("output1_dimmer_level_limit", "Output 1 Dimmer Level Limit", "/config/" KEY_OUTPUT1_DIMMER_LEVEL_LIMIT "/set", "/config/" KEY_OUTPUT1_DIMMER_LEVEL_LIMIT, Mycila::HANumberMode::SLIDER, 0, MYCILA_DIMMER_MAX_LEVEL, 1, "mdi:flash", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HASwitch("output1_auto_bypass_enable", "Output 1 Auto Bypass", "/config/" KEY_OUTPUT1_AUTO_BYPASS_ENABLE "/set", "/config/" KEY_OUTPUT1_AUTO_BYPASS_ENABLE, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HASwitch("output1_auto_dimmer_enable", "Output 1 Auto Dimmer", "/config/" KEY_OUTPUT1_DIMMER_AUTO "/set", "/config/" KEY_OUTPUT1_DIMMER_AUTO, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output1_auto_start_temperature", "Output 1 Auto Start Temperature", "/config/" KEY_OUTPUT1_AUTO_START_TEMPERATURE "/set", "/config/" KEY_OUTPUT1_AUTO_START_TEMPERATURE, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output1_auto_stop_temperature", "Output 1 Auto Stop Temperature", "/config/" KEY_OUTPUT1_AUTO_STOP_TEMPERATURE "/set", "/config/" KEY_OUTPUT1_AUTO_STOP_TEMPERATURE, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output1_auto_start_time", "Output 1 Auto Start Time", "/config/" KEY_OUTPUT1_AUTO_START_TIME "/set", "/config/" KEY_OUTPUT1_AUTO_START_TIME, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output1_auto_stop_time", "Output 1 Auto Stop Time", "/config/" KEY_OUTPUT1_AUTO_STOP_TIME "/set", "/config/" KEY_OUTPUT1_AUTO_STOP_TIME, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output1_auto_wdays", "Output 1 Auto Week Days", "/config/" KEY_OUTPUT1_AUTO_WEEK_DAYS "/set", "/config/" KEY_OUTPUT1_AUTO_WEEK_DAYS, nullptr, "mdi:calendar", Mycila::HACategory::CONFIG));

  Mycila::HADiscovery.publish(Mycila::HANumber("output2_dimmer_level_limit", "Output 2 Dimmer Level Limit", "/config/" KEY_OUTPUT2_DIMMER_LEVEL_LIMIT "/set", "/config/" KEY_OUTPUT2_DIMMER_LEVEL_LIMIT, Mycila::HANumberMode::SLIDER, 0, MYCILA_DIMMER_MAX_LEVEL, 1, "mdi:flash", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HASwitch("output2_auto_bypass_enable", "Output 2 Auto Bypass", "/config/" KEY_OUTPUT2_AUTO_BYPASS_ENABLE "/set", "/config/" KEY_OUTPUT2_AUTO_BYPASS_ENABLE, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HASwitch("output2_auto_dimmer_enable", "Output 2 Auto Dimmer", "/config/" KEY_OUTPUT2_DIMMER_AUTO "/set", "/config/" KEY_OUTPUT2_DIMMER_AUTO, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output2_auto_start_temperature", "Output 2 Auto Start Temperature", "/config/" KEY_OUTPUT2_AUTO_START_TEMPERATURE "/set", "/config/" KEY_OUTPUT2_AUTO_START_TEMPERATURE, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output2_auto_stop_temperature", "Output 2 Auto Stop Temperature", "/config/" KEY_OUTPUT2_AUTO_STOP_TEMPERATURE "/set", "/config/" KEY_OUTPUT2_AUTO_STOP_TEMPERATURE, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output2_auto_start_time", "Output 2 Auto Start Time", "/config/" KEY_OUTPUT2_AUTO_START_TIME "/set", "/config/" KEY_OUTPUT2_AUTO_START_TIME, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output2_auto_stop_time", "Output 2 Auto Stop Time", "/config/" KEY_OUTPUT2_AUTO_STOP_TIME "/set", "/config/" KEY_OUTPUT2_AUTO_STOP_TIME, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HACategory::CONFIG));
  Mycila::HADiscovery.publish(Mycila::HATextField("output2_auto_wdays", "Output 2 Auto Week Days", "/config/" KEY_OUTPUT2_AUTO_WEEK_DAYS "/set", "/config/" KEY_OUTPUT2_AUTO_WEEK_DAYS, nullptr, "mdi:calendar", Mycila::HACategory::CONFIG));

  // SENSORS

  Mycila::HADiscovery.publish(Mycila::HAState("grid", "Grid Electricity", "/grid/online", YASOLR_TRUE, YASOLR_FALSE, "connectivity"));
  Mycila::HADiscovery.publish(Mycila::HACounter("grid_energy", "Grid Energy", "/grid/energy", "energy", nullptr, "kWh"));
  Mycila::HADiscovery.publish(Mycila::HACounter("grid_energy_returned", "Grid Energy Returned", "/grid/energy_returned", "energy", nullptr, "kWh"));
  Mycila::HADiscovery.publish(Mycila::HAGauge("grid_power", "Grid Power", "/grid/power", "power", nullptr, "W"));
  Mycila::HADiscovery.publish(Mycila::HAGauge("grid_power_factor", "Grid Power Factor", "/grid/power_factor", "power_factor"));
  Mycila::HADiscovery.publish(Mycila::HAGauge("grid_voltage", "Grid Voltage", "/grid/voltage", "voltage"));

  Mycila::HADiscovery.publish(Mycila::HACounter("router_energy", "Routed Energy", "/router/energy", "energy", nullptr, "kWh"));
  Mycila::HADiscovery.publish(Mycila::HAGauge("router_power", "Total Routed Power", "/router/power", "power", nullptr, "W"));
  Mycila::HADiscovery.publish(Mycila::HAGauge("router_power_factor", "Router Power Factor", "/router/power_factor", "power_factor"));
  Mycila::HADiscovery.publish(Mycila::HAGauge("router_thdi", "Router THDi", "/router/thdi"));

  Mycila::HADiscovery.publish(Mycila::HACounter(NAME_RELAY1 "_switch_count", "Relay 1 Switch Count", "/relays/" NAME_RELAY1 "/switch_count", nullptr, "mdi:counter", nullptr));
  Mycila::HADiscovery.publish(Mycila::HACounter(NAME_RELAY2 "_switch_count", "Relay 2 Switch Count", "/relays/" NAME_RELAY2 "/switch_count", nullptr, "mdi:counter", nullptr));

  Mycila::HAOutlet relay1Commute(NAME_RELAY1 "_commute", "Relay 1", "/relays/" NAME_RELAY1 "/state/set", "/relays/" NAME_RELAY1 "/state", YASOLR_ON, YASOLR_OFF);
  relay1Commute.availabilityTopic = "/relays/" NAME_RELAY1 "/enabled";
  relay1Commute.payloadAvailable = YASOLR_TRUE;
  relay1Commute.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(relay1Commute);

  Mycila::HAOutlet relay2Commute(NAME_RELAY2 "_commute", "Relay 2", "/relays/" NAME_RELAY2 "/state/set", "/relays/" NAME_RELAY2 "/state", YASOLR_ON, YASOLR_OFF);
  relay2Commute.availabilityTopic = "/relays/" NAME_RELAY2 "/enabled";
  relay2Commute.payloadAvailable = YASOLR_TRUE;
  relay2Commute.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(relay2Commute);

  Mycila::HANumber haDimmer1(NAME_OUTPUT1 "_dimmer_level", "Output 1 Dimmer Level", "/router/" NAME_OUTPUT1 "/dimmer/level/set", "/router/" NAME_OUTPUT1 "/dimmer/level", Mycila::HANumberMode::SLIDER, 0, MYCILA_DIMMER_MAX_LEVEL, 1, "mdi:water-boiler");
  haDimmer1.availabilityTopic = "/router/" NAME_OUTPUT1 "/dimmer/enabled";
  haDimmer1.payloadAvailable = YASOLR_TRUE;
  haDimmer1.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(haDimmer1);

  Mycila::HADiscovery.publish(Mycila::HACounter(NAME_OUTPUT1 "_relay_switch_count", "Output 1 Bypass Switch Count", "/router/" NAME_OUTPUT1 "/bypass_relay/switch_count", nullptr, "mdi:counter", nullptr));
  Mycila::HAOutlet output1RelayCommute(NAME_OUTPUT1 "_relay_commute", "Output 1 Bypass Relay", "/router/" NAME_OUTPUT1 "/bypass_relay/state/set", "/router/" NAME_OUTPUT1 "/bypass_relay/state", YASOLR_ON, YASOLR_OFF);
  output1RelayCommute.availabilityTopic = "/router/" NAME_OUTPUT1 "/bypass_relay/enabled";
  output1RelayCommute.payloadAvailable = YASOLR_TRUE;
  output1RelayCommute.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(output1RelayCommute);

  Mycila::HAGauge haTempExt1(NAME_OUTPUT1 "_temperature", "Output 1 Temperature", "/router/" NAME_OUTPUT1 "/temp_sensor/temperature", "temperature", "mdi:thermometer", "°C");
  haTempExt1.availabilityTopic = "/router/" NAME_OUTPUT1 "/temp_sensor/valid";
  haTempExt1.payloadAvailable = YASOLR_TRUE;
  haTempExt1.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(haTempExt1);

  Mycila::HANumber haDimmer2(NAME_OUTPUT2 "_dimmer_level", "Output 2 Dimmer Level", "/router/" NAME_OUTPUT2 "/dimmer/level/set", "/router/" NAME_OUTPUT2 "/dimmer/level", Mycila::HANumberMode::SLIDER, 0, MYCILA_DIMMER_MAX_LEVEL, 1, "mdi:water-boiler");
  haDimmer2.availabilityTopic = "/router/" NAME_OUTPUT2 "/dimmer/enabled";
  haDimmer2.payloadAvailable = YASOLR_TRUE;
  haDimmer2.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(haDimmer2);

  Mycila::HADiscovery.publish(Mycila::HACounter(NAME_OUTPUT2 "_relay_switch_count", "Output 2 Bypass Switch Count", "/router/" NAME_OUTPUT2 "/bypass_relay/switch_count", nullptr, "mdi:counter", nullptr));
  Mycila::HAOutlet output2RelayCommute(NAME_OUTPUT2 "_relay_commute", "Output 2 Bypass Relay", "/router/" NAME_OUTPUT2 "/bypass_relay/state/set", "/router/" NAME_OUTPUT2 "/bypass_relay/state", YASOLR_ON, YASOLR_OFF);
  output2RelayCommute.availabilityTopic = "/router/" NAME_OUTPUT2 "/bypass_relay/enabled";
  output2RelayCommute.payloadAvailable = YASOLR_TRUE;
  output2RelayCommute.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(output2RelayCommute);

  Mycila::HAGauge haTempExt2(NAME_OUTPUT2 "_temperature", "Output 2 Temperature", "/router/" NAME_OUTPUT2 "/temp_sensor/temperature", "temperature", "mdi:thermometer", "°C");
  haTempExt2.availabilityTopic = "/router/" NAME_OUTPUT2 "/temp_sensor/valid";
  haTempExt2.payloadAvailable = YASOLR_TRUE;
  haTempExt2.payloadNotAvailable = YASOLR_FALSE;
  Mycila::HADiscovery.publish(haTempExt2);
}
