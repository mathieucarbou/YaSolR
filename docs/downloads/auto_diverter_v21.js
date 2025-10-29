// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
const scriptName = "auto_diverter";

// Config

const CONFIG = {
  // Debug mode: 0, 1 (info) or 2 (verbose)
  DEBUG: 1,

  GRID_SOURCE: {
    // Configure the API type to call to get grid power and voltage
    // EM will use Shelly EM API (EM1.GetStatus)
    // 3EM will use Shelly 3EM aggregate API (EM.GetStatus)
    // MQTT will use MQTT topics to read grid power and voltage
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

  // Define how the bypass mode is done (== marche forcée)
  // Bypass is activated when we turn the Shelly EM switch on, and the goal is let let pass 100% power to the load.
  // - Set it to "virtual" if nothing is connected to the Shelly EM relay. The script will then turn the dimmer to 100% to do the bypass.
  // - Set it to "contactor" if you have wired the Shelly EM relay to a power contactor that will bypass the dimmer when activated. In this case, the script will turn off the dimmer when bypass is activated.
  // - Set it to "api" if you need to control the bypass of each dimmer independently through an API call to /script/1/status?dimmerName=full
  // Note: in both "virtual" and "contactor" modes, when the bypass is activated, all dimmers will be set to 100% power.
  BYPASS: "virtual",

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
    D_MODE: "error",
    // Target Grid Power (W)
    SETPOINT: -100,
    // Output Minimum (W): should be below the SETPOINT value, like 300-400W below.
    OUT_MIN: -400,
    // Output Maximum (W): should be above the nominal power of the load, ideally your maximum possible excess power, like your solar production peak power
    // I set below, the the available power to divert will be limited to this value globally for all dimmers.
    OUT_MAX: 6000,
    // PID Proportional Gain
    // Also try with 0.1, 0.2, 0.3
    KP: 0.2,
    // PID Integral Gain
    // Also try with 0.2, 0.3, 0.4, 0.5, 0.6 but keep it higher than Kp
    KI: 0.4,
    // PID Derivative Gain = keep it low
    KD: 0.02
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
      // If set to true, the calculation of the dimmer duty cycle will be done based on a power matching LUT table which considers the voltage and current sine wave.
      // This should be more precise than a linear dimming.
      // You can try it and set it to false to compare the results, there is no harm in that!
      USE_POWER_LUT: true,
      // If the dimmer is running at 0-1% for this duration, it will be turned off automatically.
      // This duration should be long enough in order to not turn off the internal dimmer relay too often
      // The value is in milliseconds, the default is 5 minutes.
      DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
      // Minimum duty cycle value to set on the dimmer when routing.
      // MIN matches the number where no power is sent to the load and when the dimmer will be turned off after DIMMER_TURN_OFF_DELAY.
      // Range: 0-100 %
      MIN: 0,
      // Maximum duty cycle value to set on the dimmer when routing.
      // Range: 0-100 %
      MAX: 100,
    },
    // pool_heater: {
    //   IP: "192.168.125.99",
    //   RESISTANCE: 0,
    //   POWER_RATIO: 1.0,
    //   POWER_LIMIT: 0,
    //   USE_POWER_LUT: true,
    //   DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
    //   MIN: 0,
    //   MAX: 9100,
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

function constrain(value, min, max) { return Math.min(Math.max(value, min), max); }

// LUT table for dimmer range mapping power output

const DIMMER_RESOLUTION = 12; // 12 bits
const DIMMER_MAX = (1 << DIMMER_RESOLUTION) - 1; // 4095
const TABLE_PHASE_LEN = 80;
const TABLE_PHASE_SCALE = (TABLE_PHASE_LEN - 1) * (1 << (16 - DIMMER_RESOLUTION)); // 1264
const TABLE_PHASE_DELAY = [0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094, 0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2, 0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9, 0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8, 0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf, 0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2, 0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606, 0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686, 0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296, 0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016];

function lookupFiringDelay(dutyCycle) {
  if (dutyCycle === 0)
    return CONFIG.SEMI_PERIOD;
  if (dutyCycle === 1)
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
  if (CONFIG.PID.P_MODE === "error")
    PID.pTerm = CONFIG.PID.KP * error;
  else if (CONFIG.PID.P_MODE === "input")
    PID.pTerm -= CONFIG.PID.KP * dInput;

  // calculate integral term
  PID.iTerm += CONFIG.PID.KI * error;
  PID.iTerm = constrain(PID.iTerm, CONFIG.PID.OUT_MIN, CONFIG.PID.OUT_MAX);

  // calculate derivative term
  if (CONFIG.PID.D_MODE === "error")
    PID.dTerm = CONFIG.PID.KD * dError;
  else if (CONFIG.PID.D_MODE === "input")
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

function callDimmer(dimmer_name, cb) {
  const dimmer = DIVERT.dimmers[dimmer_name];
  let dimmerPercentage = 0;
  let relay = "";

  if (!dimmer.lastActivation) {
    dimmer.lastActivation = 0;
  }

  if (dimmer.standby) {
    // standby mode has higher priority than bypass mode
    // we set the dimmer to 0% and turn off the relay
    dimmerPercentage = 0;
    relay = "&on=false";

    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "[" + dimmer_name + "]", "STANDBY => relay: OFF, dimmer: 0%");

  } else if (dimmer.bypass) {
    // bypass mode
    // we set the dimmer to 100% and turn on the relay
    dimmerPercentage = 100;
    relay = "&on=true";

    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "[" + dimmer_name + "]", "BYPASS => relay: ON, dimmer: 100%");


  } else {
    // normal routing mode
    // calculate dimmer percentage based on duty cycle or power LUT table
    let p = CONFIG.DIMMERS[dimmer_name].USE_POWER_LUT ? (CONFIG.SEMI_PERIOD - dimmer.firingDelay) / CONFIG.SEMI_PERIOD : dimmer.dutyCycle;
    dimmerPercentage = Math.round(constrain(p * 100, CONFIG.DIMMERS[dimmer_name].MIN, CONFIG.DIMMERS[dimmer_name].MAX));

    if (dimmerPercentage > CONFIG.DIMMERS[dimmer_name].MIN) {
      // If dimmer % > CONFIG.DIMMERS[dimmer_name].MIN
      // Then the dimmer is active and powering the load
      relay = "&on=true";
      dimmer.lastActivation = Date.now();

      if (CONFIG.DEBUG > 0)
        print(scriptName, ":", "[" + dimmer_name + "]", "ROUTING => relay: ON, dimmer:", dimmerPercentage + "%");

    } else { // dimmerPercentage === CONFIG.DIMMERS[dimmer_name].MIN
      // Dimmer reaches the MIN value where no power goes to the load
      // Depending on the time , we have to turn the relay off
      if (Date.now() - dimmer.lastActivation >= CONFIG.DIMMERS[dimmer_name].DIMMER_TURN_OFF_DELAY) {
        relay = "&on=false";

        if (CONFIG.DEBUG)
          print(scriptName, ":", "[" + dimmer_name + "]", "ROUTING => relay: OFF, dimmer:", dimmerPercentage + "%");

      } else {
        relay = "&on=true";

        if (CONFIG.DEBUG > 0)
          print(scriptName, ":", "[" + dimmer_name + "]", "ROUTING => relay: ON, dimmer:", dimmerPercentage + "% (waiting to turn relay off)");
      }
    }
  }

  // Build the URL to call...
  const url = "http://" + CONFIG.DIMMERS[dimmer_name].IP + "/rpc/Light.Set?id=0" + relay + "&brightness=" + dimmerPercentage + "&transition_duration=0.5";

  // Do not call the dimmer if the URL is the same as last time
  // This will help prevent errors such as "Uncaught Error: Too many calls in progress"
  if (dimmer.url === url) {
    // same as last time, skip the call
    dimmer.rpc = "skipped";
    if (CONFIG.DEBUG > 1)
      print(scriptName, ":", "[" + dimmer_name + "] => Skipping call!");
    cb();
    return;
  }

  dimmer.url = url;

  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "[" + dimmer_name + "] HTTP.GET ", url);

  Shelly.call("HTTP.GET", { url: url, timeout: 5 }, function (result, errCode, errMessage, data) {
    if (errCode) {
      print(scriptName, ":", "[" + data.dimmer_name + "] HTTP.GET ERR:", errCode, errMessage);
      data.dimmer.rpc = "failed";

    } else if (result.code !== 200) {
      const rpcResult = JSON.parse(result.body);
      print(scriptName, ":", "[" + data.dimmer_name + "] HTTP.GET ERR:", rpcResult.code, rpcResult.message);
      data.dimmer.rpc = "failed";

    } else {
      data.dimmer.rpc = "success";
    }

    data.cb();
  }, { dimmer_name: dimmer_name, dimmer: dimmer, cb: cb });
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
    callDimmer(dimmer_name, callDimmers);

    // exit the loop immediately to avoid multiple calls in case they are made in parallel
    return;
  }
}

// Dimmer controls

function enableBypass(dimmer_name) {
  const dimmer = DIVERT.dimmers[dimmer_name];

  if (dimmer.bypass) {
    // already in bypass mode, skip
    return;
  }

  dimmer.apparentPower = 0;
  dimmer.current = 0;
  dimmer.dimmedVoltage = 0;
  dimmer.powerToDivert = 0;
  dimmer.dutyCycle = 0;
  dimmer.firingDelay = 0;
  dimmer.powerFactor = 0;
  dimmer.thdi = 0;

  // backup standby state if not already done
  if (dimmer.standby_backup === undefined || dimmer.standby_backup === null) {
    dimmer.standby_backup = dimmer.standby;
  }

  if (CONFIG.BYPASS === "contactor") {
    // we are using the EM relay connected to a contactor to do the bypass, so we need to turn the dimmer off when the relay is ON
    dimmer.standby = true;

  } else if (CONFIG.BYPASS === "virtual") {
    // we are using the EM relay to do a bypass virtually using the dimmer itself
    // when the relay is ON, we set the dimmer to bypass mode
    // when the relay is OFF, we deactivate bypass mode
    dimmer.standby = false;
  }

  dimmer.bypass = true;

  print(scriptName, ":", "[" + dimmer_name + "]", "=> Bypass ON");
  print(scriptName, ":", "[" + dimmer_name + "]", "=> Dimmer", dimmer.standby ? "STANDBY" : "FULL POWER");
}

function disableBypass(dimmer_name) {
  const dimmer = DIVERT.dimmers[dimmer_name];

  if (!dimmer.bypass) {
    // already in normal mode, skip
    return;
  }

  // Bypass OFF ==> back to normal operation
  dimmer.bypass = false;

  if (dimmer.standby_backup !== undefined && dimmer.standby_backup !== null) {
    dimmer.standby = dimmer.standby_backup;
    dimmer.standby_backup = null;
  }

  print(scriptName, ":", "[" + dimmer_name + "]", "=> Bypass OFF");
  print(scriptName, ":", "[" + dimmer_name + "]", "=> Dimmer", dimmer.standby ? "STANDBY" : "AUTO");
}

function enableDivert(dimmer_name) {
  const dimmer = DIVERT.dimmers[dimmer_name];
  dimmer.standby = false;
  dimmer.bypass = false;
  print(scriptName, ":", "[" + dimmer_name + "]", "=> Dimmer AUTO");
}

function disableDivert(dimmer_name) {
  const dimmer = DIVERT.dimmers[dimmer_name];
  dimmer.standby = true;
  print(scriptName, ":", "[" + dimmer_name + "]", "=> Dimmer STANDBY");
}

// Shelly EM bypass switch handling

function switchGetStatus() {
  if (CONFIG.BYPASS === "api") {
    // if we are using API bypass mode, we do not need to check the switch status
    callDimmers();
    return;
  }

  // Here: we are using either "contactor" or "virtual" bypass mode
  Shelly.call("Switch.GetStatus", { id: DIVERT.switchID }, function onSwitchGetStatus(result, errCode, errMessage) {
    if (errCode === 404) {
      // Switch.GetStatus does not exist: we do not run on a EM.
      errCode = 0;
      result = { output: false };
    }

    if (errCode) {
      print(scriptName, ":", "[Switch.GetStatus] ERR:", errCode, errMessage);
      return;
    }

    if (CONFIG.DEBUG > 1)
      print(scriptName, ":", "[Switch.GetStatus]", JSON.stringify(result));

    for (let dimmer_name in DIVERT.dimmers) {
      if (result.output) {
        enableBypass(dimmer_name);
      } else {
        disableBypass(dimmer_name);
      }
    }

    // call dimmers after processing the bypass state
    callDimmers();
  });
}

// Main divert function

function divert(gridVoltage, gridPower) {
  getDynUserSettings();

  let availablePowerToDivert = calculatePID(gridPower);

  for (let dimmer_name in CONFIG.DIMMERS) {
    // initialize dimmer state if not present
    if (!DIVERT.dimmers[dimmer_name]) {
      DIVERT.dimmers[dimmer_name] = {
        standby: false,
        bypass: false,
        powerToDivert: 0,
        maxPower: 0,
        dutyCycle: 0,
        firingDelay: 0,
        powerFactor: 0,
        dimmedVoltage: 0,
        current: 0,
        apparentPower: 0,
        thdi: 0,
        rpc: "skipped",
        url: "",
      };
    }

    let dimmer = DIVERT.dimmers[dimmer_name];

    // calculate powerToDivert, between 0 and maxPower
    dimmer.maxPower = gridVoltage * gridVoltage / CONFIG.DIMMERS[dimmer_name].RESISTANCE;

    if (dimmer.bypass) {
      dimmer.powerToDivert = dimmer.maxPower;

    } else if (dimmer.standby) {
      dimmer.powerToDivert = 0;

    } else {
      // apply the POWER_RATIO first to share the available power to divert
      dimmer.powerToDivert = Math.round(availablePowerToDivert * CONFIG.DIMMERS[dimmer_name].POWER_RATIO);
      // then clamp to the available power to divert and maxPower
      dimmer.powerToDivert = constrain(dimmer.powerToDivert, 0, dimmer.maxPower);
      // then clamp to POWER_LIMIT if set
      if (CONFIG.DIMMERS[dimmer_name].POWER_LIMIT > 0)
        dimmer.powerToDivert = constrain(dimmer.powerToDivert, 0, CONFIG.DIMMERS[dimmer_name].POWER_LIMIT);

      // decrease the available power to divert for the next dimmers
      availablePowerToDivert -= dimmer.powerToDivert;
    }

    // calculate dutyCycle
    dimmer.dutyCycle = dimmer.maxPower === 0 ? 0 : constrain(dimmer.powerToDivert / dimmer.maxPower, 0, 1);
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

  switchGetStatus();
}

// HTTP handlers

function onHttpGetStatus(request, response) {
  if (request.query) {
    request.query.split("&").forEach(function (param) {
      const kv = param.split("=");

      if (kv[0] === "debug") {
        print(scriptName, ":", "[/status] Setting debug level to", kv[1]);
        CONFIG.DEBUG = parseInt(kv[1]);

      } if (kv[0] === "setpoint") {
        print(scriptName, ":", "[/status] Setting PID setpoint to", kv[1]);
        CONFIG.PID.SETPOINT = parseInt(kv[1]);

      } if (kv[0] === "reset") {
        print(scriptName, ":", "[/status] Reset");
        PID.input = 0;
        PID.output = 0;
        PID.error = 0;
        PID.pTerm = 0;
        PID.iTerm = 0;
        PID.dTerm = 0;
        DIVERT.dimmers = {};

      } else if (kv[0] in DIVERT.dimmers) {
        if (kv[1] === "standby") {
          print(scriptName, ":", "[/status] Setting dimmer", kv[0], "to STANDBY mode");
          disableDivert(kv[0]);

        } else if (kv[1] === "auto") {
          print(scriptName, ":", "[/status] Setting dimmer", kv[0], "to AUTO mode");
          enableDivert(kv[0]);

        } else if (kv[1] === "bypass") {
          print(scriptName, ":", "[/status] Setting dimmer", kv[0], "to BYPASS mode");
          enableBypass(kv[0]);
        }

      } else if (kv[0] === "all") {
        if (kv[1] === "standby") {
          for (let dimmer_name in DIVERT.dimmers) {
            print(scriptName, ":", "[/status] Setting dimmer", dimmer_name, "to STANDBY mode");
            disableDivert(dimmer_name);
          }
        } else if (kv[1] === "auto") {
          for (let dimmer_name in DIVERT.dimmers) {
            print(scriptName, ":", "[/status] Setting dimmer", dimmer_name, "to AUTO mode");
            enableDivert(dimmer_name);
          }
        } else if (kv[1] === "bypass") {
          for (let dimmer_name in DIVERT.dimmers) {
            print(scriptName, ":", "[/status] Setting dimmer", dimmer_name, "to BYPASS mode");
            enableBypass(dimmer_name);
          }
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
    if (current === null) return undefined;
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

// config validation

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

    if (CONFIG.DIMMERS[dimmer_name].MIN < 0) {
      print(scriptName, ":", "ERR: MIN must be greater than or equal to 0");
      return;
    }
    if (CONFIG.DIMMERS[dimmer_name].MAX > 100) {
      print(scriptName, ":", "ERR: MAX must be less than or equal to 100");
      return;
    }
    if (CONFIG.DIMMERS[dimmer_name].MIN >= CONFIG.DIMMERS[dimmer_name].MAX) {
      print(scriptName, ":", "ERR: MIN must be less than MAX");
      return;
    }
    if (CONFIG.BYPASS !== "virtual" && CONFIG.BYPASS !== "contactor" && CONFIG.BYPASS !== "api") {
      print(scriptName, ":", "ERR: BYPASS must be either 'virtual', 'contactor' or 'api'");
      return;
    }

    print(scriptName, ":", "Dimmer", dimmer_name, "is enabled");
  }

  if (CONFIG.GRID_SOURCE.TYPE !== "EM" && CONFIG.GRID_SOURCE.TYPE !== "3EM" && CONFIG.GRID_SOURCE.TYPE !== "MQTT") {
    print(scriptName, ":", 'ERR: GRID TYPE must be "EM", "3EM" or "MQTT"');
    return;
  }

  cb();
}

// device info

function getDeviceInfo(cb) {
  Shelly.call("Shelly.GetDeviceInfo", {}, function (result, errCode, errMessage) {
    if (errCode) {
      print(scriptName, ":", "[Shelly.GetDeviceInfo] ERR:", errCode, errMessage);
      return;
    }

    if (CONFIG.DEBUG > 1)
      print(scriptName, ":", "[Shelly.GetDeviceInfo] ", JSON.stringify(result));

    if (result.app && result.app.indexOf("3EM") != -1) {
      print(scriptName, ":", "[Shelly.GetDeviceInfo] Detected Shelly 3EM");
      DIVERT.switchID = 100;
      DIVERT.device = "3EM";

    } else if (result.app && result.app.indexOf("EM") != -1) {
      print(scriptName, ":", "[Shelly.GetDeviceInfo] Detected Shelly EM");
      DIVERT.switchID = 0;
      DIVERT.device = "EM";

    } else {
      DIVERT.switchID = -1;
      DIVERT.device = "unknown";
      print(scriptName, ":", "No Shelly EM/3EM detected, bypass switch disabled");
    }

    cb();
  });
}

// Throttling function
// Shelly Timer is a fixed-based timer, not an interval-based timer
// So we need to implement our own interval-based timer that will execute at a minimal interval of CONFIG.PID.INTERVAL_S seconds

function throttle(interval_ms, fn) {
  let elapsed = interval_ms;
  while (elapsed >= interval_ms) {
    const start = Shelly.getUptimeMs();
    fn();
    elapsed = Shelly.getUptimeMs() - start;
  }
  // we were fast! Let's schedule the next execution for later
  Timer.set(interval_ms - elapsed, false, function () {
    throttle(interval_ms, fn);
  });
}

// MQTT based or EM based diverting ?

function startShellyDivert() {
  print(scriptName, ":", "Grid Source: Shelly ", DIVERT.device);

  throttle(CONFIG.PID.INTERVAL_S * 1000, function () {
    const emMethod = CONFIG.GRID_SOURCE.TYPE === "3EM" ? "EM.GetStatus" : "EM1.GetStatus";

    Shelly.call(emMethod, { id: 0 }, function (result, errCode, errMessage) {
      if (errCode) {
        print(scriptName, ":", "[" + emMethod + "] ERR:", errCode, errMessage);
        return;
      }

      if (CONFIG.DEBUG > 1)
        print(scriptName, ":", "[" + emMethod + "] ", JSON.stringify(result));

      // exposes grid power and voltage to HTTP API endpoint and other parts of this script
      if (CONFIG.GRID_SOURCE.TYPE === "EM") {
        DIVERT.gridVoltage = result.voltage;
        DIVERT.gridPower = result.act_power;

      } else if (CONFIG.GRID_SOURCE.TYPE === "3EM") {
        DIVERT.gridVoltage = result.a_voltage;
        DIVERT.gridPower = result.total_act_power;
      }

      divert(DIVERT.gridVoltage, DIVERT.gridPower);
    });
  });
}

function startMQTTDivert() {
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

  // start periodic PID calculation and diverting
  throttle(CONFIG.PID.INTERVAL_S * 1000, function () {
    if (DIVERT.gridVoltage !== undefined && DIVERT.gridPower !== undefined) {
      divert(DIVERT.gridVoltage, DIVERT.gridPower);
    }
  });
}

// Main

validateConfig(function () {
  print(scriptName, ":", "Starting Shelly Solar Diverter...");

  getDeviceInfo(function () {
    // Register HTTP endpoint
    HTTPServer.registerEndpoint("status", onHttpGetStatus);

    if (CONFIG.GRID_SOURCE.TYPE === "EM") {
      startShellyDivert();

    } else if (CONFIG.GRID_SOURCE.TYPE === "MQTT") {
      startMQTTDivert();
    }
  });
});
