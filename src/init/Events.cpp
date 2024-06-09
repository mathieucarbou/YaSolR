// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

#define TAG "YASOLR"

Mycila::Task initEventsTask("Init Events", [](void* params) {
  logger.info(TAG, "Initializing Events...");

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

    } else if (key == KEY_ENABLE_OUTPUT1_AUTO_DIMMER) {
      output1.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT1_AUTO_DIMMER);

    } else if (key == KEY_OUTPUT1_DIMMER_LIMITER) {
      output1.config.dimmerLimit = config.get(KEY_OUTPUT1_DIMMER_LIMITER).toInt();

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

    } else if (key == KEY_ENABLE_OUTPUT2_AUTO_DIMMER) {
      output2.config.autoDimmer = config.getBool(KEY_ENABLE_OUTPUT2_AUTO_DIMMER);

    } else if (key == KEY_OUTPUT2_DIMMER_LIMITER) {
      output2.config.dimmerLimit = config.get(KEY_OUTPUT2_DIMMER_LIMITER).toInt();

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
    }

    YaSolR::Website.initCards();
    mqttPublishConfigTask.resume();
  });

  dashboard.onBeforeUpdate([](bool changes_only) {
    if (!changes_only) {
      logger.debug(TAG, "Dashboard refresh requested");
      YaSolR::Website.initCards();
    }
  });

  ESPConnect.listen([](ESPConnectState previous, ESPConnectState state) {
    logger.debug(TAG, "NetworkState: %s => %s", ESPConnect.getStateName(previous), ESPConnect.getStateName(state));
    switch (state) {
      case ESPConnectState::NETWORK_DISABLED:
        logger.warn(TAG, "Disabled Network!");
        break;
      case ESPConnectState::AP_STARTING:
        logger.info(TAG, "Starting Access Point %s...", ESPConnect.getAccessPointSSID().c_str());
        break;
      case ESPConnectState::AP_STARTED:
        logger.info(TAG, "Access Point %s started with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        networkServiceTask.resume();
        break;
      case ESPConnectState::NETWORK_CONNECTING:
        logger.info(TAG, "Connecting to network...");
        break;
      case ESPConnectState::NETWORK_CONNECTED:
        logger.info(TAG, "Connected with IP address %s", ESPConnect.getIPAddress().toString().c_str());
        networkServiceTask.resume();
        break;
      case ESPConnectState::NETWORK_TIMEOUT:
        logger.warn(TAG, "Unable to connect!");
        break;
      case ESPConnectState::NETWORK_DISCONNECTED:
        logger.warn(TAG, "Disconnected!");
        break;
      case ESPConnectState::NETWORK_RECONNECTING:
        logger.info(TAG, "Trying to reconnect...");
        break;
      case ESPConnectState::PORTAL_STARTING:
        logger.info(TAG, "Starting Captive Portal %s for %" PRIu32 " seconds...", ESPConnect.getAccessPointSSID().c_str(), ESPConnect.getCaptivePortalTimeout());
        break;
      case ESPConnectState::PORTAL_STARTED:
        logger.info(TAG, "Captive Portal started at %s with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        break;
      case ESPConnectState::PORTAL_COMPLETE: {
        if (ESPConnect.hasConfiguredAPMode()) {
          logger.info(TAG, "Captive Portal: Access Point configured");
          config.setBool(KEY_ENABLE_AP_MODE, true);
        } else {
          logger.info(TAG, "Captive Portal: WiFi configured");
          config.setBool(KEY_ENABLE_AP_MODE, false);
          config.set(KEY_WIFI_SSID, ESPConnect.getConfiguredWiFiSSID());
          config.set(KEY_WIFI_PASSWORD, ESPConnect.getConfiguredWiFiPassword());
        }
        break;
      }
      case ESPConnectState::PORTAL_TIMEOUT:
        logger.warn(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
  });

  ElegantOTA.onStart([]() { otaTask.resume(); });
  ElegantOTA.onEnd([](bool success) {
    if (success) {
      logger.info(TAG, "OTA Update Success! Restarting...");
    } else {
      logger.error(TAG, "OTA Failed! Restarting...");
    }
    restartTask.resume();
  });

  dimmerO1.listen([](Mycila::DimmerLevel event) {
    // mqttPublishTask.requestEarlyRun();
    switch (event) {
      case Mycila::DimmerLevel::OFF:
        logger.debug(TAG, "Output 1 Dimmer: turned off");
        break;
      case Mycila::DimmerLevel::FULL:
        logger.debug(TAG, "Output 1 Dimmer: at full power");
        break;
      case Mycila::DimmerLevel::DIM:
        logger.debug(TAG, "Output 1 Dimmer: dimming");
        break;
      default:
        assert(false);
        break;
    }
  });

  dimmerO2.listen([](Mycila::DimmerLevel event) {
    // mqttPublishTask.requestEarlyRun();
    switch (event) {
      case Mycila::DimmerLevel::OFF:
        logger.debug(TAG, "Output 2 dimmer: turned off");
        break;
      case Mycila::DimmerLevel::FULL:
        logger.debug(TAG, "Output 2 dimmer: at full power");
        break;
      case Mycila::DimmerLevel::DIM:
        logger.debug(TAG, "Output 2 dimmer: dimming");
        break;
      default:
        assert(false);
        break;
    }
  });

  bypassRelayO1.listen([](bool state) {
    logger.debug(TAG, "Output 1 Relay changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });
  bypassRelayO2.listen([](bool state) {
    logger.debug(TAG, "Output 2 Relay changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });
  relay1.listen([](bool state) {
    logger.debug(TAG, "Relay 1 changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });
  relay2.listen([](bool state) {
    logger.debug(TAG, "Relay 2 changed to %s", state ? "ON" : "OFF");
    mqttPublishTask.requestEarlyRun();
  });

  ds18Sys.listen([](float temperature) {
    logger.debug(TAG, "Router Temperature changed to %.02f °C", temperature);
    // mqttPublishTask.requestEarlyRun();
  });
  ds18O1.listen([](float temperature) {
    logger.debug(TAG, "Output 1 Temperature changed to %.02f °C", temperature);
    // mqttPublishTask.requestEarlyRun();
  });
  ds18O2.listen([](float temperature) {
    logger.debug(TAG, "Output 2 Temperature changed to %.02f °C", temperature);
    // mqttPublishTask.requestEarlyRun();
  });

  const bool jsyPow = config.get(KEY_GRID_POWER_MQTT_TOPIC).isEmpty();
  jsy.setCallback([jsyPow](const Mycila::JSYEventType eventType) {
    if (eventType == Mycila::JSYEventType::EVT_READ) {
      routingTask.resume();
    }
  });
});
