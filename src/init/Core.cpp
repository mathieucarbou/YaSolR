// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task initCoreTask("Init Core", [](void* params) {
  logger.warn(TAG, "Initializing %s...", Mycila::AppInfo.nameModelVersion.c_str());

  // WDT
  Mycila::TaskManager::configureWDT(10, true);

  // ioTaskManager
  haDiscoveryTask.setManager(ioTaskManager);
  mqttPublishTask.setManager(ioTaskManager);
  mqttPublishStaticTask.setManager(ioTaskManager);
  mqttPublishConfigTask.setManager(ioTaskManager);

  // jsyTaskManager
  jsyTask.setManager(jsyTaskManager);

  // core0TaskManager
  carouselTask.setManager(core0TaskManager);
  displayTask.setManager(core0TaskManager);
  ds18Task.setManager(core0TaskManager);
  lightsTask.setManager(core0TaskManager);

  // core1TaskManager
  dashboardTask.setManager(core1TaskManager);
  mqttConfigTask.setManager(core1TaskManager);
  networkManagerTask.setManager(core1TaskManager);
  networkUpTask.setManager(core1TaskManager);
  otaTask.setManager(core1TaskManager);
  resetTask.setManager(core1TaskManager);
  restartTask.setManager(core1TaskManager);
#ifdef APP_MODEL_TRIAL
  trialTask.setManager(core1TaskManager);
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

  // Task Monitor
  Mycila::TaskMonitor.begin();
  Mycila::TaskMonitor.addTask("arduino_events");            // Network
  Mycila::TaskMonitor.addTask("async_tcp");                 // AsyncTCP
  Mycila::TaskMonitor.addTask("async_udp");                 // AsyncUDP
  Mycila::TaskMonitor.addTask("mqtt_task");                 // MQTT
  Mycila::TaskMonitor.addTask("wifi");                      // WiFI
  Mycila::TaskMonitor.addTask(core0TaskManager.getName());  // YaSolR
  Mycila::TaskMonitor.addTask(core1TaskManager.getName());  // YaSolR
  Mycila::TaskMonitor.addTask(ioTaskManager.getName());     // YaSolR
  Mycila::TaskMonitor.addTask(routerTaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(jsyTaskManager.getName());    // YaSolR
  Mycila::TaskMonitor.addTask(pzemTaskManager.getName());   // YaSolR

  // HA
  haDiscovery.setPublisher([](const String& topic, const String& payload) {
    mqtt.publish(topic, payload, true);
  });
});
