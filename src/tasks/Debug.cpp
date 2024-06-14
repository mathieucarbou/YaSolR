// SPDX-License-Identifier: %GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <YaSolR.h>

Mycila::Task profilerTask("Profiler", [](void* params) {
  Mycila::TaskMonitor.log();
  coreTaskManager.log();
  routerTaskManager.log();
  jsyTaskManager.log();
  pzemTaskManager.log();
  ioTaskManager.log();
});

Mycila::Task routerDebugTask("Router Debug", [](void* params) {
  Mycila::GridMetrics gridMetrics;
  grid.getMetrics(gridMetrics);

  Mycila::RouterMetrics routerMetrics;
  router.getMetrics(routerMetrics);

  logger.info(TAG, "[ JSY CH%d] U:%7.2f V | I: %5.2f A | R: %5.1f Ω | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f | F: %4.1f Hz |", 1, jsy.getVoltage1(), jsy.getCurrent1(), jsy.getResistance1() > 1000 ? 0 : jsy.getResistance1(), jsy.getPower1(), jsy.getApparentPower1(), jsy.getPowerFactor1(), jsy.getTHDi1(0), jsy.getFrequency());
  logger.info(TAG, "[ JSY CH%d] U:%7.2f V | I: %5.2f A | R: %5.1f Ω | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f | F: %4.1f Hz |", 2, jsy.getVoltage2(), jsy.getCurrent2(), jsy.getResistance2(), jsy.getPower2(), jsy.getApparentPower2(), jsy.getPowerFactor2(), jsy.getTHDi2(0), jsy.getFrequency());
  logger.info(TAG, "[  PZEM %d] U:%7.2f V | I: %5.2f A | R: %5.1f Ω | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f | F: %4.1f Hz | U':%7.2f V |", 1, pzemO1.getVoltage(), pzemO1.getCurrent(), pzemO1.getResistance(), pzemO1.getPower(), pzemO1.getApparentPower(), pzemO1.getPowerFactor(), pzemO1.getTHDi(0), pzemO1.getFrequency(), pzemO1.getPower() / pzemO1.getCurrent());
  logger.info(TAG, "[  PZEM %d] U:%7.2f V | I: %5.2f A | R: %5.1f Ω | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f | F: %4.1f Hz | U':%7.2f V |", 2, pzemO2.getVoltage(), pzemO2.getCurrent(), pzemO2.getResistance(), pzemO2.getPower(), pzemO2.getApparentPower(), pzemO2.getPowerFactor(), pzemO2.getTHDi(0), pzemO2.getFrequency(), pzemO2.getPower() / pzemO2.getCurrent());
  logger.info(TAG, "[    GRID] U:%7.2f V | I: %5.2f A |            | P:%7.2f W | S:%7.2f VA | PF:%6.3f |             | F: %4.1f Hz |", gridMetrics.voltage, gridMetrics.current, gridMetrics.power, gridMetrics.apparentPower, gridMetrics.powerFactor, gridMetrics.frequency);
  logger.info(TAG, "[OUTPUT %d] U:%7.2f V | I: %5.2f A | R: %5.1f Ω | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f |            | U':%7.2f V | N:%7.2f W | D: %4" PRIu16 " | DC: %5.2f", 1, routerMetrics.outputs[0].voltage, routerMetrics.outputs[0].current, routerMetrics.outputs[0].resistance, routerMetrics.outputs[0].power, routerMetrics.outputs[0].apparentPower, routerMetrics.outputs[0].powerFactor, routerMetrics.outputs[0].thdi, routerMetrics.outputs[0].dimmedVoltage, routerMetrics.outputs[0].nominalPower, dimmerO1.getPowerDuty(), dimmerO1.getPowerDutyCycle());
  logger.info(TAG, "[OUTPUT %d] U:%7.2f V | I: %5.2f A | R: %5.1f Ω | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f |            | U':%7.2f V | N:%7.2f W | D: %4" PRIu16 " | DC: %5.2f", 2, routerMetrics.outputs[1].voltage, routerMetrics.outputs[1].current, routerMetrics.outputs[1].resistance, routerMetrics.outputs[1].power, routerMetrics.outputs[1].apparentPower, routerMetrics.outputs[1].powerFactor, routerMetrics.outputs[1].thdi, routerMetrics.outputs[1].dimmedVoltage, routerMetrics.outputs[1].nominalPower, dimmerO2.getPowerDuty(), dimmerO2.getPowerDutyCycle());
  logger.info(TAG,  "[  ROUTER]             | I: %5.2f A |            | P:%7.2f W | S:%7.2f VA | PF:%6.3f | THDi:%6.3f |", routerMetrics.current, routerMetrics.power, routerMetrics.apparentPower, routerMetrics.powerFactor, routerMetrics.thdi);
});
