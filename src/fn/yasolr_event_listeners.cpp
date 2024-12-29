// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>
#include <YaSolRWebsite.h>

#include <string>

void yasolr_event_listeners() {
  logger.info(TAG, "Initializing Event Listeners");

  bypassRelayO1.listen([](bool state) {
    logger.info(TAG, "Output 1 Relay changed to %s", state ? "ON" : "OFF");
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });
  bypassRelayO2.listen([](bool state) {
    logger.info(TAG, "Output 2 Relay changed to %s", state ? "ON" : "OFF");
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });
  relay1.listen([](bool state) {
    logger.info(TAG, "Relay 1 changed to %s", state ? "ON" : "OFF");
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });
  relay2.listen([](bool state) {
    logger.info(TAG, "Relay 2 changed to %s", state ? "ON" : "OFF");
    if (mqttPublishTask)
      mqttPublishTask->requestEarlyRun();
  });
}
