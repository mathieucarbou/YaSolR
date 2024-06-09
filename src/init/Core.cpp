// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

Mycila::Task initCoreTask("Init Core", [](void* params) {
  logger.warn(TAG, "Initializing %s...", Mycila::AppInfo.nameModelVersion.c_str());

  // WDT
  Mycila::TaskManager::configureWDT();

  // ioTaskManager
  haDiscoveryTask.setManager(ioTaskManager);
  mqttPublishTask.setManager(ioTaskManager);
  mqttPublishStaticTask.setManager(ioTaskManager);
  mqttPublishConfigTask.setManager(ioTaskManager);

  // jsyTaskManager
  jsyTask.setManager(jsyTaskManager);

  // coreTaskManager
  carouselTask.setManager(coreTaskManager);
  displayTask.setManager(coreTaskManager);
  ds18Task.setManager(coreTaskManager);
  lightsTask.setManager(coreTaskManager);
  networkManagerTask.setManager(coreTaskManager);
  networkServiceTask.setManager(coreTaskManager);
  otaTask.setManager(coreTaskManager);
  profilerTask.setManager(coreTaskManager);
  resetTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
  dashboardTask.setManager(coreTaskManager);
#ifdef APP_MODEL_TRIAL
  trialTask.setManager(coreTaskManager);
#endif

  // pzemTaskManager
  pzemTask.setManager(pzemTaskManager);
  pzemO1PairingTask.setManager(pzemTaskManager);
  pzemO2PairingTask.setManager(pzemTaskManager);

  // routerTaskManager
  relaysTask.setManager(routerTaskManager);
  routerTask.setManager(routerTaskManager);

  // Relay manager
  Mycila::RelayManager.addRelay("relay1", relay1);
  Mycila::RelayManager.addRelay("relay2", relay2);

  // Router
  Mycila::Router.setJSY(jsy);
  Mycila::Router.addOutput(output1);
  Mycila::Router.addOutput(output2);

  // HA
  haDiscovery.setPublisher([](const String& topic, const String& payload) {
    mqtt.publish(topic, payload, true);
  });
});
