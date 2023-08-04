// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaTrial.h>

#include <MycilaLogger.h>

#include <Preferences.h>
#include <esp32-hal-timer.h>

#define TAG "TRIAL"

#ifndef MYCILA_TRIAL_DURATION
#define MYCILA_TRIAL_DURATION 86400 // 1 day in seconds
#endif

#ifndef MYCILA_TRIAL_SAVE
#define MYCILA_TRIAL_SAVE 60
#endif

void Mycila::TrialClass::begin() {
  Preferences prefs;
  prefs.begin(TAG, true);
  const uint32_t duration = prefs.getLong("duration", false);
  prefs.end();
  if (duration > MYCILA_TRIAL_DURATION) {
    Logger.error(TAG, "Trial expired");
    while (true) {
      delay(1000);
    }
  }
  _lastSave = millis();
}

void Mycila::TrialClass::loop() {
  const uint32_t uptime = millis();
  if (uptime - _lastSave >= MYCILA_TRIAL_SAVE * 1000) {
    Preferences prefs;
    prefs.begin(TAG, false);
    const uint32_t duration = prefs.getLong("duration", 0) + (uptime - _lastSave) / 1000;
    prefs.putLong("duration", duration);
    prefs.end();
    _lastSave = uptime;
    if (duration >= MYCILA_TRIAL_DURATION) {
      Logger.error(TAG, "Trial expired");
      while (true) {
        delay(1000);
      }
    }
    Logger.warn(TAG, "%lu seconds left for trial", duration > MYCILA_TRIAL_DURATION ? 0 : MYCILA_TRIAL_DURATION - duration);
  }
}

namespace Mycila {
  TrialClass Trial;
} // namespace Mycila
