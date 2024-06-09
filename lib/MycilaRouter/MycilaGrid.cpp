// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaGrid.h>
#include <thyristor.h>

float Mycila::Grid::getFrequency() const { return _frequency > 0 ? _frequency : Thyristor::getDetectedFrequency(); }
