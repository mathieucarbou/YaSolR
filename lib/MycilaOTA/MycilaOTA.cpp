// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaHTTPd.h>
#include <MycilaLogger.h>
#include <MycilaOTA.h>

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <assert.h>

#ifdef APP_VERSION_PRO
#include <ElegantOTAPro.h>
#else
#include <ElegantOTA.h>
#endif

#define TAG "OTA"

void Mycila::OTAClass::begin(bool async) {
  if (_enabled)
    return;

  const String hostname = OTAConfig.getHostname();

  if (hostname.isEmpty())
    return;

  Logger.info(TAG, "Enable OTA...");

  const String password = OTAConfig.getPassword();

  Logger.debug(TAG, "- ota://%s.local:%u", hostname.c_str(), MYCILA_OTA_PORT);
  ArduinoOTA.setHostname(hostname.c_str());
  ArduinoOTA.setPort(MYCILA_OTA_PORT);
  if (!password.isEmpty())
    ArduinoOTA.setPassword(password.c_str());
  ArduinoOTA.setRebootOnSuccess(false);
  ArduinoOTA.onStart([this]() {
    _updating = true;
    if (_prepareCallback)
      _prepareCallback();
  });
  ArduinoOTA.onEnd([this]() {
    _updating = false;
    if (_completeCallback)
      _completeCallback(true);
  });
  ArduinoOTA.onError([this](ota_error_t error) {
    _updating = false;
    if (_completeCallback)
      _completeCallback(false);
  });
  ArduinoOTA.setMdnsEnabled(true);
  ArduinoOTA.begin();
  ArduinoOTA.setMdnsEnabled(false); // so that end() does not end MDNS

  Logger.debug(TAG, "- http://%s.local/update", hostname.c_str());
  if (!_elegantOTAInitialized) {
#ifdef APP_VERSION_PRO
    ElegantOTA.setID(OTAConfig.getFirmwareName().c_str());
    ElegantOTA.setTitle(OTAConfig.getTitle().c_str());
    ElegantOTA.setFWVersion(OTAConfig.getFirmwareVersion().c_str());
    ElegantOTA.setFirmwareMode(true);
    ElegantOTA.setFilesystemMode(false);
#endif
    ElegantOTA.setAutoReboot(false);
    ElegantOTA.setAuth(OTAConfig.getUsername().c_str(), OTAConfig.getPassword().c_str());
    ElegantOTA.onStart([this]() {
      _updating = true;
      if (_prepareCallback)
        _prepareCallback();
    });
    ElegantOTA.onEnd([this](bool success) {
      _updating = false;
      if (_completeCallback)
        _completeCallback(success);
    });
    ElegantOTA.begin(&HTTPd.server, OTAConfig.getUsername().c_str(), OTAConfig.getPassword().c_str());
    _elegantOTAInitialized = true;
  }

  _enabled = true;

  if (async)
    assert(xTaskCreateUniversal(_otaTask, "otaTask", 256 * 9, this, 1, nullptr, xPortGetCoreID()) == pdPASS);
}

void Mycila::OTAClass::loop() {
  if (!_enabled)
    return;
  if (_enabled)
    ArduinoOTA.handle();
}

void Mycila::OTAClass::_otaTask(void* params) {
  Mycila::OTAClass* instance = reinterpret_cast<Mycila::OTAClass*>(params);
  while (instance->isEnabled()) {
    instance->loop();
    yield();
  }
  vTaskDelete(nullptr);
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
