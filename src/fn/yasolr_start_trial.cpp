// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#ifdef APP_MODEL_TRIAL
Mycila::Task* trialTask;
#endif

void yasolr_start_trial() {
#ifdef APP_MODEL_TRIAL
  Mycila::Trial.begin();
  Mycila::Trial.validate();

  trialTask = new Mycila::Task("Trial", [](void* params) { Mycila::Trial.validate(); });
  trialTask->setInterval(30 * Mycila::TaskDuration::SECONDS);
  trialTask->setManager(coreTaskManager);
#endif
}
