// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>

#ifdef APP_MODEL_TRIAL
Mycila::Task trialTask("Trial", [](void* params) { Mycila::Trial.validate(); });
#endif
