// SPDX-License-Identifier: GPL-3.0-or-later
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

  logger.info(TAG, "JSY CH %d: U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz", 1, jsy.getVoltage1(), jsy.getCurrent1(), jsy.getResistance1(), jsy.getPower1(), jsy.getApparentPower1(), jsy.getPowerFactor1(), jsy.getTHDi1(0), jsy.getFrequency());
  logger.info(TAG, "JSY CH %d: U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz", 1, jsy.getVoltage2(), jsy.getCurrent2(), jsy.getResistance2(), jsy.getPower2(), jsy.getApparentPower2(), jsy.getPowerFactor2(), jsy.getTHDi2(0), jsy.getFrequency());
  logger.info(TAG, "PZEM O%d : U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz", 1, pzemO1.getVoltage(), pzemO1.getCurrent(), pzemO1.getResistance(), pzemO1.getPower(), pzemO1.getApparentPower(), pzemO1.getPowerFactor(), pzemO1.getTHDi(0), pzemO1.getFrequency());
  logger.info(TAG, "PZEM O%d : U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz", 2, pzemO2.getVoltage(), pzemO2.getCurrent(), pzemO2.getResistance(), pzemO2.getPower(), pzemO2.getApparentPower(), pzemO2.getPowerFactor(), pzemO2.getTHDi(0), pzemO2.getFrequency());
  logger.info(TAG, "GRID    : U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz", gridMetrics.voltage, gridMetrics.current, 0, gridMetrics.power, gridMetrics.apparentPower, gridMetrics.powerFactor, 0, gridMetrics.frequency);
  logger.info(TAG, "ROUTER  : U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz", 0, routerMetrics.current, 0, routerMetrics.power, routerMetrics.apparentPower, routerMetrics.powerFactor, routerMetrics.thdi, 0);
  logger.info(TAG, "OUTPUT %d: U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz U'=%3.3fV N=%3.3fW D=%" PRIu16 " DC=%1.4f", 1, routerMetrics.outputs[0].voltage, routerMetrics.outputs[0].current, output1.config.resistance, routerMetrics.outputs[0].power, routerMetrics.outputs[0].apparentPower, routerMetrics.outputs[0].powerFactor, routerMetrics.outputs[0].thdi, 0, routerMetrics.outputs[0].dimmedVoltage, routerMetrics.outputs[0].nominalPower, dimmerO1.getPowerDuty(), dimmerO1.getPowerDutyCycle());
  logger.info(TAG, "OUTPUT %d: U=%3.3fV I=%1.3fA R=%3.3fΩ P=%3.3fW S=%3.3fVA PF=%1.3f THDi=%1.3f f=%2.3fHz U'=%3.3fV N=%3.3fW D=%" PRIu16 " DC=%1.4f", 2, routerMetrics.outputs[1].voltage, routerMetrics.outputs[1].current, output2.config.resistance, routerMetrics.outputs[1].power, routerMetrics.outputs[1].apparentPower, routerMetrics.outputs[1].powerFactor, routerMetrics.outputs[1].thdi, 0, routerMetrics.outputs[1].dimmedVoltage, routerMetrics.outputs[1].nominalPower, dimmerO2.getPowerDuty(), dimmerO2.getPowerDutyCycle());
});
