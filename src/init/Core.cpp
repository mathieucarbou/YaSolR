// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task initCoreTask("Init Core", [](void* params) {
  logger.warn(TAG, "Initializing %s", Mycila::AppInfo.nameModelVersion.c_str());

  // WDT
  Mycila::TaskManager::configureWDT(10, true);

  // coreTaskManager
  calibrationTask.setManager(coreTaskManager);
  carouselTask.setManager(coreTaskManager);
  dashboardTask.setManager(coreTaskManager);
  debugTask.setManager(coreTaskManager);
  displayTask.setManager(coreTaskManager);
  lightsTask.setManager(coreTaskManager);
  safeBootTask.setManager(coreTaskManager);
  relayTask.setManager(coreTaskManager);
  resetTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
  routerTask.setManager(coreTaskManager);
  zcdTask.setManager(coreTaskManager);
#ifdef APP_MODEL_TRIAL
  trialTask.setManager(coreTaskManager);
#endif

  // mqttTaskManager
  haDiscoveryTask.setManager(mqttTaskManager);
  mqttConfigTask.setManager(mqttTaskManager);
  mqttPublishTask.setManager(mqttTaskManager);
  mqttPublishStaticTask.setManager(mqttTaskManager);
  mqttPublishConfigTask.setManager(mqttTaskManager);

  // pioTaskManager
  ds18Task.setManager(pioTaskManager);
  networkManagerTask.setManager(pioTaskManager);
  networkConfigTask.setManager(pioTaskManager);
  pzemO1PairingTask.setManager(pioTaskManager);
  pzemO2PairingTask.setManager(pioTaskManager);

  // jsyTaskManager
  jsyTask.setManager(jsyTaskManager);

  // pzemTaskManager
  pzemTask.setManager(pzemTaskManager);

  // routingTaskManager
  routingTask.setManager(routingTaskManager);

  // Router
  router.addOutput(output1);
  router.addOutput(output2);

  // Task Monitor
  Mycila::TaskMonitor.begin();
  // Mycila::TaskMonitor.addTask("arduino_events");            // Network (stack size cannot be set)
  // Mycila::TaskMonitor.addTask("async_udp");                 // AsyncUDP (stack size cannot be set)
  // Mycila::TaskMonitor.addTask("wifi");                      // WiFI (stack size cannot be set)
  Mycila::TaskMonitor.addTask("mqtt_task");                  // MQTT (set stack size with MYCILA_MQTT_STACK_SIZE)
  Mycila::TaskMonitor.addTask("async_tcp");                  // AsyncTCP (set stack size with CONFIG_ASYNC_TCP_STACK_SIZE)
  Mycila::TaskMonitor.addTask(pioTaskManager.getName());     // YaSolR
  Mycila::TaskMonitor.addTask(coreTaskManager.getName());    // YaSolR
  Mycila::TaskMonitor.addTask(mqttTaskManager.getName());    // YaSolR
  Mycila::TaskMonitor.addTask(routingTaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(jsyTaskManager.getName());     // YaSolR
  Mycila::TaskMonitor.addTask(pzemTaskManager.getName());    // YaSolR

  // HA
  haDiscovery.setPublisher([](const String& topic, const String& payload) {
    mqtt.publish(topic, payload, true);
  });
});
