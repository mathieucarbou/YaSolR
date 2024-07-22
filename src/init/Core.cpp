// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task initCoreTask("Init Core", [](void* params) {
  logger.warn(TAG, "Initializing %s...", Mycila::AppInfo.nameModelVersion.c_str());

  // WDT
  Mycila::TaskManager::configureWDT(10, true);

  // pioTaskManager
  carouselTask.setManager(pioTaskManager);
  displayTask.setManager(pioTaskManager);
  ds18Task.setManager(pioTaskManager);
  lightsTask.setManager(pioTaskManager);

  // coreTaskManager
  dashboardTask.setManager(coreTaskManager);
  debugTask.setManager(coreTaskManager);
  networkManagerTask.setManager(coreTaskManager);
  networkConfigTask.setManager(coreTaskManager);
  otaTask.setManager(coreTaskManager);
  resetTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
#ifdef APP_MODEL_TRIAL
  trialTask.setManager(coreTaskManager);
#endif

  // mqttTaskManager
  haDiscoveryTask.setManager(mqttTaskManager);
  mqttConfigTask.setManager(mqttTaskManager);
  mqttPublishTask.setManager(mqttTaskManager);
  mqttPublishStaticTask.setManager(mqttTaskManager);
  mqttPublishConfigTask.setManager(mqttTaskManager);

  // jsyTaskManager
  jsyTask.setManager(jsyTaskManager);

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

  // Task Monitor
  Mycila::TaskMonitor.begin();
  // Mycila::TaskMonitor.addTask("arduino_events");            // Network
  Mycila::TaskMonitor.addTask("async_tcp");                 // AsyncTCP
  // Mycila::TaskMonitor.addTask("async_udp");                 // AsyncUDP
  Mycila::TaskMonitor.addTask("mqtt_task");                 // MQTT
  // Mycila::TaskMonitor.addTask("wifi");                      // WiFI
  Mycila::TaskMonitor.addTask(pioTaskManager.getName());  // YaSolR
  Mycila::TaskMonitor.addTask(coreTaskManager.getName());  // YaSolR
  Mycila::TaskMonitor.addTask(mqttTaskManager.getName());     // YaSolR
  Mycila::TaskMonitor.addTask(routerTaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(jsyTaskManager.getName());    // YaSolR
  Mycila::TaskMonitor.addTask(pzemTaskManager.getName());   // YaSolR

  // HA
  haDiscovery.setPublisher([](const String& topic, const String& payload) {
    mqtt.publish(topic, payload, true);
  });
});
