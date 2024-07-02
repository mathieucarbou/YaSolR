// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task haDiscoveryTask("HADiscovery", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Publishing Home Assistant Discovery configuration...");
  haDiscovery.begin();

  // DIAGNOSTIC

  haDiscovery.publish(Mycila::HAButton("device_restart", "Device: Restart", "/system/device/restart", "restart", nullptr, Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HACounter("device_boots", "Device: Boot Count", "/system/device/boots", nullptr, nullptr, nullptr, Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HACounter("device_uptime", "Device: Uptime", "/system/device/uptime", "duration", nullptr, "s", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAGauge("device_heap_usage", "Device: Heap Usage", "/system/device/heap/usage", nullptr, "mdi:memory", "%", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAGauge("device_heap_used", "Device: Heap Used", "/system/device/heap/used", "data_size", "mdi:memory", "B", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAGauge("network_wifi_quality", "Net: WiFi Signal", "/system/network/wifi/quality", nullptr, "mdi:signal", "%", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAGauge("network_wifi_rssi", "Net: WiFi RSSI", "/system/network/wifi/rssi", "signal_strength", "mdi:signal", "dBm", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("device_id", "Device: ID", "/system/device/id", nullptr, "mdi:identifier", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("firmware_filename", "Firmware", "/system/firmware/filename", nullptr, "mdi:file", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("network_eth_mac_address", "Net: Eth MAC Address", "/system/network/eth/mac_address", nullptr, "mdi:lan", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("network_hostname", "Net: Hostname", "/system/network/hostname", nullptr, "mdi:lan", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("network_ip_address", "Net: IP Address", "/system/network/ip_address", nullptr, "mdi:ip", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("network_wifi_mac_address", "Net: WiFi MAC Address", "/system/network/wifi/mac_address", nullptr, "mdi:lan", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAText("network_wifi_ssid", "Net: WiFi SSID", "/system/network/wifi/ssid", nullptr, "mdi:wifi", Mycila::HACategory::DIAGNOSTIC));
  haDiscovery.publish(Mycila::HAState("network_ntp", "Net: NTP", "/system/network/ntp", YASOLR_ON, YASOLR_OFF, "connectivity", nullptr, Mycila::HACategory::DIAGNOSTIC));

  // CONFIG

  haDiscovery.publish(Mycila::HANumber("output1_dimmer_limiter", "Output 1 Limiter", "/config/" KEY_OUTPUT1_DIMMER_MAX_DUTY "/set", "/config/" KEY_OUTPUT1_DIMMER_MAX_DUTY, Mycila::HANumberMode::SLIDER, 0, YASOLR_DIMMER_MAX_LEVEL, 1, "mdi:flash", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HASwitch("output1_auto_bypass", "Output 1 Auto Bypass", "/config/" KEY_ENABLE_OUTPUT1_AUTO_BYPASS "/set", "/config/" KEY_ENABLE_OUTPUT1_AUTO_BYPASS, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HASwitch("output1_auto_dimmer", "Output 1 Auto Dimmer", "/config/" KEY_ENABLE_OUTPUT1_AUTO_DIMMER "/set", "/config/" KEY_ENABLE_OUTPUT1_AUTO_DIMMER, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output1_wdays", "Output 1 Week Days", "/config/" KEY_OUTPUT1_DAYS "/set", "/config/" KEY_OUTPUT1_DAYS, nullptr, "mdi:calendar", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output1_temperature_start", "Output 1 Temperature Start", "/config/" KEY_OUTPUT1_TEMPERATURE_START "/set", "/config/" KEY_OUTPUT1_TEMPERATURE_START, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output1_temperature_stop", "Output 1 Temperature Stop", "/config/" KEY_OUTPUT1_TEMPERATURE_STOP "/set", "/config/" KEY_OUTPUT1_TEMPERATURE_STOP, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output1_time_start", "Output 1 Time Start", "/config/" KEY_OUTPUT1_TIME_START "/set", "/config/" KEY_OUTPUT1_TIME_START, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output1_time_stop", "Output 1 Time Stop", "/config/" KEY_OUTPUT1_TIME_STOP "/set", "/config/" KEY_OUTPUT1_TIME_STOP, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HACategory::CONFIG));

  haDiscovery.publish(Mycila::HANumber("output2_dimmer_limiter", "Output 2 Limiter", "/config/" KEY_OUTPUT2_DIMMER_MAX_DUTY "/set", "/config/" KEY_OUTPUT2_DIMMER_MAX_DUTY, Mycila::HANumberMode::SLIDER, 0, YASOLR_DIMMER_MAX_LEVEL, 1, "mdi:flash", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HASwitch("output2_auto_bypass", "Output 2 Auto Bypass", "/config/" KEY_ENABLE_OUTPUT2_AUTO_BYPASS "/set", "/config/" KEY_ENABLE_OUTPUT2_AUTO_BYPASS, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HASwitch("output2_auto_dimmer", "Output 2 Auto Dimmer", "/config/" KEY_ENABLE_OUTPUT2_AUTO_DIMMER "/set", "/config/" KEY_ENABLE_OUTPUT2_AUTO_DIMMER, YASOLR_TRUE, YASOLR_FALSE, "mdi:water-boiler-auto", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output2_wdays", "Output 2 Week Days", "/config/" KEY_OUTPUT2_DAYS "/set", "/config/" KEY_OUTPUT2_DAYS, nullptr, "mdi:calendar", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output2_temperature_start", "Output 2 Temperature Start", "/config/" KEY_OUTPUT2_TEMPERATURE_START "/set", "/config/" KEY_OUTPUT2_TEMPERATURE_START, "^\\d{1,3}$", "mdi:thermometer-low", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output2_temperature_stop", "Output 2 Temperature Stop", "/config/" KEY_OUTPUT2_TEMPERATURE_STOP "/set", "/config/" KEY_OUTPUT2_TEMPERATURE_STOP, "^\\d{1,3}$", "mdi:thermometer-alert", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output2_time_start", "Output 2 Time Start", "/config/" KEY_OUTPUT2_TIME_START "/set", "/config/" KEY_OUTPUT2_TIME_START, "^\\d?\\d:\\d\\d$", "mdi:clock-time-ten", Mycila::HACategory::CONFIG));
  haDiscovery.publish(Mycila::HATextField("output2_time_stop", "Output 2 Time Stop", "/config/" KEY_OUTPUT2_TIME_STOP "/set", "/config/" KEY_OUTPUT2_TIME_STOP, "^\\d?\\d:\\d\\d$", "mdi:clock-time-six", Mycila::HACategory::CONFIG));

  // SENSORS

  haDiscovery.publish(Mycila::HAState("grid", "Grid Electricity", "/grid/online", YASOLR_TRUE, YASOLR_FALSE, "power"));
  haDiscovery.publish(Mycila::HACounter("grid_energy", "Grid Energy", "/grid/energy", "energy", nullptr, "kWh"));
  haDiscovery.publish(Mycila::HACounter("grid_energy_returned", "Grid Energy Returned", "/grid/energy_returned", "energy", nullptr, "kWh"));
  haDiscovery.publish(Mycila::HACounter("grid_frequency", "Grid Frequency", "/grid/frequency", "frequency", nullptr, "Hz"));
  haDiscovery.publish(Mycila::HAGauge("grid_power", "Grid Power", "/grid/power", "power", nullptr, "W"));
  haDiscovery.publish(Mycila::HAGauge("grid_power_virtual", "Grid Power Without Routing", "/router/virtual_grid_power", "power", nullptr, "W"));
  haDiscovery.publish(Mycila::HAGauge("grid_power_factor", "Grid Power Factor", "/grid/power_factor", "power_factor"));
  haDiscovery.publish(Mycila::HAGauge("grid_voltage", "Grid Voltage", "/grid/voltage", "voltage", nullptr, "V"));

  haDiscovery.publish(Mycila::HACounter("routed_energy", "Routed Energy", "/router/energy", "energy", nullptr, "kWh"));
  haDiscovery.publish(Mycila::HAGauge("routed_power", "Routed Power", "/router/power", "power", nullptr, "W"));
  haDiscovery.publish(Mycila::HAGauge("router_power_factor", "Router Power Factor", "/router/power_factor", "power_factor"));
  haDiscovery.publish(Mycila::HAGauge("router_temperature", "Router Temperature", "/router/temperature", "temperature", "mdi:thermometer", "°C"));
  haDiscovery.publish(Mycila::HAText("router_lights", "Router Lights", "/router/lights", nullptr, "mdi:cards-heart"));

  haDiscovery.publish(Mycila::HACounter("relay1_switch_count", "Relay 1 Switch Count", "/router/relay1/switch_count", nullptr, "mdi:counter"));
  haDiscovery.publish(Mycila::HAOutlet("relay1", "Relay 1", "/router/relay1/state/set", "/router/relay1/state", YASOLR_ON, YASOLR_OFF));

  haDiscovery.publish(Mycila::HACounter("relay2_switch_count", "Relay 2 Switch Count", "/router/relay2/switch_count", nullptr, "mdi:counter"));
  haDiscovery.publish(Mycila::HAOutlet("relay2", "Relay 2", "/router/relay2/state/set", "/router/relay2/state", YASOLR_ON, YASOLR_OFF));

  haDiscovery.publish(Mycila::HAText("output1_state", "Output 1", "/router/output1/state"));
  haDiscovery.publish(Mycila::HAState("output1_bypass", "Output 1 Bypass", "/router/output1/bypass", YASOLR_ON, YASOLR_OFF, "running"));
  haDiscovery.publish(Mycila::HANumber("output1_dimmer_duty", "Output 1 Dimmer Duty", "/router/output1/dimmer/duty/set", "/router/output1/dimmer/duty", Mycila::HANumberMode::SLIDER, 0, YASOLR_DIMMER_MAX_LEVEL, 1, "mdi:water-boiler"));
  haDiscovery.publish(Mycila::HACounter("output1_relay_switch_count", "Output 1 Bypass Relay Switch Count", "/router/output1/relay/switch_count", nullptr, "mdi:counter"));
  haDiscovery.publish(Mycila::HAOutlet("output1_relay", "Output 1 Bypass", "/router/output1/bypass/set", "/router/output1/bypass", YASOLR_ON, YASOLR_OFF));
  haDiscovery.publish(Mycila::HAGauge("output1_temperature", "Output 1 Temperature", "/router/output1/temperature", "temperature", "mdi:thermometer", "°C"));

  haDiscovery.publish(Mycila::HAText("output2_state", "Output 2", "/router/output2/state"));
  haDiscovery.publish(Mycila::HAState("output2_bypass", "Output 2 Bypass", "/router/output2/bypass", YASOLR_ON, YASOLR_OFF, "running"));
  haDiscovery.publish(Mycila::HANumber("output2_dimmer_duty", "Output 2 Dimmer Duty", "/router/output2/dimmer/duty/set", "/router/output2/dimmer/duty", Mycila::HANumberMode::SLIDER, 0, YASOLR_DIMMER_MAX_LEVEL, 1, "mdi:water-boiler"));
  haDiscovery.publish(Mycila::HACounter("output2_relay_switch_count", "Output 2 Bypass Relay Switch Count", "/router/output2/relay/switch_count", nullptr, "mdi:counter"));
  haDiscovery.publish(Mycila::HAOutlet("output2_relay", "Output 2 Bypass", "/router/output2/bypass/set", "/router/output2/bypass", YASOLR_ON, YASOLR_OFF));
  haDiscovery.publish(Mycila::HAGauge("output2_temperature", "Output 2 Temperature", "/router/output2/temperature", "temperature", "mdi:thermometer", "°C"));

  haDiscovery.end();
});
