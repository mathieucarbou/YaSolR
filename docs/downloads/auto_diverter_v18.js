// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 * 
 * ======================================
 * CHANGELOG
 * 
 * - v1: Initial version
 * - v2: Fix: Shelly Dimmer does not accept floats anymore (see here), which might create a max routing inaccuracy of 1% of the nominal load
 * - v3: Updated PID parameters (see here and here to have more info about how to tune the PID controller)
 * - v4: Fixed dimmer sharing feature to use Watts instead of %: using % based on PID output is wrong and will make the PID react to compensate. POWER_LIMIT can also be used to limit an output power to a specific value.
 * - v5: Introduced a 12-bits LUT to more closely match the voltage and current sine wave when computing the dimmer duty cycle to apply to the LSA
 * - v6: Added USE_POWER_LUT config to switch between linear dimming and LUT based dimming. v6 includes both code from v4 and v5.
 * - v7: Reworked the routing script to improve the internal relay lifespan inside the Shelly dimmer. This breaking change requires that you have configured the dimmer correctly according to the manual in order to have no power sent to the resistive load at 1%. You can use `DIMMER_TURN_OFF_DELAY` to control the dimmer timeout to turn it off.
 * - v8: Set OUT_MIN to -1000W by default to avoid the PID to trigger routing when the grid consumption drops near 0W in the morning for example if a load is turned off.
 * - v9: Implemented ability to switch PID parameters when the grid power is near the setpoint. This is useful to avoid the PID to overreact when the grid power is near the setpoint. The script will switch between HIGH and LOW PID parameters when the grid power is within a certain range of the setpoint. This is controlled by the HIGH_LOW_SWITCH parameter.
 * - v10: Added support for MQTT as a grid source in order to read the grid power and voltage from an external source. This is useful when the script is installed on the dimmer itself and the grid power and voltage are read from MQTT. The script will subscribe to the MQTT topics defined in the configuration and will use the values to compute the power to divert. This setup does not require a Shelly EM Pro.
 * - v11: Add config for nominal voltage
 * - v12: Implement ability to use Shelly Components for almost anything in config (currently only numbers and boolean are supported), values are read each divert call
 *        By @lexyan (https://gist.github.com/lexyan/d22b60a776bd0d8ebb677082113e8269#file-shelly_solar_diverter_v12-js)
 * - v13: Fix PID integral term anti-windup (typo in variable name)
 * - v14: Moved dimmer config in dimmer section and code cleanup
 * - v15: Support SINGLE or TRI phase modes for Shelly EM, PRO EM50, or 3EM
 *        Fixed teh behavior of Internal EM relay Switch used for bypass to correctly pause the dimmer when activating bypass
 *        Added support for remapping with MIN/MAX range on each dimmer
 *        By Mathieu and @Bormes (https://forum-photovoltaique.fr/viewtopic.php?p=840087&sid=f9f96caeac8a35eee320341a9c50b88d#p840087)
 * - v16: Added ability to use the internal relay of the Shelly Dimmer to do the bypass (full power) mode even when nothing is connected to the relay.
 * - v17: Introduced POWER_RATIO and POWER_LIMIT to better share the available power to divert between multiple dimmers and fixed issue with standby and full power modes impacting the power sharing
 * - v18: Removed GRID_SOURCE.PHASES: use GRID_SOURCE.TYPE instead with "EM", "3EM" or "MQTT"
 *        OUT_MIN and OUT_MAX moved to PID section
 *        New PID algorithm from https://mathieu.carbou.me/MycilaUtilities/pid (same used in YaSolR router)
 *        Improving MQTT integration to force a closed loop to get more frequent MQTT updates when using MQTT as grid source
 *        Added API:
 *          - /script/1/status => get current status and config
 *          - /script/1/status?debug=0|1|2 => set debug level
 *          - /script/1/status?<dimmerName>=standby|auto => set dimmer mode to standby or auto
 *        Note: bypass mode is controlled by using the Shelly switch (configure SHELLY_SWITCH_ID)
 * ======================================
 */
const scriptName = "auto_diverter";

// Config

const CONFIG = {
  // Debug mode: 0, 1 (info) or 2 (verbose)
  DEBUG: 1,
  // Set ID of the Bypass Switch button off the Shelly (== its output): 0 for PRO EM50, 100 for PRO 3EM, negative to disable
  // When set, the script will read the status of the switch to detect if the bypass relay is on or off in order to pause routing
  // This can only be set when you have wired to Shelly static relay to a contactor in order to use the Shelly button to activate 
  // a bypass mode that will heat at 100% without using the voltage regulator
  // This can also be set to 0 or 100 if you want the bypass to be done with the dimmer: when the relay will be switched on, the script will detect it and put the dimmer in full power mode.
  SHELLY_SWITCH_ID: 0,

  // Configure the sources for the grid power and voltage.
  // By default, the script will use a Shelly EM to read the grid power and voltage.
  // But it can be installed directly on the dimmer and read the power and voltage from MQTT.
  GRID_SOURCE: {
    TYPE: "EM", // "EM", "3EM" or "MQTT"

    // Enter your grid nominal voltage here. It will be used while waiting for the correct value from MQTT
    NOMINAL_VOLTAGE: 230,

    // MQTT Topic for Grid Power (only used when GRID_SOURCE.TYPE is "MQTT")
    MQTT_TOPIC_GRID_POWER: "homeassistant/states/sensor/grid_power/state",
    // MQTT Topic for Grid Voltage (only used when GRID_SOURCE.TYPE is "MQTT")
    MQTT_TOPIC_GRID_VOLTAGE: "homeassistant/states/sensor/grid_voltage/state",
  },
  // grid semi-period in micro-seconds
  SEMI_PERIOD: 10000,

  // PID
  // More information for tuning:
  // - https://forum-photovoltaique.fr/viewtopic.php?p=796194#p796194
  // - https://yasolr.carbou.me/manual#pid
  PID: {
    // PID trigger update interval in seconds
    INTERVAL_S: 1,
    // Proportional Mode:
    // - "error" (proportional on error),
    // - "input" (proportional on measurement),
    // - "both" (proportional on 50% error and 50% measurement)
    P_MODE: "input",
    // Derivative Mode:
    // - "error" (derivative on error),
    // - "input" (derivative on measurement)
    D_MODE: "input",
    // Target Grid Power (W)
    SETPOINT: -100,
    // Output Minimum (W): should be below the SETPOINT value, like 300-400W below.
    OUT_MIN: -400,
    // Output Maximum (W): should be above the nominal power of the load, ideally your maximum possible excess power, like your solar production peak power
    // I set below, the the available power to divert will be limited to this value globally for all dimmers.
    OUT_MAX: 6000,
    // PID Proportional Gain
    // Also try with 0.1, 0.2, 0.3
    KP: 0.1,
    // PID Integral Gain
    // Also try with 0.2, 0.3, 0.4, 0.5, 0.6 but keep it higher than Kp
    KI: 0.2,
    // PID Derivative Gain = keep it low
    KD: 0.05
  },

  // DIMMER LIST
  DIMMERS: {
    boiler: {
      IP: "192.168.123.173",
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 85,
      // The ratio of the power that the PID has calculated to be diverted to the dimmers
      // For example if set to 0.5 (== 50%), then this dimmer will take 50% of the available power to divert and the remaining 50% will be given to the next dimmers
      POWER_RATIO: 1.0,
      // Maximum excess power in Watts to reserve to this load.
      // The remaining power will be given to the next dimmers
      POWER_LIMIT: 0,
      // When set to true: the bypass mode (full power, 100%) is done thanks to the internal relay of the Shelly EM which must be wired to a contactor that will bypass the dimmer and allow the full load to be at 100%.
      // When set to false: the internal relay of the Shelly EM is not connected to anything but its state is read by the script to detect if the bypass mode is activated or not. If yes, the dimmer will be set to full power mode at 100%.
      BYPASS_THROUGH_EM_RELAY: false,
      // If set to true, the calculation of the dimmer duty cycle will be done based on a power matching LUT table which considers the voltage and current sine wave.
      // This should be more precise than a linear dimming.
      // You can try it and set it to false to compare the results, there is no harm in that!
      USE_POWER_LUT: true,
      // If the dimmer is running at 0-1% for this duration, it will be turned off automatically.
      // This duration should be long enough in order to not turn off the internal dimmer relay too often
      // The value is in milliseconds, the default is 5 minutes.
      DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
      // Minimum duty cycle value to set on the dimmer when routing.
      // Range: 1-99 %
      MIN: 1,
      // Maximum duty cycle value to set on the dimmer
      // Range: 1-99 %
      MAX: 99,
    },
    // pool_heater: {
    //   IP: "192.168.125.99",
    //   RESISTANCE: 0,
    //   POWER_RATIO: 1.0,
    //   POWER_LIMIT: 0,
    //   BYPASS_THROUGH_EM_RELAY: false,
    //   USE_POWER_LUT: true,
    //   DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
    //   MIN: 1,
    //   MAX: 99,
    // }
  }
};

// variable component
// You can add virtual components on the Shelly and register the IDs here
// They will take precedence over the static values in the config
const usercomponents = {
  // key is path in CONFIG
  // value is component identifier
  // "PID.SETPOINT": "number:200",
  // "DIMMERS.boiler.POWER_LIMIT": "number:201",
  // "DIMMERS.pool_heater.POWER_LIMIT": "number:202",
  // etc
}

// PID Controller 

let PID = {
  // PID Input
  input: 0,
  // PID Output
  output: 0,
  // current error value
  error: 0,
  // Proportional Term
  pTerm: 0,
  // Integral Term
  iTerm: 0,
  // Derivative Term
  dTerm: 0,
};

// Divert Control

let DIVERT = {
  gridVoltage: CONFIG.GRID_SOURCE.NOMINAL_VOLTAGE,
  dimmers: {}
};

function validateConfig(cb) {
  print(scriptName, ":", "Validating Config...");

  if (Object.keys(CONFIG.DIMMERS).length === 0) {
    print(scriptName, ":", "ERR: No dimmer configured");
    return;
  }

  for (let dimmer_name in CONFIG.DIMMERS) {
    if (CONFIG.DIMMERS[dimmer_name].RESISTANCE < 0) {
      print(scriptName, ":", "ERR: Dimmer", dimmer_name, " resistance should be greater than 0");
      return;
    }

    if (CONFIG.DIMMERS[dimmer_name].RESISTANCE === 0) {
      print(scriptName, ":", "Dimmer", dimmer_name, "is disabled");
      continue;
    }

    if (CONFIG.DIMMERS[dimmer_name].MIN < 1) {
      print(scriptName, ":", "ERR: MIN must be greater than or equal to 1");
      return;
    }
    if (CONFIG.DIMMERS[dimmer_name].MAX > 99) {
      print(scriptName, ":", "ERR: MAX must be less than or equal to 99");
      return;
    }
    if (CONFIG.DIMMERS[dimmer_name].MIN >= CONFIG.DIMMERS[dimmer_name].MAX) {
      print(scriptName, ":", "ERR: MIN must be less than MAX");
      return;
    }

    print(scriptName, ":", "Dimmer", dimmer_name, "is enabled");
    DIVERT.dimmers[dimmer_name] = {
      powerToDivert: 0,
      fullPower: false,
      standby: false,
    };
  }

  if (CONFIG.GRID_SOURCE.TYPE !== "EM" && CONFIG.GRID_SOURCE.TYPE !== "3EM" && CONFIG.GRID_SOURCE.TYPE !== "MQTT") {
    print(scriptName, ":", 'ERR: GRID TYPE must be "EM", "3EM" or "MQTT"');
    return;
  }

  cb();
}

function constrain(value, min, max) { return Math.min(Math.max(value, min), max); }

// LUT table for dimmer range mapping power output

const DIMMER_RESOLUTION = 12; // 12 bits
const DIMMER_MAX = (1 << DIMMER_RESOLUTION) - 1; // 4095
const TABLE_PHASE_LEN = 80;
const TABLE_PHASE_SCALE = (TABLE_PHASE_LEN - 1) * (1 << (16 - DIMMER_RESOLUTION)); // 1264
const TABLE_PHASE_DELAY = [0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094, 0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2, 0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9, 0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8, 0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf, 0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2, 0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606, 0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686, 0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296, 0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016];

function lookupFiringDelay(dutyCycle) {
  if (dutyCycle == 0)
    return CONFIG.SEMI_PERIOD;
  if (dutyCycle == 1)
    return 0;
  const duty = dutyCycle * DIMMER_MAX;
  const slot = duty * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 2);
  const index = slot >> 16;
  const a = TABLE_PHASE_DELAY[index];
  const b = TABLE_PHASE_DELAY[index + 1];
  const delay = a - (((a - b) * (slot & 0xffff)) >> 16); // interpolate a b
  return (delay * CONFIG.SEMI_PERIOD) >> 16; // scale to period
}

// PID
// https://mathieu.carbou.me/MycilaUtilities/pid

function calculatePID(input) {
  // input delta
  const dInput = input - PID.input;

  // error and error delta
  const error = CONFIG.PID.SETPOINT - input;
  const dError = error - PID.error;

  // calculate proportional term
  if (CONFIG.PID.P_MODE == "error")
    PID.pTerm = CONFIG.PID.KP * error;
  else if (CONFIG.PID.P_MODE == "input")
    PID.pTerm -= CONFIG.PID.KP * dInput;

  // calculate integral term
  PID.iTerm += CONFIG.PID.KI * error;
  PID.iTerm = constrain(PID.iTerm, CONFIG.PID.OUT_MIN, CONFIG.PID.OUT_MAX);

  // calculate derivative term
  if (CONFIG.PID.D_MODE == "error")
    PID.dTerm = CONFIG.PID.KD * dError;
  else if (CONFIG.PID.D_MODE == "input")
    PID.dTerm = -CONFIG.PID.KD * dInput;

  // calculate output
  PID.output = constrain(PID.pTerm + PID.iTerm + PID.dTerm, CONFIG.PID.OUT_MIN, CONFIG.PID.OUT_MAX);

  if (CONFIG.DEBUG > 0) {
    print(scriptName, ":", "[PID] Input:", input, "W, Error:", error, "W, pTerm:", PID.pTerm, "W, iTerm:", PID.iTerm, "W, dTerm:", PID.dTerm, "W, Output:", PID.output, "W");
  }

  // store input and error for next iteration
  PID.input = input;
  PID.error = error;

  return PID.output;
}

// Dimmer calls

function callDimmerCallback(result, errCode, errMessage, data) {
  if (errCode) {
    print(scriptName, ":", "ERR callDimmerCallback:", errCode);
    data.dimmer.rpc = "failed";
  } else if (result.code !== 200) {
    const rpcResult = JSON.parse(result.body);
    print(scriptName, ":", "ERR", rpcResult.code, ":", rpcResult.message);
    data.dimmer.rpc = "failed";
  } else {
    data.dimmer.rpc = "success";
  }
  data.cb();
}

function callDimmer(dimmer_name, dimmer, cb) {
  let dimmerPercentage = 0;
  let relay = "";

  if (!dimmer.lastActivation) {
    dimmer.lastActivation = 0;
  }

  if (dimmer.standby) {
    // standby mode has higher priority than full power mode
    // we set the dimmer to 0% and turn off the relay
    dimmer.fullPower = false;
    dimmerPercentage = 0;
    relay = "&on=false";
    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "[" + dimmer_name + "] STANDBY => relay: OFF, dimmer: 0%");

  } else if (dimmer.fullPower) {
    // full power mode
    // we set the dimmer to 100% and turn on the relay
    dimmerPercentage = 100;
    relay = "&on=true";
    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "[" + dimmer_name + "] FULL POWER => relay: ON, dimmer: 100%");


  } else {
    // normal routing mode
    // calculate dimmer percentage based on duty cycle or power LUT table
    // and make sure it is within the min/max range which is validated to be within 1-99
    let p = CONFIG.DIMMERS[dimmer_name].USE_POWER_LUT ? (CONFIG.SEMI_PERIOD - dimmer.firingDelay) / CONFIG.SEMI_PERIOD : dimmer.dutyCycle;
    dimmerPercentage = Math.round(constrain(p * 100, CONFIG.DIMMERS[dimmer_name].MIN, CONFIG.DIMMERS[dimmer_name].MAX));

    if (dimmerPercentage > CONFIG.DIMMERS[dimmer_name].MIN) {
      // If dimmer % > CONFIG.DIMMERS[dimmer_name].MIN
      // Then the dimmer is active and powering the load
      relay = "&on=true";
      dimmer.lastActivation = Date.now();
      if (CONFIG.DEBUG > 1)
        print(scriptName, ":", "[" + dimmer_name + "] ROUTING => relay: ON, dimmer:", dimmerPercentage + "%");

    } else { // dimmerPercentage === CONFIG.DIMMERS[dimmer_name].MIN
      // Dimmer reaches the MIN value where no power goes to the load
      // Depending on the time , we have to turn the relay off
      if (Date.now() - dimmer.lastActivation >= CONFIG.DIMMERS[dimmer_name].DIMMER_TURN_OFF_DELAY) {
        relay = "&on=false";
        if (CONFIG.DEBUG)
          print(scriptName, ":", "[" + dimmer_name + "] ROUTING => relay: OFF, dimmer:", dimmerPercentage + "%");
      } else {
        relay = "&on=true";
        if (CONFIG.DEBUG > 1)
          print(scriptName, ":", "[" + dimmer_name + "] ROUTING => relay: ON, dimmer:", dimmerPercentage + "% (waiting to turn relay off)");
      }
    }
  }

  // Build the URL to call...
  const url = "http://" + CONFIG.DIMMERS[dimmer_name].IP + "/rpc/Light.Set?id=0" + relay + "&brightness=" + dimmerPercentage + "&transition_duration=0.5";

  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "[" + dimmer_name + "] => Calling: ", url);

  Shelly.call("HTTP.GET", { url: url, timeout: 5 }, callDimmerCallback, { dimmer: dimmer, cb: cb });
}

function callDimmers() {
  for (let dimmer_name in DIVERT.dimmers) {
    let dimmer = DIVERT.dimmers[dimmer_name];

    // ignore contacted dimmers
    if (dimmer.rpc !== "pending") {
      continue;
    }

    if (isNaN(dimmer.dutyCycle)) {
      dimmer.rpc = "failed";
      print(scriptName, ":", "ERR: Invalid duty cycle for dimmer", dimmer_name);
      continue;
    }

    // call dimmer
    callDimmer(dimmer_name, dimmer, callDimmers);

    // exit the loop immediately to avoid multiple calls in case they are made in parallel
    return;
  }
}

// Shelly EM bypass switch handling

function onSwitchGetStatus(result, errCode, errMessage, data) {
  if (errCode === 404) {
    // Switch.GetStatus does not exist: we do not run on a EM.
    errCode = 0;
    result = { output: false };
  }
  if (errCode) {
    print(scriptName, ":", "ERR onSwitchGetStatus:", errCode);
    return;
  }
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "[onSwitchGetStatus] ", JSON.stringify(result));
  for (let dimmer_name in DIVERT.dimmers) {
    let dimmer = DIVERT.dimmers[dimmer_name];
    if (CONFIG.DIMMERS[dimmer_name].BYPASS_THROUGH_EM_RELAY) {
      // we are using the EM relay to do the bypass, so there is a contactor wired to the relay
      if (result.output) {
        // Relay ON ==> bypass mode ON ==> set dimmer OFF
        print(scriptName, ":", "[" + dimmer_name + "] Bypass ON => turning off");
        dimmer.apparentPower = 0;
        dimmer.current = 0;
        dimmer.dimmedVoltage = 0;
        dimmer.powerToDivert = 0;
        dimmer.dutyCycle = 0;
        dimmer.firingDelay = 0;
        dimmer.powerFactor = 0;
        dimmer.thdi = 0;
        // when we press on the Shelly button, we want to turn on the bypass mode
        // so we deactivate standby
        dimmer.standby = true;
        dimmer.fullPower = false;
      } else {
        // Relay OFF ==> bypass mode OFF ==> normal operation
        // when we switch off the bypass mode, we want to exit bypass mode
        // but do not touch the standby mode in case it was activated by user to keep the dimmer off
        dimmer.fullPower = false;
      }
    } else {
      // we are using the EM relay to do a bypass virtually using the dimmer itself
      // when the relay is ON, we set the dimmer to full power mode
      // when the relay is OFF, we deactivate full power mode
      if (result.output) {
        print(scriptName, ":", "[" + dimmer_name + "] Bypass ON through dimmer => 100% (full power)");
        dimmer.apparentPower = 0;
        dimmer.current = 0;
        dimmer.dimmedVoltage = 0;
        dimmer.powerToDivert = 0;
        dimmer.dutyCycle = 0;
        dimmer.firingDelay = 0;
        dimmer.powerFactor = 0;
        dimmer.thdi = 0;
        dimmer.standby = false;
        dimmer.fullPower = true;
      } else {
        dimmer.fullPower = false;
      }
    }
  }
  callDimmers();
}

// Main divert function

function divert(gridVoltage, gridPower) {
  getDynUserSettings();

  let availablePowerToDivert = calculatePID(gridPower);

  for (let dimmer_name in DIVERT.dimmers) {
    let dimmer = DIVERT.dimmers[dimmer_name];

    // calculate powerToDivert, between 0 and maxPower
    dimmer.maxPower = gridVoltage * gridVoltage / CONFIG.DIMMERS[dimmer_name].RESISTANCE;

    if (dimmer.fullPower) {
      dimmer.powerToDivert = dimmer.maxPower;

    } else if (dimmer.standby) {
      dimmer.powerToDivert = 0;

    } else {
      dimmer.powerToDivert = constrain(availablePowerToDivert, 0, dimmer.maxPower);
      // apply the POWER_RATIO
      dimmer.powerToDivert = Math.round(dimmer.powerToDivert * CONFIG.DIMMERS[dimmer_name].POWER_RATIO);
      // clamp to POWER_LIMIT if set
      if (CONFIG.DIMMERS[dimmer_name].POWER_LIMIT > 0)
        dimmer.powerToDivert = constrain(dimmer.powerToDivert, 0, CONFIG.DIMMERS[dimmer_name].POWER_LIMIT);

      // decrease the available power to divert for the next dimmers
      availablePowerToDivert -= dimmer.powerToDivert;
    }

    // calculate dutyCycle
    dimmer.dutyCycle = dimmer.maxPower == 0 ? 0 : constrain(dimmer.powerToDivert / dimmer.maxPower, 0, 1);
    dimmer.firingDelay = lookupFiringDelay(dimmer.dutyCycle);

    // derive abstract (not real measured) metrics
    dimmer.powerFactor = Math.sqrt(dimmer.dutyCycle);
    dimmer.dimmedVoltage = dimmer.powerFactor * gridVoltage;
    dimmer.current = dimmer.dimmedVoltage / CONFIG.DIMMERS[dimmer_name].RESISTANCE;
    dimmer.apparentPower = dimmer.current * gridVoltage;
    dimmer.thdi = dimmer.dutyCycle === 0 ? 0 : Math.sqrt(1 / dimmer.dutyCycle - 1);
    dimmer.rpc = "pending";

    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "[divert] [" + dimmer_name + "] => ", Math.round(dimmer.powerToDivert), "W,", Math.round(dimmer.dutyCycle * 100), "%");
  }

  if (CONFIG.SHELLY_SWITCH_ID >= 0)
    Shelly.call("Switch.GetStatus", { id: CONFIG.SHELLY_SWITCH_ID }, onSwitchGetStatus);
  else
    callDimmers();
}

// HTTP handlers

function onHttpGetStatus(request, response) {
  if (request.query) {
    request.query.split("&").forEach(function (param) {
      const kv = param.split("=");
      if (kv[0] == "debug") {
        print(scriptName, ":", "[HTTP] Setting debug level to", kv[1]);
        CONFIG.DEBUG = parseInt(kv[1]);
      } else if (kv[0] in DIVERT.dimmers) {
        if (kv[1] == "standby") {
          print(scriptName, ":", "[HTTP] Setting dimmer", kv[0], "to STANDBY mode");
          DIVERT.dimmers[kv[0]].standby = true;
          DIVERT.dimmers[kv[0]].fullPower = false;
        } else if (kv[1] == "auto") {
          print(scriptName, ":", "[HTTP] Setting dimmer", kv[0], "to AUTO mode");
          DIVERT.dimmers[kv[0]].standby = false;
          DIVERT.dimmers[kv[0]].fullPower = false;
        }
      }
    });
  }

  response.code = 200;
  response.headers = {
    "Content-Type": "application/json"
  };
  response.body = JSON.stringify({
    config: CONFIG,
    pid: PID,
    divert: DIVERT
  });
  response.send();
}

// Dynamic User configurable Config Variables
function getByPath(obj, path) {
  var parts = path.split(".");
  var current = obj;
  var i;
  for (i = 0; i < parts.length; i++) {
    if (current == null) return undefined;
    current = current[parts[i]];
  }
  return current;
}

function setByPath(obj, path, value) {
  var parts = path.split(".");
  var current = obj;
  var i;
  for (i = 0; i < parts.length - 1; i++) {
    if (!current[parts[i]]) current[parts[i]] = {};
    current = current[parts[i]];
  }
  current[parts[parts.length - 1]] = value;
}

function getDynUserSettings() {
  for (let user_def in usercomponents) {
    let virtualId = usercomponents[user_def]; // e.g. "number:200"
    let handle = Virtual.getHandle(virtualId);

    if (handle === null) {
      print(scriptName, ":", "WARN: Dynamic component not found with virtual ID: ", virtualId);
      continue; // on ignore cet item et passe au suivant
    }

    try {
      let value = handle.getValue();
      setByPath(CONFIG, user_def, value);
    } catch (e) {
      print(scriptName, ":", "ERR: getValue() failed for", virtualId, "-", e);
    }
  }
}

// Main

validateConfig(function () {
  print(scriptName, ":", "Starting Shelly Solar Diverter...");

  // Register HTTP endpoint
  HTTPServer.registerEndpoint("status", onHttpGetStatus);

  if (CONFIG.GRID_SOURCE.TYPE === "EM") {
    // Setup Shelly EM / 3EM readings
    print(scriptName, ":", "Grid Source: Shelly EM");
    Timer.set(CONFIG.PID.INTERVAL_S * 1000, true, function () {
      const emMethod = CONFIG.GRID_SOURCE.TYPE === "3EM" ? "EM.GetStatus" : "EM1.GetStatus";
      Shelly.call(emMethod, { id: 0 }, function (result, errCode, errMessage) {
        if (errCode) {
          print(scriptName, ":", "ERR", emMethod, ":", errCode, errMessage);
          return;
        }
        if (CONFIG.DEBUG > 1)
          print(scriptName, ":", emMethod, JSON.stringify(result));
        // exposes grid power and voltage to HTTP API endpoint and other parts of this script
        if (CONFIG.GRID_SOURCE.TYPE === "EM") {
          DIVERT.gridVoltage = result.voltage;
          DIVERT.gridPower = result.act_power;
        } else if (CONFIG.GRID_SOURCE.TYPE === "3EM") {
          DIVERT.gridVoltage = result.a_voltage;
          DIVERT.gridPower = result.total_act_power;
        }
      });
    });

  } else if (CONFIG.GRID_SOURCE.TYPE === "MQTT") {
    // Setup MQTT readings
    print(scriptName, ":", "Grid Source: MQTT");
    MQTT.subscribe(CONFIG.GRID_SOURCE.MQTT_TOPIC_GRID_VOLTAGE, function (topic, message) {
      if (CONFIG.DEBUG > 1) {
        print(scriptName, ":", "MQTT:", topic, "=>", message);
      }
      DIVERT.gridVoltage = parseFloat(message);
    });
    MQTT.subscribe(CONFIG.GRID_SOURCE.MQTT_TOPIC_GRID_POWER, function (topic, message) {
      if (CONFIG.DEBUG > 1) {
        print(scriptName, ":", "MQTT:", topic, "=>", message);
      }
      DIVERT.gridPower = parseFloat(message);
    });

  } else {
    print(scriptName, ":", "ERR: Invalid Grid Source: ", CONFIG.GRID_SOURCE.TYPE);
    return;
  }

  // start periodic PID calculation and diverting
  Timer.set(CONFIG.PID.INTERVAL_S * 1000, true, function () {
    if (DIVERT.gridVoltage && DIVERT.gridPower) {
      divert(DIVERT.gridVoltage, DIVERT.gridPower);
    }
  });
});
