// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#include <string>

Mycila::Task mqttPublishTask("MQTT", [](void* params) {
  const std::string& baseTopic = config.getString(KEY_MQTT_TOPIC);

  Mycila::System::Memory memory;
  Mycila::System::getMemory(memory);

  mqtt.publish(baseTopic + "/system/device/heap/total", std::to_string(memory.total));
  mqtt.publish(baseTopic + "/system/device/heap/usage", std::to_string(memory.usage));
  mqtt.publish(baseTopic + "/system/device/heap/used", std::to_string(memory.used));
  mqtt.publish(baseTopic + "/system/device/uptime", std::to_string(Mycila::System::getUptime()));
  yield();

  mqtt.publish(baseTopic + "/system/network/eth/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::ETH).toString().c_str());
  mqtt.publish(baseTopic + "/system/network/ip_address", espConnect.getIPAddress().toString().c_str());
  mqtt.publish(baseTopic + "/system/network/mac_address", espConnect.getMACAddress());
  mqtt.publish(baseTopic + "/system/network/ntp", YASOLR_STATE(Mycila::NTP.isSynced()));
  mqtt.publish(baseTopic + "/system/network/wifi/bssid", espConnect.getWiFiBSSID());
  mqtt.publish(baseTopic + "/system/network/wifi/ip_address", espConnect.getIPAddress(Mycila::ESPConnect::Mode::STA).toString().c_str());
  mqtt.publish(baseTopic + "/system/network/wifi/quality", std::to_string(espConnect.getWiFiSignalQuality()));
  mqtt.publish(baseTopic + "/system/network/wifi/rssi", std::to_string(espConnect.getWiFiRSSI()));
  mqtt.publish(baseTopic + "/system/network/wifi/ssid", espConnect.getWiFiSSID());
  yield();

  switch (espConnect.getMode()) {
    case Mycila::ESPConnect::Mode::ETH:
      mqtt.publish(baseTopic + "/system/network/mode", "eth");
      break;
    case Mycila::ESPConnect::Mode::STA:
      mqtt.publish(baseTopic + "/system/network/mode", "wifi");
      break;
    case Mycila::ESPConnect::Mode::AP:
      mqtt.publish(baseTopic + "/system/network/mode", "ap");
      break;
    default:
      mqtt.publish(baseTopic + "/system/network/mode", "");
      break;
  }

  Mycila::Grid::Metrics gridMetrics;
  grid.getGridMeasurements(gridMetrics);
  mqtt.publish(baseTopic + "/grid/apparent_power", std::to_string(gridMetrics.apparentPower));
  mqtt.publish(baseTopic + "/grid/current", std::to_string(gridMetrics.current));
  mqtt.publish(baseTopic + "/grid/energy", std::to_string(gridMetrics.energy));
  mqtt.publish(baseTopic + "/grid/energy_returned", std::to_string(gridMetrics.energyReturned));
  mqtt.publish(baseTopic + "/grid/frequency", std::to_string(gridMetrics.frequency));
  mqtt.publish(baseTopic + "/grid/online", YASOLR_BOOL(grid.isConnected()));
  mqtt.publish(baseTopic + "/grid/power", std::to_string(gridMetrics.power));
  mqtt.publish(baseTopic + "/grid/power_factor", std::to_string(gridMetrics.powerFactor));
  mqtt.publish(baseTopic + "/grid/voltage", std::to_string(gridMetrics.voltage));
  yield();

  Mycila::Router::Metrics routerMeasurements;
  router.getRouterMeasurements(routerMeasurements);
  mqtt.publish(baseTopic + "/router/apparent_power", std::to_string(routerMeasurements.apparentPower));
  mqtt.publish(baseTopic + "/router/current", std::to_string(routerMeasurements.current));
  mqtt.publish(baseTopic + "/router/energy", std::to_string(routerMeasurements.energy));
  mqtt.publish(baseTopic + "/router/lights", lights.toString());
  mqtt.publish(baseTopic + "/router/power_factor", isnan(routerMeasurements.powerFactor) ? "0" : std::to_string(routerMeasurements.powerFactor));
  mqtt.publish(baseTopic + "/router/power", std::to_string(routerMeasurements.power));
  mqtt.publish(baseTopic + "/router/relay1", YASOLR_STATE(relay1.isOn()));
  mqtt.publish(baseTopic + "/router/relay2", YASOLR_STATE(relay2.isOn()));
  if (ds18Sys)
    mqtt.publish(baseTopic + "/router/temperature", std::to_string(ds18Sys->getTemperature().value_or(0)));
  mqtt.publish(baseTopic + "/router/thdi", isnan(routerMeasurements.thdi) ? "0" : std::to_string(routerMeasurements.thdi));
  mqtt.publish(baseTopic + "/router/virtual_grid_power", std::to_string(gridMetrics.power - routerMeasurements.power));
  yield();

  for (const auto& output : router.getOutputs()) {
    const std::string outputTopic = baseTopic + "/router/" + output->getName();
    mqtt.publish(outputTopic + "/state", output->getStateName());
    mqtt.publish(outputTopic + "/bypass", YASOLR_STATE(output->isBypassOn()));
    mqtt.publish(outputTopic + "/dimmer", YASOLR_STATE(output->isDimmerOn()));
    mqtt.publish(outputTopic + "/duty_cycle", std::to_string(output->getDimmerDutyCycle() * 100));
    mqtt.publish(outputTopic + "/temperature", std::to_string(output->temperature().orElse(0)));
    yield();
  }
});
