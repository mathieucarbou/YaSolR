// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

void yasolr_init_tasks() {
  logger.info(TAG, "Initialize tasks");

  // profiling tasks ?
  if (config.getBool(KEY_ENABLE_DEBUG)) {
    coreTaskManager.enableProfiling();
    unsafeTaskManager.enableProfiling();
  }

  // core task manager
  assert(coreTaskManager.asyncStart(512 * 8, 5, 1, 100, true));

  // task manager for long running tasks like mqtt / pzem
  if (unsafeTaskManager.tasks())
    assert(unsafeTaskManager.asyncStart(512 * 8, 1, 1, 100, false));
}
