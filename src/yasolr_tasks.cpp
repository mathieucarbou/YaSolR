// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

void yasolr_start_task_manager() {
  LOGI(TAG, "Start task managers");

  // core task manager
  assert(coreTaskManager.asyncStart(4096, 5, 1, 100, true));

  // task manager for long running tasks like mqtt / pzem
  if (unsafeTaskManager.tasks())
    assert(unsafeTaskManager.asyncStart(4096, 1, 1, 100, true));
}
