// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task haDiscoveryTask("HADiscovery", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Publishing Home Assistant Discovery configuration");

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
});
