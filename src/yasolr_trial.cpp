// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

void yasolr_init_trial() {
#ifdef APP_MODEL_TRIAL
  LOGI(TAG, "Initialize trial");

  Mycila::Trial.begin();
  Mycila::Trial.validate();

  Mycila::Task* trialTask = new Mycila::Task("Trial", [](void* params) { Mycila::Trial.validate(); });
  trialTask->setInterval(30000);
  coreTaskManager.addTask(*trialTask);
#endif
}
