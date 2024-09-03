// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

Mycila::Task safeBootTask("SafeBoot", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Restarting %s in SafeBoot mode...", Mycila::AppInfo.nameModelVersion.c_str());
  // stop electricity
  dimmerO1.endDimmer();
  dimmerO2.endDimmer();
  bypassRelayO1.end();
  bypassRelayO2.end();
  relay1.end();
  relay2.end();
  zcd.end();
  // stop blocking I/O tasks
  mqtt.end();
  udp.close();
#ifdef APP_MODEL_TRIAL
  Mycila::Trial.end();
#endif
  Mycila::System::restartFactory("safeboot");
});
