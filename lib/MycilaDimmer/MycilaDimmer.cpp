// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <MycilaDimmer.h>

#define TABLE_PHASE_LEN    (80U)
#define DIMMER_RESOLUTION  12

static const uint32_t DIMMER_MAX = (1 << DIMMER_RESOLUTION) - 1;
static const uint32_t TABLE_PHASE_SCALE = (TABLE_PHASE_LEN - 1U) * (1UL << (16 - DIMMER_RESOLUTION));
static const uint16_t TABLE_PHASE_DELAY[TABLE_PHASE_LEN] PROGMEM = {0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094, 0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2, 0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9, 0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8, 0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf, 0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2, 0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606, 0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686, 0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296, 0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016};

// =============================================================================
// Mycila::Dimmer
// =============================================================================

uint16_t Mycila::Dimmer::_lookupFiringDelay(float dutyCycle) {
  uint32_t duty = dutyCycle * DIMMER_MAX;
  uint32_t slot = duty * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 1);
  uint32_t index = slot >> 16;
  uint32_t a = TABLE_PHASE_DELAY[index];
  uint32_t b = TABLE_PHASE_DELAY[index + 1];
  uint32_t delay = a - (((a - b) * (slot & 0xffff)) >> 16); // interpolate a b
  return (delay * _semiPeriod) >> 16; // scale to period
}
