// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

void yasolr_boot() {
  Serial.begin(YASOLR_SERIAL_BAUDRATE);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  // early logging
  logger.forwardTo(&Serial);
  logger.info(TAG, "Booting %s", Mycila::AppInfo.nameModelVersion.c_str());

  // system
  Mycila::System::init(true, "fs");

  // trial
#ifdef APP_MODEL_TRIAL
  Mycila::Trial.begin();
  Mycila::Trial.validate();
#endif
};
