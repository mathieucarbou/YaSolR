// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

#ifdef APP_MODEL_TRIAL
Mycila::Task* trialTask;
#endif

void yasolr_init_trial() {
#ifdef APP_MODEL_TRIAL
  logger.info(TAG, "Initialize trial...");

  Mycila::Trial.begin();
  Mycila::Trial.validate();

  trialTask = new Mycila::Task("Trial", [](void* params) { Mycila::Trial.validate(); });
  trialTask->setInterval(30 * Mycila::TaskDuration::SECONDS);
  trialTask->setManager(coreTaskManager);
#endif
}
