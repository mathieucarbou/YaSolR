// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

Mycila::Task pzemO1Task("PZEM 0x01", [](void* params) { pzemO1.read(); });
Mycila::Task pzemO2Task("PZEM 0x02", [](void* params) { pzemO2.read(); });

Mycila::Task pzemO1PairingTask("PZEM 0x01 Pairing", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Pairing connected PZEM to Output 1...");
  pzemO1.end();
  pzemO2.end();
  pzemO1.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt());
  switch (pzemO1.readAddress()) {
    case MYCILA_PZEM_INVALID_ADDRESS:
      logger.error(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is connected to Serial RX/TX and powered from Grid");
      pzemO1.end();
      break;
    case YASOLR_PZEM_ADDRESS_OUTPUT1:
      logger.info(TAG, "PZEM already paired to Output 1");
      pzemO1.end();
      if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM))
        pzemO1.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT1);
      break;
    default:
      if (pzemO1.setAddress(YASOLR_PZEM_ADDRESS_OUTPUT1)) {
        logger.info(TAG, "PZEM has been paired to Output 1");
        pzemO1.end();
        if (config.getBool(KEY_ENABLE_OUTPUT1_PZEM))
          pzemO1.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT1);
      } else {
        logger.error(TAG, "Failed to pair PZEM to Output 1: make sure only PZEM of Output 1 is connected to Serial RX/TX and powered from Grid");
        pzemO1.end();
      }
      break;
  }
});

Mycila::Task pzemO2PairingTask("PZEM 0x02 Pairing", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Pairing connected PZEM to Output 2...");
  pzemO1.end();
  pzemO2.end();
  pzemO2.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt());
  switch (pzemO2.readAddress()) {
    case MYCILA_PZEM_INVALID_ADDRESS:
      logger.error(TAG, "Failed to pair PZEM to Output 2: make sure only PZEM of Output 2 is connected to Serial RX/TX and powered from Grid");
      pzemO2.end();
      break;
    case YASOLR_PZEM_ADDRESS_OUTPUT2:
      logger.info(TAG, "PZEM already paired to Output 2");
      pzemO2.end();
      if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM))
        pzemO2.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT2);
      break;
    default:
      if (pzemO2.setAddress(YASOLR_PZEM_ADDRESS_OUTPUT2)) {
        logger.info(TAG, "PZEM has been paired to Output 2");
        pzemO2.end();
        if (config.getBool(KEY_ENABLE_OUTPUT2_PZEM))
          pzemO2.begin(YASOLR_PZEM_SERIAL, config.get(KEY_PIN_PZEM_RX).toInt(), config.get(KEY_PIN_PZEM_TX).toInt(), YASOLR_PZEM_ADDRESS_OUTPUT2);
      } else {
        logger.error(TAG, "Failed to pair PZEM to Output 2: make sure only PZEM of Output 2 is connected to Serial RX/TX and powered from Grid");
        pzemO2.end();
      }
      break;
  }
});
