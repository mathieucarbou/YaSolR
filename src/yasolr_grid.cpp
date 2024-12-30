// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Grid grid;

float yasolr_frequency() {
  float frequency = round(grid.getFrequency().value_or(NAN));
  if (frequency > 0)
    return frequency;

  if (pzemO1) {
    frequency = round(pzemO1->data.frequency);
    if (frequency > 0)
      return frequency;
  }

  if (pzemO2) {
    frequency = round(pzemO2->data.frequency);
    if (frequency > 0)
      return frequency;
  }

  frequency = config.getFloat(KEY_GRID_FREQUENCY);
  if (frequency)
    return frequency;

  if (pulseAnalyzer) {
    frequency = pulseAnalyzer->getNominalGridFrequency();
    if (frequency)
      return frequency;
  }

  return 0;
}

void yasolr_init_grid() {
  logger.info(TAG, "Initialize grid electricity...");

  grid.localMetrics().setExpiration(10000);                             // local is fast
  grid.remoteMetrics().setExpiration(10000);                            // remote JSY is fast
  grid.pzemMetrics().setExpiration(10000);                              // local is fast
  grid.mqttPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);   // through mqtt
  grid.mqttVoltage().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // through mqtt
  grid.getPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);    // local is fast
}
