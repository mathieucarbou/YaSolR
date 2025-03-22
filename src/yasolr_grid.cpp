// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#include <yasolr.h>

Mycila::Grid grid;

float yasolr_frequency() {
  // 1. check if frequency is set in config
  float frequency = config.getFloat(KEY_GRID_FREQUENCY);
  if (frequency)
    return frequency;

  // 2. check if frequency is set from a measurement devices (PZEM, JSY, ...)
  frequency = std::round(grid.getFrequency().value_or(NAN));
  if (frequency > 0)
    return frequency;

  // 3. check if frequency is set in pulse analyzer
  if (pulseAnalyzer) {
    frequency = pulseAnalyzer->getNominalGridFrequency();
    if (frequency)
      return frequency;
  }

  
  // 4. frequency from modbbus
  if (MB) { 
    return modbus_frequency;
  }


  return 0;
}

void yasolr_init_grid() {
  logger.info(TAG, "Initialize grid electricity");

  grid.localMetrics().setExpiration(10000);                             // local is fast
  grid.remoteMetrics().setExpiration(10000);                            // remote JSY is fast
  grid.pzemMetrics().setExpiration(10000);                              // local is fast
  grid.mqttPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);   // through mqtt
  grid.mqttVoltage().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION); // through mqtt
  grid.getPower().setExpiration(YASOLR_MQTT_MEASUREMENT_EXPIRATION);    // local is fast
}
