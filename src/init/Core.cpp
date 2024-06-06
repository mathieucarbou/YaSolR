// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

#define TAG "YASOLR"

Mycila::Task initCoreTask("Init Core", [](void* params) {
  logger.warn(TAG, "Initializing %s...", Mycila::AppInfo.nameModelVersion.c_str());

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

  // pzemO1TaskManager
  pzemO1Task.setManager(pzemO1TaskManager);
  pzemO1PairingTask.setManager(pzemO1TaskManager);

  // pzemO2TaskManager
  pzemO2Task.setManager(pzemO2TaskManager);
  pzemO2PairingTask.setManager(pzemO2TaskManager);

  // routerTaskManager
  relaysTask.setManager(routerTaskManager);
  routerTask.setManager(routerTaskManager);

  // Relay manager
  Mycila::RelayManager.addRelay("relay1", relay1);
  Mycila::RelayManager.addRelay("relay2", relay2);

  // Grid
  Mycila::Grid.setJSY(jsy);
  Mycila::Grid.setMQTT(mqtt);

  // Router
  Mycila::Router.setJSY(jsy);
  Mycila::Router.addOutput(output1);
  Mycila::Router.addOutput(output2);

  // HA
  haDiscovery.setPublisher([](const String& topic, const String& payload) {
    mqtt.publish(topic, payload, true);
  });
});
