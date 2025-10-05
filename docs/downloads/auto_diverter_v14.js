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
 * - v4: Fixed dimmer sharing feature to use Watts instead of %: using % based on PID output is wrong and will make the PID react to compensate. EXCESS_POWER_LIMIT can also be used to limit an output power to a specific value.
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
 * ======================================
 */
const scriptName = "auto_diverter";

// Config

const CONFIG = {
  // Debug mode: 0, 1 (info) or 2 (verbose)
  DEBUG: 0,
  // Set ID of the Switch button on the Shelly: 0 for PRO EM50, 100 for PRO 3EM, negative to disable
  SHELLY_SWITCH_ID: -1,
  // Configure the sources for the grid power and voltage.
  // By default, the script will use a Shelly EM to read the grid power and voltage.
  // But it can be installed directly on the dimmer and read the power and voltage from MQTT.
  GRID_SOURCE: {
    TYPE: "EM", // "EM" or "MQTT"

    // Enter your grid nominal voltage here. It will be used while waiting for the correct value from MQTT
    NOMINAL_VOLTAGE: 230,

    // Grid Read Interval (s) for power and voltage (only used when GRID_SOURCE.TYPE is "EM")
    EM_READ_INTERVAL_S: 1,

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
    // Reverse
    REVERSE: false,
    // Proportional Mode:
    // - "error" (proportional on error),
    // - "input" (proportional on measurement),
    // - "both" (proportional on 50% error and 50% measurement)
    P_MODE: "input",
    // Derivative Mode:
    // - "error" (derivative on error),
    // - "input" (derivative on measurement)
    D_MODE: "error",
    // Integral Correction
    // - "off" (integral sum not clamped),
    // - "clamp" (integral sum not clamped to OUT_MIN and OUT_MAX),
    // - "advanced" (advanced anti-windup algorithm)
    IC_MODE: "advanced",
    // Target Grid Power (W)
    SETPOINT: 0,
    // At which power to switch between HIGH and LOW PID parameters
    HIGH_LOW_SWITCH: 200,
    // PID Parameters to use when grid power is far away from setpoint
    HIGH: {
      // PID Proportional Gain
      // Also try with 0.1, 0.2, 0.3
      KP: 0.2,
      // PID Integral Gain
      // Also try with 0.2, 0.3, 0.4, 0.5, 0.6 but keep it higher than Kp
      KI: 0.4,
      // PID Derivative Gain = keep it low
      KD: 0.05,
      // Output Minimum (W): should be below the SETPOINT value, like 300W below.
      OUT_MIN: -300,
      // Output Maximum (W): should be above the nominal power of the load.
      // I set below, the the available power to divert will be limited to this value globally for all dimmers.
      OUT_MAX: 2000,
    },
    // PID Parameters to use when grid power is near the setpoint
    LOW: {
      KP: 0.1,         // half of HIGH.KP
      KI: 0.2,         // half of HIGH.KI
      KD: 0.05,        // same as HIGH.KD
      OUT_MIN: -300,   // same as HIGH.OUT_MIN
      OUT_MAX: 2000,   // same as HIGH.OUT_MAX
    }
  },
  // DIMMER LIST
  DIMMERS: {
    boiler: {
      IP: "192.168.125.98",
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 24.37,
      // Maximum excess power to reserve to this load.
      // The remaining power will be given to the next dimmers
      EXCESS_POWER_LIMIT: 0,
      // Set whether the Shelly EM with this script will be used to control the bypass relay to force a heating
      // When set to true, if you activate remotely the bypass to force a heating, then the script will detect it and turn the dimmer off
      BYPASS_CONTROLLED_BY_EM: false,
      // If set to true, the calculation of the dimmer duty cycle will be done based on a power matching LUT table which considers the voltage and current sine wave.
      // This should be more precise than a linear dimming.
      // You can try it and set it to false to compare the results, there is no harm in that!
      USE_POWER_LUT: true,
      // If the dimmer is running at 0-1% for this duration, it will be turned off automatically.
      // This duration should be long enough in order to not turn off the internal dimmer relay too often
      // The value is in milliseconds, the default is 5 minutes.
      DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
    },
    // pool_heater: {
    //   IP: "192.168.125.99",
    //   RESISTANCE: 0,
    //   EXCESS_POWER_LIMIT: 0,
    //   BYPASS_CONTROLLED_BY_EM: false,
    //   USE_POWER_LUT: true,
    //   DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
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
  // "DIMMERS.heater1.EXCESS_POWER_LIMIT": "number:201",
  // "DIMMERS.heater2.EXCESS_POWER_LIMIT": "number:202",
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
  // Sum
  sum: 0,
};

// Divert Control

let DIVERT = {
  lastReadTime: 0,
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

    print(scriptName, ":", "Dimmer", dimmer_name, "is enabled");
    DIVERT.dimmers[dimmer_name] = {
      powerToDivert: 0
    };
  }

  cb();
}

function constrain(value, min, max) { return Math.min(Math.max(value, min), max); }

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

// - https://github.com/Dlloydev/QuickPID
// - https://github.com/br3ttb/Arduino-PID-Library
function calculatePID(input) {
  const dInput = CONFIG.PID.REVERSE ? PID.input - input : input - PID.input;
  const error = CONFIG.PID.REVERSE ? input - CONFIG.PID.SETPOINT : CONFIG.PID.SETPOINT - input;
  const dError = error - PID.error;

  if (CONFIG.DEBUG > 1) {
    print(scriptName, ":", "Input:", input, "W, Error:", error, "W, dError:", dError, "W");
  }

  let peTerm = PID.kp * error;
  let pmTerm = PID.kp * dInput;
  switch (CONFIG.PID.P_MODE) {
    case "error":
      pmTerm = 0;
      break;
    case "input":
      peTerm = 0;
      break;
    case "both":
      peTerm *= 0.5;
      pmTerm *= 0.5;
      break;
    default:
      return PID.output;
  }

  // pTerm
  PID.pTerm = peTerm - pmTerm;

  // iTerm
  PID.iTerm = PID.ki * error;

  if (CONFIG.DEBUG > 1) {
    print(scriptName, ":", "pTerm:", PID.pTerm, "W, iTerm:", PID.iTerm, "W");
  }

  // anti-windup
  if (CONFIG.PID.IC_MODE == "advanced" && PID.ki) {
    const iTermOut = PID.pTerm + PID.ki * (PID.iTerm + error);
    if ((iTermOut > PID.out_max && dError > 0) || (iTermOut < PID.out_min && dError < 0)) {
      PID.iTerm = constrain(iTermOut, -PID.out_max, PID.out_max);
    }
  }

  // integral sum
  PID.sum = CONFIG.PID.IC_MODE == "off" ? (PID.sum + PID.iTerm - pmTerm) : constrain(PID.sum + PID.iTerm - pmTerm, PID.out_min, PID.out_max);

  // dTerm
  switch (CONFIG.PID.D_MODE) {
    case "error":
      PID.dTerm = PID.kd * dError;
      break;
    case "input":
      PID.dTerm = -PID.kd * dInput;
      break;
    default:
      return PID.output;
  }

  PID.output = constrain(PID.sum + peTerm + PID.dTerm, PID.out_min, PID.out_max);

  PID.input = input;
  PID.error = error;

  return PID.output;
}

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
  // computer the dimmer % based on either the duty cycle or the power LUT table
  let dimmerPercentage = CONFIG.DIMMERS[dimmer_name].USE_POWER_LUT ? (CONFIG.SEMI_PERIOD - dimmer.firingDelay) / CONFIG.SEMI_PERIOD : dimmer.dutyCycle;
  dimmerPercentage = Math.round(constrain(dimmerPercentage * 100, 0, 100));

  // compute whether we should activate or deactivate the relay
  let relay = "";
  if (!dimmer.lastActivation) {
    dimmer.lastActivation = 0;
  }
  if (dimmerPercentage > 1) {
    // if dimmerPercentage > 1, we set the dimmer to the computed percentage which will turn on the internal dimmer relay if it was off.
    relay = "&on=true";
    dimmer.lastActivation = Date.now();
  } else {
    // if dimmerPercentage <= 1, we set the dimmer to 1% because we do not want to set the dimmer to 0% otherwise it will deactivate the internal relays.
    // But if the dimmer was already at 1% or less for a while, we can turn it off.
    if (Date.now() - dimmer.lastActivation >= CONFIG.DIMMERS[dimmer_name].DIMMER_TURN_OFF_DELAY) {
      dimmerPercentage = 0;
      relay = "&on=false";
      if (CONFIG.DEBUG)
        console.log(scriptName, ":", "[" + dimmer_name + "] ROUTING => relay: OFF, dimmer:", dimmerPercentage + "%");
    } else {
      dimmerPercentage = 1;
    }
  }

  // Build the URL to call...
  const url = "http://" + CONFIG.DIMMERS[dimmer_name].IP + "/rpc/Light.Set?id=0" + relay + "&brightness=" + dimmerPercentage + "&transition_duration=0.5";

  if (CONFIG.DEBUG > 1)
    console.log(scriptName, ":", "[" + dimmer_name + "] => Calling: ", url);

  Shelly.call("HTTP.GET", { url: url, timeout: 5 }, callDimmerCallback, { dimmer: dimmer, cb: cb });
}

function callDimmers(cb) {
  function recallMe() {
    callDimmers(cb)
  }

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
    callDimmer(dimmer_name, dimmer, recallMe);

    // exit the loop immediately to avoid multiple calls in case the yare made in parallel
    return;
  }

  // if we are here, all dimmers have been contacted
  cb();
}

function onSwitchGetStatus(result, errCode, errMessage, data) {
  if (errCode === 404) {
    // Switch.GetStatus does not exist: we do not run on a EM.
    errCode = 0;
    result = { output: false };
  }
  if (errCode) {
    print(scriptName, ":", "ERR onSwitchGetStatus:", errCode);
    throttleReadShellyEM();
    return;
  }
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "onSwitchGetStatus:", JSON.stringify(result));
  if (result.output) {
    for (let dimmer_name in DIVERT.dimmers) {
      let dimmer = DIVERT.dimmers[dimmer_name];
      if (CONFIG.DIMMERS[dimmer_name].BYPASS_CONTROLLED_BY_EM) {
        print(scriptName, ":", "Bypass is ON, turning off dimmer", dimmer_name);
        dimmer.apparentPower = 0;
        dimmer.current = 0;
        dimmer.dimmedVoltage = 0;
        dimmer.powerToDivert = 0;
        dimmer.dutyCycle = 0;
        dimmer.firingDelay = 0;
        dimmer.powerFactor = 0;
        dimmer.thdi = 0;
      }
    }
  }
  callDimmers(throttleReadShellyEM);
}

function divert(gridVoltage, gridPower) {
  getDynUserSettings();
  if (Math.abs(DIVERT.gridPower - CONFIG.PID.SETPOINT) < CONFIG.PID.HIGH_LOW_SWITCH) {
    PID.mode = "LOW";
    PID.kp = CONFIG.PID.LOW.KP;
    PID.ki = CONFIG.PID.LOW.KI;
    PID.kd = CONFIG.PID.LOW.KD;
    PID.out_min = CONFIG.PID.LOW.OUT_MIN;
    PID.out_max = CONFIG.PID.LOW.OUT_MAX;
  } else {
    PID.mode = "HIGH";
    PID.kp = CONFIG.PID.HIGH.KP;
    PID.ki = CONFIG.PID.HIGH.KI;
    PID.kd = CONFIG.PID.HIGH.KD;
    PID.out_min = CONFIG.PID.HIGH.OUT_MIN;
    PID.out_max = CONFIG.PID.HIGH.OUT_MAX;
  }

  let availablePowerToDivert = calculatePID(gridPower);

  if (CONFIG.DEBUG > 0)
    print(scriptName, ":", "Grid:", gridVoltage, "V,", Math.round(gridPower), "W => To divert:", Math.round(availablePowerToDivert), "W");

  for (let dimmer_name in DIVERT.dimmers) {
    let dimmer = DIVERT.dimmers[dimmer_name];

    // calculate powerToDivert
    dimmer.maxPower = gridVoltage * gridVoltage / CONFIG.DIMMERS[dimmer_name].RESISTANCE;
    dimmer.powerToDivert = constrain(availablePowerToDivert, 0, dimmer.maxPower);
    if (CONFIG.DIMMERS[dimmer_name].EXCESS_POWER_LIMIT > 0)
      dimmer.powerToDivert = constrain(dimmer.powerToDivert, 0, CONFIG.DIMMERS[dimmer_name].EXCESS_POWER_LIMIT);

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

    availablePowerToDivert -= dimmer.powerToDivert;

    if (CONFIG.DEBUG > 0)
      console.log(scriptName, ":", "[" + dimmer_name + "] => ", Math.round(dimmer.powerToDivert), "W,", Math.round(dimmer.dutyCycle * 100), "%,", "PID mode: ", PID.mode);
  }

  if (CONFIG.SHELLY_SWITCH_ID >= 0)
    Shelly.call("Switch.GetStatus", { id: CONFIG.SHELLY_SWITCH_ID }, onSwitchGetStatus);
  else
    callDimmers(throttleReadShellyEM);
}

function onEM1GetStatus(result, errCode, errMessage, data) {
  if (errCode) {
    print(scriptName, ":", "ERR onEM1GetStatus:", errCode);
    throttleReadShellyEM();
    return;
  }
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "EM1.GetStatus:", JSON.stringify(result));
  // exposes grid power and voltage to HTTP API endpoint and other parts of this script
  DIVERT.gridVoltage = result.voltage;
  DIVERT.gridPower = result.act_power;
  divert(result.voltage, result.act_power);
}

function readShellyEM() {
  DIVERT.lastReadTime = Date.now();
  Shelly.call("EM1.GetStatus", { id: 0 }, onEM1GetStatus);
}

function throttleReadShellyEM() {
  if (CONFIG.GRID_SOURCE.TYPE !== "EM") {
    // do nothing if we are not using a Shelly EM
    // values are coming from MQTT
    return;
  }
  const now = Date.now();
  const itvl = CONFIG.GRID_SOURCE.EM_READ_INTERVAL_S * 1000;
  const diff = now - DIVERT.lastReadTime;
  if (diff > itvl) {
    readShellyEM();
  } else {
    Timer.set(itvl - diff, false, readShellyEM);
  }
}

// HTTP handlers

function onHttpGetStatus(request, response) {
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
  HTTPServer.registerEndpoint("status", onHttpGetStatus);
  if (CONFIG.GRID_SOURCE.TYPE === "EM") {
    print(scriptName, ":", "Grid Source: Shelly EM");
    throttleReadShellyEM();
  } else if (CONFIG.GRID_SOURCE.TYPE === "MQTT") {
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
      if (DIVERT.gridVoltage) {
        divert(DIVERT.gridVoltage, DIVERT.gridPower);
      }
    });
  } else {
    print(scriptName, ":", "ERR: Invalid Grid Source: ", CONFIG.GRID_SOURCE.TYPE);
  }
});
