// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task pzemO1PairingTask("PZEM Pairing 0x01", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Pairing connected PZEM to Output 1...");
  pzemO1.end();
  pzemO1.begin(YASOLR_PZEM_SERIAL, config.getLong(KEY_PIN_PZEM_RX), config.getLong(KEY_PIN_PZEM_TX), MYCILA_PZEM_ADDRESS_GENERAL);
  switch (pzemO1.getDeviceAddress()) {
    case YASOLR_PZEM_ADDRESS_OUTPUT1:
      // already paired
      if (!config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
        // stop PZEM if it was not enabled
        pzemO1.end();
      }
      logger.warn(TAG, "PZEM already paired to Output 1");
      break;
    case MYCILA_PZEM_ADDRESS_UNKNOWN:
      // no device found
      pzemO1.end();
      logger.error(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is powered and connected to Serial RX/TX!");
      break;
    default:
      // found a device
      if (pzemO1.setDeviceAddress(YASOLR_PZEM_ADDRESS_OUTPUT1)) {
        if (!config.getBool(KEY_ENABLE_OUTPUT1_PZEM)) {
          // stop PZEM if it was not enabled
          pzemO1.end();
        }
        logger.info(TAG, "PZEM has been paired to Output 1");
      } else {
        pzemO1.end();
        logger.error(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is powered and connected to Serial RX/TX!");
      }
  }
});