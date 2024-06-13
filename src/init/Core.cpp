// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

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
  dashboardTask.setManager(coreTaskManager);
  displayTask.setManager(coreTaskManager);
  ds18Task.setManager(coreTaskManager);
  lightsTask.setManager(coreTaskManager);
  networkManagerTask.setManager(coreTaskManager);
  networkServiceTask.setManager(coreTaskManager);
  otaTask.setManager(coreTaskManager);
  profilerTask.setManager(coreTaskManager);
  resetTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
  routerDebugTask.setManager(coreTaskManager);
#ifdef APP_MODEL_TRIAL
  trialTask.setManager(coreTaskManager);
#endif

  // pzemTaskManager
  pzemTask.setManager(pzemTaskManager);
  pzemO1PairingTask.setManager(pzemTaskManager);
  pzemO2PairingTask.setManager(pzemTaskManager);

  // routerTaskManager
  relayTask.setManager(routerTaskManager);
  routerTask.setManager(routerTaskManager);
  routingTask.setManager(routerTaskManager);

  // Router
  router.addOutput(output1);
  router.addOutput(output2);

  // HA
  haDiscovery.setPublisher([](const String& topic, const String& payload) {
    mqtt.publish(topic, payload, true);
  });
});
