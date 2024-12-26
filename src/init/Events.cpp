// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

#include <string>

extern YaSolR::Website website;

Mycila::Task initEventsTask("Init Events", [](void* params) {
  logger.info(TAG, "Initializing Events");

  mqtt.onConnect([](void) {
    logger.info(TAG, "MQTT connected!");
    if (config.getBool(KEY_ENABLE_HA_DISCOVERY) && !config.isEmpty(KEY_HA_DISCOVERY_TOPIC))
      haDiscoveryTask.resume();
    mqttPublishStaticTask.resume();
    mqttPublishConfigTask.resume();
  });

  config.listen([]() {
    logger.info(TAG, "Configuration restored!");
    restartTask.resume();
  });

  config.listen([](const char* k, const std::string& newValue) {
    logger.info(TAG, "'%s' => '%s'", k, newValue.c_str());
    const std::string key = k;

    if (key == KEY_ENABLE_DEBUG) {
      initLoggingTask.forceRun();

    } else if (key == KEY_RELAY1_LOAD) {
      routerRelay1.setLoad(config.getLong(KEY_RELAY1_LOAD));

    } else if (key == KEY_RELAY2_LOAD) {
      routerRelay2.setLoad(config.getLong(KEY_RELAY2_LOAD));

    } else if (key == KEY_OUTPUT1_RESISTANCE) {
      output1.config.calibratedResistance = config.getFloat(KEY_OUTPUT1_RESISTANCE);

    } else if (key == KEY_OUTPUT2_RESISTANCE) {
      output2.config.calibratedResistance = config.getFloat(KEY_OUTPUT2_RESISTANCE);

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_DIMMER) {
      output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
      dimmerO1.off();

    } else if (key == KEY_OUTPUT1_DIMMER_MIN) {
      dimmerO1.setDutyCycleMin(config.getFloat(KEY_OUTPUT1_DIMMER_MIN) / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_MAX) {
      dimmerO1.setDutyCycleMax(config.getFloat(KEY_OUTPUT1_DIMMER_MAX) / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_LIMIT) {
      dimmerO1.setDutyCycleLimit(config.getFloat(KEY_OUTPUT1_DIMMER_LIMIT) / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_STOP_TEMP) {
      output1.config.dimmerTempLimit = config.getLong(KEY_OUTPUT1_DIMMER_STOP_TEMP);

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_BYPASS) {
      output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_START) {
      output1.config.autoStartTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_START);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_STOP) {
      output1.config.autoStopTemperature = config.getLong(KEY_OUTPUT1_TEMPERATURE_STOP);

    } else if (key == KEY_OUTPUT1_TIME_START) {
      output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);

    } else if (key == KEY_OUTPUT1_TIME_STOP) {
      output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);

    } else if (key == KEY_OUTPUT1_DAYS) {
      output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);

    } else if (key == KEY_OUTPUT1_RESERVED_EXCESS) {
      output1.config.reservedExcessPowerRatio = config.getFloat(KEY_OUTPUT1_RESERVED_EXCESS) / 100;

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_DIMMER) {
      output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
      dimmerO2.off();

    } else if (key == KEY_OUTPUT2_DIMMER_MIN) {
      dimmerO2.setDutyCycleMin(config.getFloat(KEY_OUTPUT2_DIMMER_MIN) / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_MAX) {
      dimmerO2.setDutyCycleMax(config.getFloat(KEY_OUTPUT2_DIMMER_MAX) / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_LIMIT) {
      dimmerO2.setDutyCycleLimit(config.getFloat(KEY_OUTPUT2_DIMMER_LIMIT) / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_STOP_TEMP) {
      output2.config.dimmerTempLimit = config.getLong(KEY_OUTPUT2_DIMMER_STOP_TEMP);

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_BYPASS) {
      output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_START) {
      output2.config.autoStartTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_START);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_STOP) {
      output2.config.autoStopTemperature = config.getLong(KEY_OUTPUT2_TEMPERATURE_STOP);

    } else if (key == KEY_OUTPUT2_TIME_START) {
      output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);

    } else if (key == KEY_OUTPUT2_TIME_STOP) {
      output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);

    } else if (key == KEY_OUTPUT2_DAYS) {
      output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);

    } else if (key == KEY_OUTPUT2_RESERVED_EXCESS) {
      output2.config.reservedExcessPowerRatio = config.getFloat(KEY_OUTPUT2_RESERVED_EXCESS) / 100;

    } else if (key == KEY_NTP_TIMEZONE) {
      Mycila::NTP.setTimeZone(config.get(KEY_NTP_TIMEZONE));

    } else if (key == KEY_NTP_SERVER) {
      if (!config.getBool(KEY_ENABLE_AP_MODE))
        Mycila::NTP.sync(config.get(KEY_NTP_SERVER));

    } else if (key == KEY_HA_DISCOVERY_TOPIC) {
      haDiscovery.setDiscoveryTopic(config.get(KEY_HA_DISCOVERY_TOPIC));

    } else if (key == KEY_ENABLE_LIGHTS) {
      lights.end();
      if (config.getBool(KEY_ENABLE_LIGHTS))
        lights.begin(config.getLong(KEY_PIN_LIGHTS_GREEN), config.getLong(KEY_PIN_LIGHTS_YELLOW), config.getLong(KEY_PIN_LIGHTS_RED));

    } else if (key == KEY_ENABLE_DS18_SYSTEM) {
      ds18Sys.end();
      if (config.getBool(KEY_ENABLE_DS18_SYSTEM))
        ds18Sys.begin(config.getLong(KEY_PIN_ROUTER_DS18));

    } else if (key == KEY_ENABLE_OUTPUT1_DS18) {
      ds18O1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_DS18))
        ds18O1.begin(config.getLong(KEY_PIN_OUTPUT1_DS18));

    } else if (key == KEY_ENABLE_OUTPUT2_DS18) {
      ds18O2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_DS18))
        ds18O2.begin(config.getLong(KEY_PIN_OUTPUT2_DS18));

    } else if (key == KEY_ENABLE_MQTT) {
      mqttConfigTask.resume();

    } else if (key == KEY_ENABLE_ZCD) {
      zcdTask.requestEarlyRun();

    } else if (key == KEY_ENABLE_OUTPUT1_DIMMER) {
      zcdTask.requestEarlyRun();

    } else if (key == KEY_ENABLE_OUTPUT2_DIMMER) {
      zcdTask.requestEarlyRun();

    } else if (key == KEY_ENABLE_OUTPUT1_RELAY) {
      bypassRelayO1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY))
        bypassRelayO1.begin(config.getLong(KEY_PIN_OUTPUT1_RELAY), config.isEqual(KEY_OUTPUT1_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_OUTPUT2_RELAY) {
      bypassRelayO2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY))
        bypassRelayO2.begin(config.getLong(KEY_PIN_OUTPUT2_RELAY), config.isEqual(KEY_OUTPUT2_RELAY_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_RELAY1) {
      relay1.end();
      if (config.getBool(KEY_ENABLE_RELAY1))
        relay1.begin(config.getLong(KEY_PIN_RELAY1), config.isEqual(KEY_RELAY1_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_RELAY2) {
      relay2.end();
      if (config.getBool(KEY_ENABLE_RELAY2))
        relay2.begin(config.getLong(KEY_PIN_RELAY2), config.isEqual(KEY_RELAY2_TYPE, YASOLR_RELAY_TYPE_NC) ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_JSY) {
      jsy.end();
      if (config.getBool(KEY_ENABLE_JSY)) {
        jsy.begin(YASOLR_JSY_SERIAL, config.getLong(KEY_PIN_JSY_RX), config.getLong(KEY_PIN_JSY_TX));
        if (jsy.isEnabled() && jsy.getBaudRate() != jsy.getMaxAvailableBaudRate())
          jsy.setBaudRate(jsy.getMaxAvailableBaudRate());
      }

    } else if (key == KEY_ENABLE_OUTPUT1_PZEM) {
      pzemO1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM))
        pzemO1.begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT1);

    } else if (key == KEY_ENABLE_OUTPUT2_PZEM) {
      pzemO2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM))
        pzemO2.begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), YASOLR_PZEM_ADDRESS_OUTPUT2);

    } else if (key == KEY_ENABLE_DISPLAY) {
      display.end();
      if (config.getBool(KEY_ENABLE_DISPLAY)) {
        const std::string& displayType = config.getString(KEY_DISPLAY_TYPE);
        if (displayType == "SSD1306")
          display.begin(Mycila::EasyDisplayType::SSD1306, config.getLong(KEY_PIN_DISPLAY_SCL), config.getLong(KEY_PIN_DISPLAY_SDA), config.getLong(KEY_DISPLAY_ROTATION));
        else if (displayType == "SH1107")
          display.begin(Mycila::EasyDisplayType::SH1107, config.getLong(KEY_PIN_DISPLAY_SCL), config.getLong(KEY_PIN_DISPLAY_SDA), config.getLong(KEY_DISPLAY_ROTATION));
        else if (displayType == "SH1106")
          display.begin(Mycila::EasyDisplayType::SH1106, config.getLong(KEY_PIN_DISPLAY_SCL), config.getLong(KEY_PIN_DISPLAY_SDA), config.getLong(KEY_DISPLAY_ROTATION));
        display.clearDisplay();
        display.setActive(true);
      }

    } else if (key == KEY_PID_KP || key == KEY_PID_KI || key == KEY_PID_KD || key == KEY_PID_OUT_MIN || key == KEY_PID_OUT_MAX || key == KEY_PID_P_MODE || key == KEY_PID_D_MODE || key == KEY_PID_IC_MODE || key == KEY_PID_SETPOINT) {
      pidController.setProportionalMode((Mycila::PID::ProportionalMode)config.getLong(KEY_PID_P_MODE));
      pidController.setDerivativeMode((Mycila::PID::DerivativeMode)config.getLong(KEY_PID_D_MODE));
      pidController.setIntegralCorrectionMode((Mycila::PID::IntegralCorrectionMode)config.getLong(KEY_PID_IC_MODE));
      pidController.setSetPoint(config.getFloat(KEY_PID_SETPOINT));
      pidController.setTunings(config.getFloat(KEY_PID_KP), config.getFloat(KEY_PID_KI), config.getFloat(KEY_PID_KD));
      pidController.setOutputLimits(config.getFloat(KEY_PID_OUT_MIN), config.getFloat(KEY_PID_OUT_MAX));
      logger.info(TAG, "PID Controller reconfigured!");

    } else if (key == KEY_DISPLAY_SPEED) {
      carouselTask.setInterval(config.getLong(KEY_DISPLAY_SPEED) * Mycila::TaskDuration::SECONDS);

    } else if (key == KEY_MQTT_PUBLISH_INTERVAL) {
      mqttPublishTask.setInterval(config.getLong(KEY_MQTT_PUBLISH_INTERVAL) * Mycila::TaskDuration::SECONDS);
    }

    dashboardInitTask.resume();
    mqttPublishConfigTask.resume();
    mqttPublishTask.requestEarlyRun();
  });

  dashboard.onBeforeUpdate([](bool changes_only) {
    if (!changes_only) {
      logger.info(TAG, "Dashboard refresh requested");
      website.initCards();
    }
  });

  espConnect.listen([](Mycila::ESPConnect::State previous, Mycila::ESPConnect::State state) {
    logger.debug(TAG, "NetworkState: %s => %s", espConnect.getStateName(previous), espConnect.getStateName(state));
    switch (state) {
      case Mycila::ESPConnect::State::NETWORK_DISABLED:
        logger.warn(TAG, "Disabled Network!");
        break;
      case Mycila::ESPConnect::State::AP_STARTING:
        logger.info(TAG, "Starting Access Point %s", espConnect.getAccessPointSSID().c_str());
        break;
      case Mycila::ESPConnect::State::AP_STARTED:
        logger.info(TAG, "Access Point %s started with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        networkConfigTask.resume();
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTING:
        logger.info(TAG, "Connecting to network");
        break;
      case Mycila::ESPConnect::State::NETWORK_CONNECTED:
        logger.info(TAG, "Connected with IP address %s", espConnect.getIPAddress().toString().c_str());
        networkConfigTask.resume();
        break;
      case Mycila::ESPConnect::State::NETWORK_TIMEOUT:
        logger.warn(TAG, "Unable to connect!");
        break;
      case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
        logger.warn(TAG, "Disconnected!");
        break;
      case Mycila::ESPConnect::State::NETWORK_RECONNECTING:
        logger.info(TAG, "Trying to reconnect");
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTING:
        logger.info(TAG, "Starting Captive Portal %s for %" PRIu32 " seconds", espConnect.getAccessPointSSID().c_str(), espConnect.getCaptivePortalTimeout());
        break;
      case Mycila::ESPConnect::State::PORTAL_STARTED:
        logger.info(TAG, "Captive Portal started at %s with IP address %s", espConnect.getWiFiSSID().c_str(), espConnect.getIPAddress().toString().c_str());
        break;
      case Mycila::ESPConnect::State::PORTAL_COMPLETE: {
        if (espConnect.hasConfiguredAPMode()) {
          logger.info(TAG, "Captive Portal: Access Point configured");
          config.setBool(KEY_ENABLE_AP_MODE, true);
        } else {
          logger.info(TAG, "Captive Portal: WiFi configured");
          config.setBool(KEY_ENABLE_AP_MODE, false);
          config.set(KEY_WIFI_SSID, espConnect.getConfiguredWiFiSSID());
          config.set(KEY_WIFI_PASSWORD, espConnect.getConfiguredWiFiPassword());
        }
        break;
      }
      case Mycila::ESPConnect::State::PORTAL_TIMEOUT:
        logger.warn(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
    dashboardInitTask.resume();
  });

  bypassRelayO1.listen([](bool state) {
    logger.info(TAG, "Output 1 Relay changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });
  bypassRelayO2.listen([](bool state) {
    logger.info(TAG, "Output 2 Relay changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });
  relay1.listen([](bool state) {
    logger.info(TAG, "Relay 1 changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });
  relay2.listen([](bool state) {
    logger.info(TAG, "Relay 2 changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });

  ds18Sys.listen([](float temperature, bool changed) {
    if (changed) {
      logger.info(TAG, "Router Temperature changed to %.02f °C", temperature);
      mqttPublishTask.requestEarlyRun();
    }
  });
  ds18O1.listen([](float temperature, bool changed) {
    output1.temperature().update(temperature);
    if (changed) {
      logger.info(TAG, "Output 1 Temperature changed to %.02f °C", temperature);
      mqttPublishTask.requestEarlyRun();
    }
  });
  ds18O2.listen([](float temperature, bool changed) {
    output2.temperature().update(temperature);
    if (changed) {
      logger.info(TAG, "Output 2 Temperature changed to %.02f °C", temperature);
      mqttPublishTask.requestEarlyRun();
    }
  });

  pzemO1.setCallback([](const Mycila::PZEM::EventType eventType) {
    if (eventType == Mycila::PZEM::EventType::EVT_READ) {
      grid.pzemMetrics().update({
        .apparentPower = NAN,
        .current = NAN,
        .energy = NAN,
        .energyReturned = NAN,
        .frequency = pzemO1.data.frequency,
        .power = NAN,
        .powerFactor = NAN,
        .voltage = pzemO1.data.voltage,
      });
    }
  });

  pzemO2.setCallback([](const Mycila::PZEM::EventType eventType) {
    if (eventType == Mycila::PZEM::EventType::EVT_READ) {
      grid.pzemMetrics().update({
        .apparentPower = NAN,
        .current = NAN,
        .energy = NAN,
        .energyReturned = NAN,
        .frequency = pzemO2.data.frequency,
        .power = NAN,
        .powerFactor = NAN,
        .voltage = pzemO2.data.voltage,
      });
    }
  });

  jsy.setCallback([](const Mycila::JSY::EventType eventType) {
    if (eventType == Mycila::JSY::EventType::EVT_CHANGE) {
      switch (jsy.data.model) {
        case MYCILA_JSY_MK_1031:
          // JSY1030 has no sign: it cannot be used to measure the grid
          break;

        case MYCILA_JSY_MK_163:
        case MYCILA_JSY_MK_227:
        case MYCILA_JSY_MK_229:
          grid.localMetrics().update({
            .apparentPower = jsy.data.single().apparentPower,
            .current = jsy.data.single().current,
            .energy = jsy.data.single().activeEnergyImported,
            .energyReturned = jsy.data.single().activeEnergyReturned,
            .frequency = jsy.data.single().frequency,
            .power = jsy.data.single().activePower,
            .powerFactor = jsy.data.single().powerFactor,
            .voltage = jsy.data.single().voltage,
          });
          break;

        case MYCILA_JSY_MK_193:
        case MYCILA_JSY_MK_194:
          grid.localMetrics().update({
            .apparentPower = jsy.data.channel2().apparentPower,
            .current = jsy.data.channel2().current,
            .energy = jsy.data.channel2().activeEnergyImported,
            .energyReturned = jsy.data.channel2().activeEnergyReturned,
            .frequency = jsy.data.aggregate.frequency,
            .power = jsy.data.channel2().activePower,
            .powerFactor = jsy.data.channel2().powerFactor,
            .voltage = jsy.data.channel2().voltage,
          });
          break;

        case MYCILA_JSY_MK_333:
          grid.localMetrics().update({
            .apparentPower = jsy.data.aggregate.apparentPower,
            .current = jsy.data.aggregate.current,
            .energy = jsy.data.aggregate.activeEnergyImported,
            .energyReturned = jsy.data.aggregate.activeEnergyReturned,
            .frequency = jsy.data.aggregate.frequency,
            .power = jsy.data.aggregate.activePower,
            .powerFactor = jsy.data.aggregate.powerFactor,
            .voltage = jsy.data.aggregate.voltage,
          });
          break;

        default:
          break;
      }
      if (grid.updatePower()) {
        routingTask.forceRun();
      }
    }
  });

  udp.onPacket([](AsyncUDPPacket packet) {
    // buffer[0] == MYCILA_UDP_MSG_TYPE_JSY_DATA (1)
    // buffer[1] == size_t (4)
    // buffer[5] == MsgPack (?)
    // buffer[5 + size] == CRC32 (4)

    size_t len = packet.length();
    uint8_t* buffer = packet.data();

    if (len < 5 || buffer[0] != YASOLR_UDP_MSG_TYPE_JSY_DATA)
      return;

    uint32_t size;
    memcpy(&size, buffer + 1, 4);

    if (len != size + 9)
      return;

    // crc32
    FastCRC32 crc32;
    crc32.add(buffer, size + 5);
    uint32_t crc = crc32.calc();

    if (memcmp(&crc, buffer + size + 5, 4) != 0)
      return;

    udpMessageRateBuffer.add(millis() / 1000.0f);

    JsonDocument doc;
    deserializeMsgPack(doc, buffer + 5, size);

    switch (doc["model"].as<uint16_t>()) {
      case MYCILA_JSY_MK_1031:
        // JSY1030 has no sign: it cannot be used to measure the grid
        break;

      case MYCILA_JSY_MK_163:
      case MYCILA_JSY_MK_227:
      case MYCILA_JSY_MK_229: {
        grid.remoteMetrics().update({
          .apparentPower = doc["apparent_power"] | NAN,
          .current = doc["current"] | NAN,
          .energy = doc["active_energy_imported"] | NAN,
          .energyReturned = doc["active_energy_returned"] | NAN,
          .power = doc["active_power"] | NAN,
          .powerFactor = doc["power_factor"] | NAN,
          .voltage = doc["voltage"] | NAN,
        });
        break;
      }
      case MYCILA_JSY_MK_193:
      case MYCILA_JSY_MK_194: {
        JsonObject channel2 = doc["channel2"].as<JsonObject>();
        grid.remoteMetrics().update({
          .apparentPower = channel2["apparent_power"] | NAN,
          .current = channel2["current"] | NAN,
          .energy = channel2["active_energy_imported"] | NAN,
          .energyReturned = channel2["active_energy_returned"] | NAN,
          .frequency = channel2["frequency"] | NAN,
          .power = channel2["active_power"] | NAN,
          .powerFactor = channel2["power_factor"] | NAN,
          .voltage = channel2["voltage"] | NAN,
        });
        break;
      }
      case MYCILA_JSY_MK_333: {
        JsonObject aggregate = doc["aggregate"].as<JsonObject>();
        grid.remoteMetrics().update({
          .apparentPower = aggregate["apparent_power"] | NAN,
          .current = aggregate["current"] | NAN,
          .energy = aggregate["active_energy_imported"] | NAN,
          .energyReturned = aggregate["active_energy_returned"] | NAN,
          .frequency = aggregate["frequency"] | NAN,
          .power = aggregate["active_power"] | NAN,
          .powerFactor = aggregate["power_factor"] | NAN,
          .voltage = aggregate["voltage"] | NAN,
        });
        break;
      }
      default:
        break;
    }

    if (grid.updatePower()) {
      routingTask.forceRun();
    }
  });
});
