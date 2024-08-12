// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

Mycila::Task initEventsTask("Init Events", [](void* params) {
  logger.info(TAG, "Initializing Events");

  mqtt.onConnect([](void) {
    logger.info(TAG, "MQTT connected!");
    if (config.getBool(KEY_ENABLE_HA_DISCOVERY) && !config.get(KEY_HA_DISCOVERY_TOPIC).isEmpty())
      haDiscoveryTask.resume();
    mqttPublishStaticTask.resume();
    mqttPublishConfigTask.resume();
  });

  config.listen([]() {
    logger.info(TAG, "Configuration restored!");
    restartTask.resume();
  });

  config.listen([](const String& key, const String& oldValue, const String& newValue) {
    logger.info(TAG, "Set %s: '%s' => '%s'", key.c_str(), oldValue.c_str(), newValue.c_str());

    if (key == KEY_ENABLE_DEBUG) {
      initLoggingTask.forceRun();

    } else if (key == KEY_RELAY1_LOAD) {
      routerRelay1.setLoad(config.get(KEY_RELAY1_LOAD).toInt());

    } else if (key == KEY_RELAY2_LOAD) {
      routerRelay2.setLoad(config.get(KEY_RELAY2_LOAD).toInt());

    } else if (key == KEY_OUTPUT1_RESISTANCE) {
      output1.config.calibratedResistance = config.get(KEY_OUTPUT1_RESISTANCE).toFloat();

    } else if (key == KEY_OUTPUT2_RESISTANCE) {
      output2.config.calibratedResistance = config.get(KEY_OUTPUT2_RESISTANCE).toFloat();

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_DIMMER) {
      output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);
      dimmerO1.off();

    } else if (key == KEY_OUTPUT1_DIMMER_MIN) {
      dimmerO1.setDutyCycleMin(config.get(KEY_OUTPUT1_DIMMER_MIN).toFloat() / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_MAX) {
      dimmerO1.setDutyCycleMax(config.get(KEY_OUTPUT1_DIMMER_MAX).toFloat() / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_LIMIT) {
      dimmerO1.setDutyCycleLimit(config.get(KEY_OUTPUT1_DIMMER_LIMIT).toFloat() / 100);

    } else if (key == KEY_OUTPUT1_DIMMER_STOP_TEMP) {
      output1.config.dimmerTempLimit = config.get(KEY_OUTPUT1_DIMMER_STOP_TEMP).toInt();

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_BYPASS) {
      output1.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT1_TEMPERATURE_START) {
      output1.config.autoStartTemperature = config.get(KEY_OUTPUT1_TEMPERATURE_START).toInt();

    } else if (key == KEY_OUTPUT1_TEMPERATURE_STOP) {
      output1.config.autoStopTemperature = config.get(KEY_OUTPUT1_TEMPERATURE_STOP).toInt();

    } else if (key == KEY_OUTPUT1_TIME_START) {
      output1.config.autoStartTime = config.get(KEY_OUTPUT1_TIME_START);

    } else if (key == KEY_OUTPUT1_TIME_STOP) {
      output1.config.autoStopTime = config.get(KEY_OUTPUT1_TIME_STOP);

    } else if (key == KEY_OUTPUT1_DAYS) {
      output1.config.weekDays = config.get(KEY_OUTPUT1_DAYS);

    } else if (key == KEY_OUTPUT1_RESERVED_EXCESS) {
      output1.config.reservedExcessPowerRatio = config.get(KEY_OUTPUT1_RESERVED_EXCESS).toFloat() / 100;

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_DIMMER) {
      output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);
      dimmerO2.off();

    } else if (key == KEY_OUTPUT2_DIMMER_MIN) {
      dimmerO2.setDutyCycleMin(config.get(KEY_OUTPUT2_DIMMER_MIN).toFloat() / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_MAX) {
      dimmerO2.setDutyCycleMax(config.get(KEY_OUTPUT2_DIMMER_MAX).toFloat() / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_LIMIT) {
      dimmerO2.setDutyCycleLimit(config.get(KEY_OUTPUT2_DIMMER_LIMIT).toFloat() / 100);

    } else if (key == KEY_OUTPUT2_DIMMER_STOP_TEMP) {
      output2.config.dimmerTempLimit = config.get(KEY_OUTPUT2_DIMMER_STOP_TEMP).toInt();

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_BYPASS) {
      output2.config.autoBypass = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_BYPASS);

    } else if (key == KEY_OUTPUT2_TEMPERATURE_START) {
      output2.config.autoStartTemperature = config.get(KEY_OUTPUT2_TEMPERATURE_START).toInt();

    } else if (key == KEY_OUTPUT2_TEMPERATURE_STOP) {
      output2.config.autoStopTemperature = config.get(KEY_OUTPUT2_TEMPERATURE_STOP).toInt();

    } else if (key == KEY_OUTPUT2_TIME_START) {
      output2.config.autoStartTime = config.get(KEY_OUTPUT2_TIME_START);

    } else if (key == KEY_OUTPUT2_TIME_STOP) {
      output2.config.autoStopTime = config.get(KEY_OUTPUT2_TIME_STOP);

    } else if (key == KEY_OUTPUT2_DAYS) {
      output2.config.weekDays = config.get(KEY_OUTPUT2_DAYS);

    } else if (key == KEY_OUTPUT2_RESERVED_EXCESS) {
      output2.config.reservedExcessPowerRatio = config.get(KEY_OUTPUT2_RESERVED_EXCESS).toFloat() / 100;

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
        lights.begin(config.get(KEY_PIN_LIGHTS_GREEN).toInt(), config.get(KEY_PIN_LIGHTS_YELLOW).toInt(), config.get(KEY_PIN_LIGHTS_RED).toInt());

    } else if (key == KEY_ENABLE_DS18_SYSTEM) {
      ds18Sys.end();
      if (config.getBool(KEY_ENABLE_DS18_SYSTEM))
        ds18Sys.begin(config.get(KEY_PIN_ROUTER_DS18).toInt());

    } else if (key == KEY_ENABLE_OUTPUT1_DS18) {
      ds18O1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_DS18))
        ds18O1.begin(config.get(KEY_PIN_OUTPUT1_DS18).toInt());

    } else if (key == KEY_ENABLE_OUTPUT2_DS18) {
      ds18O2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_DS18))
        ds18O2.begin(config.get(KEY_PIN_OUTPUT2_DS18).toInt());

    } else if (key == KEY_ENABLE_ZCD) {
      zcdTask.requestEarlyRun();

    } else if (key == KEY_ENABLE_MQTT) {
      mqttConfigTask.resume();

    } else if (key == KEY_ENABLE_OUTPUT1_DIMMER) {
      dimmer1Task.resume();

    } else if (key == KEY_ENABLE_OUTPUT2_DIMMER) {
      dimmer2Task.resume();

    } else if (key == KEY_ENABLE_OUTPUT1_RELAY) {
      bypassRelayO1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_RELAY))
        bypassRelayO1.begin(config.get(KEY_PIN_OUTPUT1_RELAY).toInt(), config.get(KEY_OUTPUT1_RELAY_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_OUTPUT2_RELAY) {
      bypassRelayO2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_RELAY))
        bypassRelayO2.begin(config.get(KEY_PIN_OUTPUT2_RELAY).toInt(), config.get(KEY_OUTPUT2_RELAY_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_RELAY1) {
      relay1.end();
      if (config.getBool(KEY_ENABLE_RELAY1))
        relay1.begin(config.get(KEY_PIN_RELAY1).toInt(), config.get(KEY_RELAY1_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_RELAY2) {
      relay2.end();
      if (config.getBool(KEY_ENABLE_RELAY2))
        relay2.begin(config.get(KEY_PIN_RELAY2).toInt(), config.get(KEY_RELAY2_TYPE) == YASOLR_RELAY_TYPE_NC ? Mycila::RelayType::NC : Mycila::RelayType::NO);

    } else if (key == KEY_ENABLE_JSY) {
      Mycila::JSYBaudRate bauds = jsy.getBaudRate();
      jsy.end();
      if (config.getBool(KEY_ENABLE_JSY)) {
        jsy.begin(YASOLR_JSY_SERIAL, config.get(KEY_PIN_JSY_RX).toInt(), config.get(KEY_PIN_JSY_TX).toInt(), bauds);
      }

    } else if (key == KEY_ENABLE_OUTPUT1_PZEM) {
      pzemO1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM))
        pzemO1.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT1);

    } else if (key == KEY_ENABLE_OUTPUT2_PZEM) {
      pzemO2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM))
        pzemO2.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT2);

    } else if (key == KEY_ENABLE_DISPLAY) {
      display.end();
      if (config.getBool(KEY_ENABLE_DISPLAY)) {
        const String displayType = config.get(KEY_DISPLAY_TYPE);
        if (displayType == "SSD1306")
          display.begin(Mycila::EasyDisplayType::SSD1306, config.get(KEY_PIN_DISPLAY_SCL).toInt(), config.get(KEY_PIN_DISPLAY_SDA).toInt(), config.get(KEY_DISPLAY_ROTATION).toInt());
        else if (displayType == "SH1107")
          display.begin(Mycila::EasyDisplayType::SH1107, config.get(KEY_PIN_DISPLAY_SCL).toInt(), config.get(KEY_PIN_DISPLAY_SDA).toInt(), config.get(KEY_DISPLAY_ROTATION).toInt());
        else if (displayType == "SH1106")
          display.begin(Mycila::EasyDisplayType::SH1106, config.get(KEY_PIN_DISPLAY_SCL).toInt(), config.get(KEY_PIN_DISPLAY_SDA).toInt(), config.get(KEY_DISPLAY_ROTATION).toInt());
        display.clearDisplay();
        display.setActive(true);
      }

    } else if (key == KEY_PID_KP || key == KEY_PID_KI || key == KEY_PID_KD || key == KEY_PID_OUT_MIN || key == KEY_PID_OUT_MAX || key == KEY_PID_P_MODE || key == KEY_PID_D_MODE || key == KEY_PID_IC_MODE || key == KEY_PID_SETPOINT) {
      pidController.setProportionalMode((Mycila::PID::ProportionalMode)config.get(KEY_PID_P_MODE).toInt());
      pidController.setDerivativeMode((Mycila::PID::DerivativeMode)config.get(KEY_PID_D_MODE).toInt());
      pidController.setIntegralCorrectionMode((Mycila::PID::IntegralCorrectionMode)config.get(KEY_PID_IC_MODE).toInt());
      pidController.setSetPoint(config.get(KEY_PID_SETPOINT).toFloat());
      pidController.setTunings(config.get(KEY_PID_KP).toFloat(), config.get(KEY_PID_KI).toFloat(), config.get(KEY_PID_KD).toFloat());
      pidController.setOutputLimits(config.get(KEY_PID_OUT_MIN).toFloat(), config.get(KEY_PID_OUT_MAX).toFloat());
      logger.info(TAG, "PID Controller reconfigured!");
    }

    YaSolR::Website.initCards();
    mqttPublishConfigTask.resume();
    mqttPublishTask.requestEarlyRun();
  });

  dashboard.onBeforeUpdate([](bool changes_only) {
    if (!changes_only) {
      logger.info(TAG, "Dashboard refresh requested");
      YaSolR::Website.initCards();
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

  pzemO1.setCallback([](const Mycila::PZEMEventType eventType) {
    if (eventType == Mycila::PZEMEventType::EVT_READ) {
      grid.pzemVoltage().update(pzemO1.getVoltage());
    }
  });

  pzemO2.setCallback([](const Mycila::PZEMEventType eventType) {
    if (eventType == Mycila::PZEMEventType::EVT_READ) {
      grid.pzemVoltage().update(pzemO2.getVoltage());
    }
  });

  jsy.setCallback([](const Mycila::JSYEventType eventType) {
    if (eventType == Mycila::JSYEventType::EVT_CHANGE) {
      grid.localMetrics().update({
        .apparentPower = jsy.getApparentPower2(),
        .current = jsy.getCurrent2(),
        .energy = jsy.getEnergy2(),
        .energyReturned = jsy.getEnergyReturned2(),
        .frequency = jsy.getFrequency(),
        .power = jsy.getPower2(),
        .powerFactor = jsy.getPowerFactor2(),
        .voltage = jsy.getVoltage2(),
      });
      if (grid.updatePower()) {
        routingTask.resume();
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

    JsonDocument doc;
    deserializeMsgPack(doc, buffer + 5, size);

    grid.remoteMetrics().update({
      .apparentPower = doc["pf2"].as<float>() == 0 ? 0 : doc["p2"].as<float>() / doc["pf2"].as<float>(),
      .current = doc["c2"].as<float>(),
      .energy = doc["e2"].as<float>(),
      .energyReturned = doc["er2"].as<float>(),
      .frequency = doc["f"].as<float>(),
      .power = doc["p2"].as<float>(),
      .powerFactor = doc["pf2"].as<float>(),
      .voltage = doc["v2"].as<float>(),
    });

    udpMessageRateBuffer.add(millis() / 1000.0f);

    if (grid.updatePower()) {
      routingTask.resume();
    }
  });
});
