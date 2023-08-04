// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaHTTPd.h>
#include <MycilaLogger.h>
#include <MycilaOTA.h>

#include <ArduinoOTA.h>
#include <ESPmDNS.h>

#ifdef APP_VERSION_PRO
#include <ElegantOTAPro.h>
#else
#include <ElegantOTA.h>
#endif

#define TAG "OTA"

void Mycila::OTAClass::begin() {
  if (_enabled)
    return;

  const String hostname = OTAConfig.getHostname();

  if (hostname.isEmpty())
    return;

  Logger.info(TAG, "Enable OTA...");

  const String password = OTAConfig.getPassword();

  Logger.debug(TAG, "- ota://%s:%u", hostname.c_str(), MYCILA_OTA_PORT);
  ArduinoOTA.setHostname(hostname.c_str());
  ArduinoOTA.setPort(MYCILA_OTA_PORT);
  if (!password.isEmpty())
    ArduinoOTA.setPassword(password.c_str());
  ArduinoOTA.setRebootOnSuccess(false);
  ArduinoOTA.onStart([this]() {
    if (_prepareCallback)
      _prepareCallback();
  });
  ArduinoOTA.onEnd([this]() {
    if (_completeCallback)
      _completeCallback(true);
  });
  ArduinoOTA.onError([this](ota_error_t error) {
    if (_completeCallback)
      _completeCallback(false);
  });
  ArduinoOTA.setMdnsEnabled(true);
  ArduinoOTA.begin();
  ArduinoOTA.setMdnsEnabled(false); // so that end() does not end MDNS

  Logger.debug(TAG, "- http://%s/update", (hostname + (hostname.indexOf(".") >= 0 ? "" : ".local")).c_str());
  if (!_elegantOTAInitialized) {
#ifdef APP_VERSION_PRO
    ElegantOTA.setID(OTAConfig.getID().c_str());
    ElegantOTA.setTitle(OTAConfig.getTitle().c_str());
    ElegantOTA.setFWVersion(OTAConfig.getFWVersion().c_str());
    ElegantOTA.setFirmwareMode(true);
    ElegantOTA.setFilesystemMode(false);
#endif
    ElegantOTA.setAutoReboot(false);
    ElegantOTA.setAuth(OTAConfig.getUsername().c_str(), OTAConfig.getPassword().c_str());
    ElegantOTA.onStart([this]() {
      if (_prepareCallback)
        _prepareCallback();
    });
    ElegantOTA.onEnd([this](bool success) {
      if (_completeCallback)
        _completeCallback(success);
    });
    ElegantOTA.begin(&HTTPd.server, OTAConfig.getUsername().c_str(), OTAConfig.getPassword().c_str());
    _elegantOTAInitialized = true;
  }

  _enabled = true;
}

void Mycila::OTAClass::loop() {
  if (_enabled) {
    ArduinoOTA.handle();
#ifdef APP_VERSION_PRO
    ElegantOTA.loop();
#endif
  }
}

void Mycila::OTAClass::end() {
  if (!_enabled)
    return;
  Logger.info(TAG, "Disable OTA...");
  MDNS.disableArduino();
  ArduinoOTA.end();
  _enabled = false;
}

namespace Mycila {
  OTAClass OTA;
  OTAConfigClass OTAConfig;
} // namespace Mycila
