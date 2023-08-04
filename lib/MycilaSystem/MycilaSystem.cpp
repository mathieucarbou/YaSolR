// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaLogger.h>
#include <MycilaSystem.h>

#include <LittleFS.h>
#include <Preferences.h>
#include <nvs_flash.h>

#define TAG "SYSTEM"

#define KEY_BOOTS "boots"
#define KEY_RESETS "resets"

#ifndef MYCILA_NVM_RESET_BOOT_DELAY
#define MYCILA_NVM_RESET_BOOT_DELAY 3
#endif

void Mycila::SystemClass::begin() {
  Logger.info(TAG, "Initializing File System...");
  nvs_flash_init();
  if (LittleFS.begin(false))
    Logger.debug(TAG, "File System initialized");
  else {
    Logger.error(TAG, "Failed. Trying to format...");
    if (LittleFS.begin(true))
      Logger.info(TAG, "Successfully formatted and initialized");
    else
      Logger.error(TAG, "Failed to format");
  }

  Preferences prefs;
  prefs.begin(TAG, false);

  _boots = (prefs.isKey(KEY_BOOTS) ? prefs.getULong(KEY_BOOTS, 0) : 0) + 1;
  prefs.putULong(KEY_BOOTS, _boots);
  Logger.debug(TAG, "Booted %llu times", _boots);

#ifdef MYCILA_BOOT_WAIT_FOR_RESET
  const int count = (prefs.isKey(KEY_RESETS) ? prefs.getInt(KEY_RESETS, 0) : 0) + 1;
  prefs.putInt(KEY_RESETS, count);
  if (count >= 3) {
    prefs.end();
    reset();
  } else {
    Logger.warn(TAG, "WAITING FOR HARD RESET...");
    for (uint32_t d = 0; d < MYCILA_NVM_RESET_BOOT_DELAY * 1000UL; d += 500) {
      delay(500);
    }
    prefs.remove(KEY_RESETS);
    Logger.debug(TAG, "No hard reset");
  }
#endif

  prefs.end();
}

void Mycila::SystemClass::reset() {
  Logger.debug(TAG, "Triggering System Reset...");
  nvs_flash_erase();
  nvs_flash_init();
  esp_restart();
}

void Mycila::SystemClass::restart(uint32_t delayMillisBeforeRestart) {
  Logger.debug(TAG, "Triggering System Restart...");
  if (delayMillisBeforeRestart == 0)
    esp_restart();
  else {
    _delayedTask.once_ms(delayMillisBeforeRestart, esp_restart);
  }
}

const Mycila::SystemMemory Mycila::SystemClass::getMemory() const {
  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
  return {
    .total = info.total_free_bytes + info.total_allocated_bytes,
    .used = info.total_allocated_bytes,
    .free = info.total_free_bytes,
    .usage = round(static_cast<float>(info.total_allocated_bytes) / static_cast<float>(info.total_free_bytes + info.total_allocated_bytes) * 10000) / 100};
}

void Mycila::SystemClass::toJson(const JsonObject& root) const {
  SystemMemory memory = getMemory();
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  root["boots"] = _boots;
  root["chip_cores"] = chip_info.cores;
  root["chip_model"] = ESP.getChipModel();
  root["chip_revision"] = chip_info.revision;
  root["cpu_freq"] = ESP.getCpuFreqMHz();
  root["heap_total"] = memory.total;
  root["heap_usage"] = memory.usage;
  root["heap_used"] = memory.used;
  root["uptime"] = getUptime();
}

namespace Mycila {
  SystemClass System;
} // namespace Mycila
