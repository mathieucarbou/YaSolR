// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <YaSolR.h>


Mycila::Task networkManagerTask("ESPConnect", [](void* params) { espConnect.loop(); });